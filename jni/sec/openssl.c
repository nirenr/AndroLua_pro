/* ==========================================================================
 * openssl.c - Lua OpenSSL
 * --------------------------------------------------------------------------
 * Copyright (c) 2012-2015  William Ahern
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 * ==========================================================================
 */
#include <limits.h>       /* INT_MAX INT_MIN LLONG_MAX LLONG_MIN UCHAR_MAX ULLONG_MAX */
#include <stdint.h>       /* uintptr_t */
#include <string.h>       /* memset(3) strerror_r(3) */
#include <strings.h>      /* strcasecmp(3) */
#include <math.h>         /* INFINITY fabs(3) floor(3) frexp(3) fmod(3) round(3) isfinite(3) */
#include <time.h>         /* struct tm time_t strptime(3) time(2) */
#include <ctype.h>        /* tolower(3) */
#include <errno.h>        /* ENOMEM ENOTSUP EOVERFLOW errno */
#include <assert.h>       /* assert */

#include <sys/types.h>    /* ssize_t pid_t */
#include <sys/time.h>     /* struct timeval gettimeofday(2) */
#include <sys/stat.h>     /* struct stat stat(2) */
#include <sys/socket.h>   /* AF_INET AF_INET6 */
#include <sys/resource.h> /* RUSAGE_SELF struct rusage getrusage(2) */
#include <sys/utsname.h>  /* struct utsname uname(3) */
#include <fcntl.h>        /* O_RDONLY O_CLOEXEC open(2) */
#include <unistd.h>       /* close(2) getpid(2) */
#include <netinet/in.h>   /* struct in_addr struct in6_addr */
#include <arpa/inet.h>    /* inet_pton(3) */
#include <pthread.h>      /* pthread_mutex_init(3) pthread_mutex_lock(3) pthread_mutex_unlock(3) */
#include <dlfcn.h>        /* dladdr(3) dlopen(3) */

#if __APPLE__
#include <mach/mach_time.h> /* mach_absolute_time() */
#endif

#include <openssl/opensslconf.h>
#include <openssl/opensslv.h>
#include <openssl/err.h>
#include <openssl/bn.h>
#include <openssl/asn1.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pkcs12.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <openssl/des.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#if LUA_VERSION_NUM < 502
#include "compat52.h"
#endif

#define OPENSSL_PREREQ(M, m, p) \
	(OPENSSL_VERSION_NUMBER >= (((M) << 28) | ((m) << 20) | ((p) << 12)) && !defined LIBRESSL_VERSION_NUMBER)

#define LIBRESSL_PREREQ(M, m, p) \
	(LIBRESSL_VERSION_NUMBER >= (((M) << 28) | ((m) << 20) | ((p) << 12)))

#ifndef HAVE_DLADDR
#define HAVE_DLADDR (!defined _AIX) /* TODO: https://root.cern.ch/drupal/content/aix-and-dladdr */
#endif

#ifndef HAVE_SSL_CTX_SET_ALPN_PROTOS
#define HAVE_SSL_CTX_SET_ALPN_PROTOS OPENSSL_PREREQ(1, 0, 2)
#endif

#ifndef HAVE_SSL_CTX_SET_ALPN_SELECT_CB
#define HAVE_SSL_CTX_SET_ALPN_SELECT_CB HAVE_SSL_CTX_SET_ALPN_PROTOS
#endif

#ifndef HAVE_SSL_SET_ALPN_PROTOS
#define HAVE_SSL_SET_ALPN_PROTOS HAVE_SSL_CTX_SET_ALPN_PROTOS
#endif

#ifndef HAVE_SSL_GET0_ALPN_SELECTED
#define HAVE_SSL_GET0_ALPN_SELECTED HAVE_SSL_CTX_SET_ALPN_PROTOS
#endif

#ifndef HAVE_DTLSV1_CLIENT_METHOD
#define HAVE_DTLSV1_CLIENT_METHOD (!defined OPENSSL_NO_DTLS1)
#endif

#ifndef HAVE_DTLSV1_SERVER_METHOD
#define HAVE_DTLSV1_SERVER_METHOD HAVE_DTLSV1_CLIENT_METHOD
#endif

#ifndef HAVE_DTLS_CLIENT_METHOD
#define HAVE_DTLS_CLIENT_METHOD (OPENSSL_PREREQ(1, 0, 2) && !defined OPENSSL_NO_DTLS1)
#endif

#ifndef HAVE_DTLS_SERVER_METHOD
#define HAVE_DTLS_SERVER_METHOD HAVE_DTLS_CLIENT_METHOD
#endif

#ifndef HAVE_DTLSV1_2_CLIENT_METHOD
#define HAVE_DTLSV1_2_CLIENT_METHOD (OPENSSL_PREREQ(1, 0, 2) && !defined OPENSSL_NO_DTLS1)
#endif

#ifndef HAVE_DTLSV1_2_SERVER_METHOD
#define HAVE_DTLSV1_2_SERVER_METHOD HAVE_DTLSV1_2_CLIENT_METHOD
#endif

#ifndef STRERROR_R_CHAR_P
#define STRERROR_R_CHAR_P (defined __GLIBC__ && (_GNU_SOURCE || !(_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)))
#endif

#ifndef LIST_HEAD
#define LIST_HEAD(name, type) struct name { struct type *lh_first; }
#define LIST_ENTRY(type) struct { struct type *le_next, **le_prev; }
#define LIST_INIT(head) do { LIST_FIRST((head)) = NULL; } while (0)
#define LIST_FIRST(head) ((head)->lh_first)
#define LIST_NEXT(elm, field) ((elm)->field.le_next)
#define LIST_REMOVE(elm, field) do { \
	if (LIST_NEXT((elm), field) != NULL) \
		LIST_NEXT((elm), field)->field.le_prev = (elm)->field.le_prev; \
	*(elm)->field.le_prev = LIST_NEXT((elm), field); \
} while (0)
#define LIST_INSERT_HEAD(head, elm, field) do { \
	if ((LIST_NEXT((elm), field) = LIST_FIRST((head))) != NULL) \
		LIST_FIRST((head))->field.le_prev = &LIST_NEXT((elm), field); \
	LIST_FIRST((head)) = (elm); \
	(elm)->field.le_prev = &LIST_FIRST((head)); \
} while (0)
#endif

#define BIGNUM_CLASS     "BIGNUM*"
#define PKEY_CLASS       "EVP_PKEY*"
#define X509_NAME_CLASS  "X509_NAME*"
#define X509_GENS_CLASS  "GENERAL_NAMES*"
#define X509_EXT_CLASS   "X509_EXTENSION*"
#define X509_CERT_CLASS  "X509*"
#define X509_CHAIN_CLASS "STACK_OF(X509)*"
#define X509_CSR_CLASS   "X509_REQ*"
#define X509_CRL_CLASS   "X509_CRL*"
#define X509_STORE_CLASS "X509_STORE*"
#define X509_STCTX_CLASS "X509_STORE_CTX*"
#define PKCS12_CLASS     "PKCS12*"
#define SSL_CTX_CLASS    "SSL_CTX*"
#define SSL_CLASS        "SSL*"
#define DIGEST_CLASS     "EVP_MD_CTX"     /* not a pointer */
#define HMAC_CLASS       "HMAC_CTX"       /* not a pointer */
#define CIPHER_CLASS     "EVP_CIPHER_CTX" /* not a pointer */


#if __GNUC__
#define NOTUSED __attribute__((unused))
#else
#define NOTUSED
#endif


#define countof(a) (sizeof (a) / sizeof *(a))
#define endof(a) (&(a)[countof(a)])

#define CLAMP(i, min, max) (((i) < (min))? (min) : ((i) > (max))? (max) : (i))

#undef MIN
#define MIN(a, b) (((a) < (b))? (a) : (b))

#define stricmp(a, b) strcasecmp((a), (b))
#define strieq(a, b) (!stricmp((a), (b)))

#define xtolower(c) tolower((unsigned char)(c))

#define SAY_(file, func, line, fmt, ...) \
	fprintf(stderr, "%s:%d: " fmt "%s", __func__, __LINE__, __VA_ARGS__)

#define SAY(...) SAY_(__FILE__, __func__, __LINE__, __VA_ARGS__, "\n")

#define HAI SAY("hai")


#define xitoa_putc(c) do { if (p < lim) dst[p] = (c); p++; } while (0)

static const char *xitoa(char *dst, size_t lim, long i) {
	size_t p = 0;
	unsigned long d = 1000000000UL, n = 0, r;

	if (i < 0) {
		xitoa_putc('-');
		i *= -1;
	}

	if ((i = MIN(2147483647L, i))) {
		do {
			if ((r = i / d) || n) {
				i -= r * d;
				n++;
				xitoa_putc('0' + r);
			}
		} while (d /= 10);
	} else {
		xitoa_putc('0');
	}

	if (lim)
		dst[MIN(p, lim - 1)] = '\0';

	return dst;
} /* xitoa() */


static void *prepudata(lua_State *L, size_t size, const char *tname, int (*gc)(lua_State *)) {
	void *p = memset(lua_newuserdata(L, size), 0, size);

	if (tname) {
		luaL_setmetatable(L, tname);
	} else {
		lua_newtable(L);
		lua_pushcfunction(L, gc);
		lua_setfield(L, -2, "__gc");
		lua_setmetatable(L, -2);
	}

	return p;
} /* prepudata() */


static void *prepsimple(lua_State *L, const char *tname, int (*gc)(lua_State *)) {
	void **p = prepudata(L, sizeof (void *), tname, gc);
	return p;
} /* prepsimple() */

#define prepsimple_(a, b, c, ...) prepsimple((a), (b), (c))
#define prepsimple(...) prepsimple_(__VA_ARGS__, 0, 0)


static void *checksimple(lua_State *L, int index, const char *tname) {
	void **p;

	if (tname) {
		p = luaL_checkudata(L, index, tname);
	} else {
		luaL_checktype(L, index, LUA_TUSERDATA);
		p = lua_touserdata(L, index);
	}

	return *p;
} /* checksimple() */


static void *testsimple(lua_State *L, int index, const char *tname) {
	void **p;

	if (tname) {
		p = luaL_testudata(L, index, tname);
	} else {
		luaL_checktype(L, index, LUA_TUSERDATA);
		p = lua_touserdata(L, index);
	}

	return (p)? *p : (void *)0;
} /* testsimple() */


static int interpose(lua_State *L, const char *mt) {
	luaL_getmetatable(L, mt);

	if (!strncmp("__", luaL_checkstring(L, 1), 2))
		lua_pushvalue(L, -1);
	else
		lua_getfield(L, -1, "__index");

	lua_pushvalue(L, -4); /* push method name */
	lua_gettable(L, -2);  /* push old method */

	lua_pushvalue(L, -5); /* push method name */
	lua_pushvalue(L, -5); /* push new method */
	lua_settable(L, -4);  /* replace old method */

	return 1; /* return old method */
} /* interpose() */


static void addclass(lua_State *L, const char *name, const luaL_Reg *methods, const luaL_Reg *metamethods) {
	if (luaL_newmetatable(L, name)) {
		luaL_setfuncs(L, metamethods, 0);
		lua_newtable(L);
		luaL_setfuncs(L, methods, 0);
		lua_setfield(L, -2, "__index");
		lua_pop(L, 1);
	}
} /* addclass() */


static int badoption(lua_State *L, int index, const char *opt) {
	opt = (opt)? opt : luaL_checkstring(L, index);

	return luaL_argerror(L, index, lua_pushfstring(L, "invalid option %s", opt));
} /* badoption() */

static int checkoption(lua_State *L, int index, const char *def, const char *const opts[]) {
	const char *opt = (def)? luaL_optstring(L, index, def) : luaL_checkstring(L, index);
	int i; 

	for (i = 0; opts[i]; i++) {
		if (strieq(opts[i], opt))
			return i;
	}

	return badoption(L, index, opt);
} /* checkoption() */


#define X509_ANY 0x01
#define X509_PEM 0x02
#define X509_DER 0x04
#define X509_ALL (X509_PEM|X509_DER)

static int optencoding(lua_State *L, int index, const char *def, int allow) {
	static const char *const opts[] = { "*", "pem", "der", NULL };
	int type = 0;

	switch (checkoption(L, index, def, opts)) {
	case 0:
		type = X509_ANY;
		break;
	case 1:
		type = X509_PEM;
		break;
	case 2:
		type = X509_DER;
		break;
	}

	if (!(type & allow))
		luaL_argerror(L, index, lua_pushfstring(L, "invalid option %s", luaL_checkstring(L, index)));

	return type;
} /* optencoding() */


static _Bool rawgeti(lua_State *L, int index, int n) {
	lua_rawgeti(L, index, n);

	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);

		return 0;
	} else {
		return 1;
	}
} /* rawgeti() */


/* check ALPN protocols and add to buffer of length-prefixed strings */
static void checkprotos(luaL_Buffer *B, lua_State *L, int index) {
	int n;

	luaL_checktype(L, index, LUA_TTABLE);

	for (n = 1; rawgeti(L, index, n); n++) {
		const char *tmp;
		size_t len;

		switch (lua_type(L, -1)) {
		case LUA_TSTRING:
			break;
		default:
			luaL_argerror(L, index, "array of strings expected");
		}

		tmp = luaL_checklstring(L, -1, &len);
		luaL_argcheck(L, len > 0 && len <= UCHAR_MAX, index, "proto string length invalid");
		luaL_addchar(B, (unsigned char)len);
		luaL_addlstring(B, tmp, len);
		lua_pop(L, 1);
	}
} /* checkprotos() */

static void pushprotos(lua_State *L, const unsigned char *p, size_t n) {
	const unsigned char *pe = &p[n];
	int i = 0;

	lua_newtable(L);

	while (p < pe) {
		n = *p++;

		if ((size_t)(pe - p) < n)
			luaL_error(L, "corrupt ALPN protocol list (%zu > %zu)", n, (size_t)(pe - p));

		lua_pushlstring(L, (const void *)p, n);
		lua_rawseti(L, -2, ++i);
		p += n;
	}
} /* pushprotos() */


static _Bool getfield(lua_State *L, int index, const char *k) {
	lua_getfield(L, index, k);

	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);

		return 0;
	} else {
		return 1;
	}
} /* getfield() */


static _Bool loadfield(lua_State *L, int index, const char *k, int type, void *p) {
	if (!getfield(L, index, k))
		return 0;

	switch (type) {
	case LUA_TSTRING:
		*(const char **)p = luaL_checkstring(L, -1);
		break;
	case LUA_TNUMBER:
		*(lua_Number *)p = luaL_checknumber(L, -1);
		break;
	default:
		luaL_error(L, "loadfield(type=%d): invalid type", type);
		break;
	} /* switch() */

	lua_pop(L, 1); /* table keeps reference */

	return 1;
} /* loadfield() */


static void *loadfield_udata(lua_State *L, int index, const char *k, const char *tname) {
	if (!getfield(L, index, k))
		return NULL;

	void **p = luaL_checkudata(L, -1, tname);

	lua_pop(L, 1); /* table keeps reference */

	return *p;
} /* loadfield_udata() */


/*
 * Auxiliary C routines
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define AUX_MIN(a, b) (((a) < (b))? (a) : (b))

static size_t aux_strlcpy(char *dst, const char *src, size_t lim) {
	size_t n = strlen(src);

	if (lim > 0) {
		size_t m = AUX_MIN(lim - 1, n);

		memcpy(dst, src, m);
		dst[m] = '\0';
	}

	return n;
} /* aux_strlcpy() */

#define aux_strerror(error) aux_strerror_r((error), (char[256]){ 0 }, 256)

static const char *aux_strerror_r(int error, char *dst, size_t lim) {
	static const char unknown[] = "Unknown error: ";
	size_t n;

#if STRERROR_R_CHAR_P
	char *rv = strerror_r(error, dst, lim);

	if (rv != NULL)
		return dst;
#else
	int rv = strerror_r(error, dst, lim);

	if (0 == rv)
		return dst;
#endif

	/*
	 * glibc snprintf can fail on memory pressure, so format our number
	 * manually.
	 */
	n = MIN(sizeof unknown - 1, lim);
	memcpy(dst, unknown, n);

	return xitoa(&dst[n], lim - n, error);
} /* aux_strerror_r() */


/*
 * Auxiliary OpenSSL API routines
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static size_t auxS_nid2sn(void *dst, size_t lim, int nid) {
	const char *sn;

	if (nid == NID_undef || !(sn = OBJ_nid2sn(nid)))
		return 0;

	return aux_strlcpy(dst, sn, lim);
} /* aux2_nid2sn() */

static size_t auxS_obj2sn(void *dst, size_t lim, const ASN1_OBJECT *obj) {
	return auxS_nid2sn(dst, lim, OBJ_obj2nid(obj));
} /* auxS_obj2sn() */

static size_t auxS_nid2ln(void *dst, size_t lim, int nid) {
	const char *ln;

	if (nid == NID_undef || !(ln = OBJ_nid2ln(nid)))
		return 0;

	return aux_strlcpy(dst, ln, lim);
} /* aux2_nid2ln() */

static size_t auxS_obj2ln(void *dst, size_t lim, const ASN1_OBJECT *obj) {
	return auxS_nid2ln(dst, lim, OBJ_obj2nid(obj));
} /* auxS_obj2ln() */

static size_t auxS_obj2id(void *dst, size_t lim, const ASN1_OBJECT *obj) {
	int n = OBJ_obj2txt(dst, AUX_MIN(lim, INT_MAX), obj, 1);

	/* TODO: push custom errors onto error stack */
	if (n == 0) {
		return 0; /* obj->data == NULL */
	} else if (n < 0) {
		return 0; /* memory allocation error */
	} else {
		return n;
	}
} /* auxS_obj2id() */

static size_t auxS_nid2id(void *dst, size_t lim, int nid) {
	ASN1_OBJECT *obj;

	/* TODO: push custom error onto error stack */
	if (!(obj = OBJ_nid2obj(nid)))
		return 0;

	return auxS_obj2id(dst, lim, obj);
} /* auxS_nid2id() */

static size_t auxS_nid2txt(void *dst, size_t lim, int nid) {
	size_t n;

	if ((n = auxS_nid2sn(dst, lim, nid)))
		return n;
	if ((n = auxS_nid2ln(dst, lim, nid)))
		return n;

	return auxS_nid2id(dst, lim, nid);
} /* auxS_nid2txt() */

static size_t auxS_obj2txt(void *dst, size_t lim, const ASN1_OBJECT *obj) {
	size_t n;

	if ((n = auxS_obj2sn(dst, lim, obj)))
		return n;
	if ((n = auxS_obj2ln(dst, lim, obj)))
		return n;

	return auxS_obj2id(dst, lim, obj);
} /* auxS_obj2txt() */

static _Bool auxS_isoid(const char *txt) {
	return (*txt >= '0' && *txt <= '9');
} /* auxS_isoid() */

static _Bool auxS_txt2obj(ASN1_OBJECT **obj, const char *txt) {
	int nid;

	if ((nid = OBJ_sn2nid(txt)) != NID_undef
	||  (nid = OBJ_ln2nid(txt)) != NID_undef) {
		return NULL != (*obj = OBJ_nid2obj(nid));
	} else if (auxS_isoid(txt)) {
		return NULL != (*obj = OBJ_txt2obj(txt, 1));
	} else {
		*obj = NULL;
		return 1;
	}
} /* auxS_txt2obj() */


/*
 * Auxiliary Lua API routines
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef int auxref_t;
typedef int auxtype_t;

static void auxL_unref(lua_State *L, auxref_t *ref) {
	luaL_unref(L, LUA_REGISTRYINDEX, *ref);
	*ref = LUA_NOREF;
} /* auxL_unref() */

static void auxL_ref(lua_State *L, int index, auxref_t *ref) {
	auxL_unref(L, ref);
	lua_pushvalue(L, index);
	*ref = luaL_ref(L, LUA_REGISTRYINDEX);
} /* auxL_ref() */

static auxtype_t auxL_getref(lua_State *L, auxref_t ref) {
	if (ref == LUA_NOREF || ref == LUA_REFNIL) {
		lua_pushnil(L);
	} else {
		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
	}

	return lua_type(L, -1);
} /* auxL_getref() */

/*
 * Lua 5.3 distinguishes integers and numbers, and by default uses 64-bit
 * integers. The following routines try to preserve this distinction and
 * where possible detect range issues.
 *
 * The signed range checking assumes two's complement, no padding bits, and
 * sizeof lua_Integer <= sizeof long long. Which is a safe bet where OpenSSL
 * is typically used.
 */
#define auxL_Integer long long
#define auxL_IntegerMin LLONG_MIN
#define auxL_IntegerMax LLONG_MAX
#define auxL_Unsigned unsigned long long
#define auxL_UnsignedMin 0
#define auxL_UnsignedMax ULLONG_MAX

#define lua_IntegerMax ((1ULL << (sizeof (lua_Integer) * 8 - 1)) - 1)
#define lua_IntegerMin (-lua_IntegerMax - 1)

static void auxL_pushinteger(lua_State *L, auxL_Integer i) {
	/*
	 * TODO: Check value explicitly, but will need to silence compiler
	 * diagnostics about useless comparisons.
	 */
	if (sizeof (lua_Integer) >= sizeof i) {
		lua_pushinteger(L, i);
	} else {
		/* TODO: Check overflow. */
		lua_pushnumber(L, i);
	}
} /* auxL_pushinteger() */

NOTUSED static void auxL_pushunsigned(lua_State *L, auxL_Unsigned i) {
	if (i <= lua_IntegerMax) {
		lua_pushinteger(L, i);
	} else if (i == (auxL_Unsigned)(lua_Number)i) {
		lua_pushnumber(L, i);
	} else {
		luaL_error(L, "unsigned integer value not representable as lua_Integer or lua_Number");
	}
} /* auxL_pushunsigned() */

#define auxL_checkinteger_(a, b, c, d, ...) auxL_checkinteger((a), (b), (c), (d))
#define auxL_checkinteger(...) auxL_checkinteger_(__VA_ARGS__, auxL_IntegerMin, auxL_IntegerMax, 0)

static auxL_Integer (auxL_checkinteger)(lua_State *L, int index, auxL_Integer min, auxL_Integer max) {
	auxL_Integer i;

	if (sizeof (lua_Integer) >= sizeof (auxL_Integer)) {
		i = luaL_checkinteger(L, index);
	} else {
		/* TODO: Check overflow. */
		i = (auxL_Integer)luaL_checknumber(L, index);
	}

	if (i < min || i > max)
		luaL_error(L, "integer value out of range");

	return i;
} /* auxL_checkinteger() */

#define auxL_optinteger_(a, b, c, d, e, ...) auxL_optinteger((a), (b), (c), (d), (e))
#define auxL_optinteger(...) auxL_optinteger_(__VA_ARGS__, auxL_IntegerMin, auxL_IntegerMax, 0)

static auxL_Integer (auxL_optinteger)(lua_State *L, int index, auxL_Integer def, auxL_Integer min, auxL_Integer max) {
	return (lua_isnoneornil(L, index))? def : auxL_checkinteger(L, index, min, max);
} /* auxL_optinteger() */

#define auxL_checkunsigned_(a, b, c, d, ...) auxL_checkunsigned((a), (b), (c), (d))
#define auxL_checkunsigned(...) auxL_checkunsigned_(__VA_ARGS__, auxL_UnsignedMin, auxL_UnsignedMax, 0)

static auxL_Unsigned (auxL_checkunsigned)(lua_State *L, int index, auxL_Unsigned min, auxL_Unsigned max) {
	auxL_Unsigned i;

	if (sizeof (lua_Integer) >= sizeof (auxL_Unsigned)) {
		/* TODO: Check sign. */
		i = luaL_checkinteger(L, index);
	} else {
		/* TODO: Check sign and overflow. */
		i = (auxL_Integer)luaL_checknumber(L, index);
	}

	if (i < min || i > max)
		luaL_error(L, "integer value out of range");

	return i;
} /* auxL_checkunsigned() */

#define auxL_optunsigned_(a, b, c, d, e, ...) auxL_optunsigned((a), (b), (c), (d), (e))
#define auxL_optunsigned(...) auxL_optunsigned_(__VA_ARGS__, auxL_UnsignedMin, auxL_UnsignedMax, 0)

static auxL_Unsigned (auxL_optunsigned)(lua_State *L, int index, auxL_Unsigned def, auxL_Unsigned min, auxL_Unsigned max) {
	return (lua_isnoneornil(L, index))? def : auxL_checkunsigned(L, index, min, max);
} /* auxL_optunsigned() */

typedef struct {
	const char *name;
	auxL_Integer value;
} auxL_IntegerReg;

static void auxL_setintegers(lua_State *L, const auxL_IntegerReg *l) {
	for (; l->name; l++) {
		auxL_pushinteger(L, l->value);
		lua_setfield(L, -2, l->name);
	}
} /* auxL_setintegers() */

#define auxL_EDYLD -2
#define auxL_EOPENSSL -1

static const char *auxL_pusherror(lua_State *L, int error, const char *fun) {
	if (error == auxL_EOPENSSL) {
		unsigned long code;
		const char *path, *file;
		int line;
		char txt[256];

		if (!ERR_peek_error())
			return lua_pushstring(L, "oops: no OpenSSL errors set");

		code = ERR_get_error_line(&path, &line);

		if ((file = strrchr(path, '/'))) {
			++file;
		} else {
			file = path;
		}

		ERR_clear_error();

		ERR_error_string_n(code, txt, sizeof txt);

		if (fun) {
			return lua_pushfstring(L, "%s: %s:%d:%s", fun, file, line, txt);
		} else {
			return lua_pushfstring(L, "%s:%d:%s", file, line, txt);
		}
	} else if (error == auxL_EDYLD) {
		const char *const fmt = (fun)? "%s: %s" : "%.0s%s";

		return lua_pushfstring(L, fmt, (fun)? fun : "", dlerror());
	} else {
		const char *const fmt = (fun)? "%s: %s" : "%.0s%s";

		return lua_pushfstring(L, fmt, (fun)? fun : "", aux_strerror(error));
	}
} /* auxL_pusherror() */

static int auxL_error(lua_State *L, int error, const char *fun) {
	auxL_pusherror(L, error, fun);

	return lua_error(L);
} /* auxL_error() */

static const char *auxL_pushnid(lua_State *L, int nid) {
	char txt[256] = { 0 };
	size_t n;

	if (!(n = auxS_nid2txt(txt, sizeof txt, nid)) || n >= sizeof txt)
		luaL_error(L, "%d: invalid ASN.1 NID", nid);

	lua_pushlstring(L, txt, n);

	return lua_tostring(L, -1);
} /* auxL_pushnid() */


/*
 * dl - dynamically loaded module management
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * Prevent loader from unlinking us if we've registered a callback with
 * OpenSSL by taking another reference to ourselves.
 */
static int dl_anchor(void) {
#if HAVE_DLADDR
	extern int luaopen_sec(lua_State *);
	static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	static void *anchor;
	Dl_info info;
	int error = 0;

	if ((error = pthread_mutex_lock(&mutex)))
		return error;

	if (anchor)
		goto epilog;

	if (!dladdr((void *)&luaopen_sec, &info))
		goto dlerr;

	if (!(anchor = dlopen(info.dli_fname, RTLD_NOW|RTLD_LOCAL)))
		goto dlerr;
epilog:
	(void)pthread_mutex_unlock(&mutex);

	return error;
dlerr:
	error = auxL_EDYLD;

	goto epilog;
#else
	return 0;//ENOTSUP;
#endif
} /* dl_anchor() */


/*
 * compat - OpenSSL API compatibility and bug workarounds
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define COMPAT_X509_STORE_FREE_BUG 0x01

static struct {
	int flags;

	void (*X509_STORE_free)(X509_STORE *);

	struct {
		X509_STORE *store;
	} tmp;
} compat = {
	.flags = 0,
	.X509_STORE_free = &X509_STORE_free,
};

#if !HAVE_EVP_PKEY_BASE_ID
#define EVP_PKEY_base_id(key) compat_EVP_PKEY_base_id((key))

static int compat_EVP_PKEY_base_id(EVP_PKEY *key) {
	return EVP_PKEY_type(key->type);
} /* compat_EVP_PKEY_base_id() */
#endif


#if !HAVE_EVP_PKEY_GET0
#define EVP_PKEY_get0(key) compat_EVP_PKEY_get0((key))

static void *compat_EVP_PKEY_get0(EVP_PKEY *key) {
	void *ptr = NULL;

	switch (EVP_PKEY_base_id(key)) {
	case EVP_PKEY_RSA:
		if ((ptr = EVP_PKEY_get1_RSA(key)))
			RSA_free(ptr);
		break;
	case EVP_PKEY_DSA:
		if ((ptr = EVP_PKEY_get1_DSA(key)))
			DSA_free(ptr);
		break;
	case EVP_PKEY_DH:
		if ((ptr = EVP_PKEY_get1_DH(key)))
			DH_free(ptr);
		break;
#ifndef OPENSSL_NO_EC
	case EVP_PKEY_EC:
		if ((ptr = EVP_PKEY_get1_EC_KEY(key)))
			EC_KEY_free(ptr);
		break;
#endif
	default:
		/* TODO: Use ERR_put_error */

		break;
	}

	return ptr;
} /* compat_EVP_PKEY_get0() */
#endif

#if !HAVE_X509_GET0_EXT
#define X509_get0_ext(crt, i) X509_get_ext((crt), (i))
#endif

#if !HAVE_X509_CRL_GET0_EXT
#define X509_CRL_get0_ext(crt, i) X509_CRL_get_ext((crt), (i))
#endif

#if !HAVE_X509_EXTENSION_GET0_OBJECT
#define X509_EXTENSION_get0_object(ext) X509_EXTENSION_get_object((ext))
#endif

#if !HAVE_X509_EXTENSION_GET0_DATA
#define X509_EXTENSION_get0_data(ext) X509_EXTENSION_get_data((ext))
#endif

/*
 * X509_STORE_free in OpenSSL versions < 1.0.2 doesn't obey reference count
 */
#define X509_STORE_free(store) \
	(compat.X509_STORE_free)((store))

static void compat_X509_STORE_free(X509_STORE *store) {
	int i;

	i = CRYPTO_add(&store->references, -1, CRYPTO_LOCK_X509_STORE);

        if (i > 0)
                return;

	(X509_STORE_free)(store);
} /* compat_X509_STORE_free() */

#if !HAVE_SSL_CTX_set1_cert_store
#define SSL_CTX_set1_cert_store(ctx, store) \
	compat_SSL_CTX_set1_cert_store((ctx), (store))

static void compat_SSL_CTX_set1_cert_store(SSL_CTX *ctx, X509_STORE *store) {
	int n;

	/*
	 * This isn't thead-safe, but using X509_STORE or SSL_CTX objects
	 * from different threads isn't safe generally.
	 */
	if (ctx->cert_store) {
		X509_STORE_free(ctx->cert_store);
		ctx->cert_store = NULL;
	}

	n = store->references;

	SSL_CTX_set_cert_store(ctx, store);

	if (n == store->references)
		CRYPTO_add(&store->references, 1, CRYPTO_LOCK_X509_STORE);
} /* compat_SSL_CTX_set1_cert_store() */
#endif

static void compat_init_SSL_CTX_onfree(void *_ctx, void *data NOTUSED, CRYPTO_EX_DATA *ad NOTUSED, int idx NOTUSED, long argl NOTUSED, void *argp NOTUSED) {
	SSL_CTX *ctx = _ctx;

	if (ctx->cert_store) {
		X509_STORE_free(ctx->cert_store);
		ctx->cert_store = NULL;
	}
} /* compat_init_SSL_CTX_onfree() */

/* helper routine to determine if X509_STORE_free obeys reference count */
static void compat_init_X509_STORE_onfree(void *store, void *data NOTUSED, CRYPTO_EX_DATA *ad NOTUSED, int idx NOTUSED, long argl NOTUSED, void *argp NOTUSED) {
	/* unfortunately there's no way to remove a handler */
	if (store != compat.tmp.store)
		return;

	/* signal that we were freed by nulling our reference */
	compat.tmp.store = NULL;
} /* compat_init_X509_STORE_onfree() */

static int compat_init(void) {
	static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	static int store_index = -1, ssl_ctx_index = -1, done;
	int error = 0;

	if ((error = pthread_mutex_lock(&mutex)))
		return error;

	if (done)
		goto epilog;

	/*
	 * We need to unconditionally install at least one external
	 * application data callback. Because these can never be
	 * uninstalled, we can never be unloaded.
	 */
	if ((error = dl_anchor()))
		goto epilog;

	/*
	 * Test if X509_STORE_free obeys reference counts by installing an
	 * onfree callback.
	 */
	if (store_index == -1
	&&  -1 == (store_index = CRYPTO_get_ex_new_index(CRYPTO_EX_INDEX_X509_STORE, 0, NULL, NULL, NULL, &compat_init_X509_STORE_onfree)))
		goto sslerr;

	if (!(compat.tmp.store = X509_STORE_new()))
		goto sslerr;

	CRYPTO_add(&compat.tmp.store->references, 1, CRYPTO_LOCK_X509_STORE);
	X509_STORE_free(compat.tmp.store);

	if (compat.tmp.store) {
		/*
		 * Because our onfree callback didn't execute, we assume
		 * X509_STORE_free obeys reference counts. Alternatively,
		 * our callback might not have executed for some other
		 * reason. We assert the truth of our assumption by checking
		 * again after calling X509_STORE_free once more.
		 */
		X509_STORE_free(compat.tmp.store);
		assert(compat.tmp.store == NULL);
		compat.tmp.store = NULL; /* in case assertions disabled */
	} else {
		/*
		 * Because our onfree callback was invoked, X509_STORE_free
		 * appears not to obey reference counts. Use our fixed
		 * version in our own code.
		 */
		compat.X509_STORE_free = &compat_X509_STORE_free;

		 /*
		 * Ensure that our fixed version is called on SSL_CTX
		 * destruction.
		 *
		 * NB: We depend on the coincidental order of operations in
		 * SSL_CTX_free that user data destruction occurs before
		 * free'ing the cert_store member. Ruby's OpenSSL bindings
		 * also depend on this order as we both use the onfree
		 * callback to clear the member.
		 */
		if (ssl_ctx_index == -1
		&&  -1 == (ssl_ctx_index = CRYPTO_get_ex_new_index(CRYPTO_EX_INDEX_SSL_CTX, 0, NULL, NULL, NULL, &compat_init_SSL_CTX_onfree)))
			goto sslerr;

		compat.flags |= COMPAT_X509_STORE_FREE_BUG;
	}

	done = 1;
epilog:
	if (compat.tmp.store) {
		X509_STORE_free(compat.tmp.store);
		compat.tmp.store = NULL;
	}

	(void)pthread_mutex_unlock(&mutex);

	return error;
sslerr:
	error = auxL_EOPENSSL;

	goto epilog;
} /* compat_init() */


/*
 * External Application Data Hooks
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

struct ex_state {
	lua_State *L;
	LIST_HEAD(, ex_data) data;
}; /* struct ex_state */

#ifndef EX_DATA_MAXARGS
#define EX_DATA_MAXARGS 8
#endif

struct ex_data {
	struct ex_state *state;
	int refs;
	auxref_t arg[EX_DATA_MAXARGS];
	LIST_ENTRY(ex_data) le;
}; /* struct ex_data */

enum {
	EX_SSL_CTX_ALPN_SELECT_CB,
};

static struct ex_type {
	int class_index; /* OpenSSL object type identifier */
	int index; /* OpenSSL-allocated external data identifier */
	void *(*get_ex_data)();
	int (*set_ex_data)();
} ex_type[] = {
	[EX_SSL_CTX_ALPN_SELECT_CB] = { CRYPTO_EX_INDEX_SSL_CTX, -1, &SSL_CTX_get_ex_data, &SSL_CTX_set_ex_data },
};

static int ex_ondup(CRYPTO_EX_DATA *to NOTUSED, CRYPTO_EX_DATA *from NOTUSED, void *from_d, int idx NOTUSED, long argl NOTUSED, void *argp NOTUSED) {
	struct ex_data **data = from_d;

	if (*data)
		(*data)->refs++;

	return 1;
} /* ex_ondup() */

static void ex_onfree(void *parent NOTUSED, void *_data, CRYPTO_EX_DATA *ad NOTUSED, int idx NOTUSED, long argl NOTUSED, void *argp NOTUSED) {
	struct ex_data *data = _data;

	if (!data || --data->refs > 0)
		return;

	if (data->state) {
		int i;

		for (i = 0; i < (int)countof(data->arg); i++) {
			auxL_unref(data->state->L, &data->arg[i]);
		}

		LIST_REMOVE(data, le);
	}

	free(data);
} /* ex_onfree() */

static int ex_init(void) {
	static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	static int done;
	struct ex_type *type;
	int error = 0;

	if ((error = pthread_mutex_lock(&mutex)))
		return error;

	if (done)
		goto epilog;

	/*
	 * Our callbacks can never be uninstalled, so ensure we're never
	 * unloaded.
	 */
	if ((error = dl_anchor()))
		goto epilog;

	for (type = ex_type; type < endof(ex_type); type++) {
		if (type->index != -1)
			continue;

		if (-1 == (type->index = CRYPTO_get_ex_new_index(type->class_index, 0, NULL, NULL, &ex_ondup, &ex_onfree)))
			goto sslerr;
	};

	done = 1;
epilog:
	(void)pthread_mutex_unlock(&mutex);

	return error;
sslerr:
	error = auxL_EOPENSSL;

	goto epilog;
} /* ex_init() */

static int ex__gc(lua_State *L) {
	struct ex_state *state = lua_touserdata(L, 1);
	struct ex_data *data;

	if (!state)
		return 0;

	/* invalidate back references to Lua state */
	for (data = LIST_FIRST(&state->data); data; data = LIST_NEXT(data, le)) {
		data->state = NULL;
	}

	return 0;
} /* ex__gc() */

static _Bool ex_hasstate(lua_State *L) {
	_Bool has;

	lua_pushlightuserdata(L, (void *)&ex__gc);
	lua_gettable(L, LUA_REGISTRYINDEX);
	has = !lua_isnil(L, -1);
	lua_pop(L, 1);

	return has;
} /* ex_hasstate() */

static void ex_newstate(lua_State *L) {
	struct ex_state *state;
	struct lua_State *thr;

	if (ex_hasstate(L))
		return;

	state = prepudata(L, sizeof *state, NULL, &ex__gc);
	LIST_INIT(&state->data);

	/*
	 * XXX: Don't reuse mainthread because if an error occurs in a
	 * callback Lua might longjmp across the OpenSSL call stack.
	 * Instead, we'll install our own panic handlers.
	 */
#if defined LUA_RIDX_MAINTHREAD
	lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
	state->L = lua_tothread(L, -1);
	lua_pop(L, 1);
#else
	lua_pushvalue(L, -1);
	thr = lua_newthread(L);
	lua_settable(L, LUA_REGISTRYINDEX);
	state->L = thr;
#endif

	lua_pushlightuserdata(L, (void *)&ex__gc);
	lua_pushvalue(L, -2);
	lua_settable(L, LUA_REGISTRYINDEX);

	lua_pop(L, 1);
} /* ex_newstate() */

static struct ex_state *ex_getstate(lua_State *L) {
	struct ex_state *state;

	lua_pushlightuserdata(L, (void *)&ex__gc);
	lua_gettable(L, LUA_REGISTRYINDEX);

	luaL_checktype(L, -1, LUA_TUSERDATA);
	state = lua_touserdata(L, -1);
	lua_pop(L, 1);

	return state;
} /* ex_getstate() */

static size_t ex_getdata(lua_State **L, int _type, void *obj) {
	struct ex_type *type = &ex_type[_type];
	struct ex_data *data;
	size_t i;

	if (!(data = type->get_ex_data(obj, type->index)))
		return 0;
	if (!data->state)
		return 0;

	if (!*L)
		*L = data->state->L;

	if (!lua_checkstack(*L, countof(data->arg)))
		return 0;

	for (i = 0; i < countof(data->arg) && data->arg[i] != LUA_NOREF; i++) {
		lua_rawgeti(*L, LUA_REGISTRYINDEX, data->arg[i]);
	}

	return i;
} /* ex_getdata() */

/* returns 0 on success, otherwise error (>0 == errno, -1 == OpenSSL error) */
static int ex_setdata(lua_State *L, int _type, void *obj, size_t n) {
	struct ex_type *type = &ex_type[_type];
	struct ex_state *state;
	struct ex_data *data;
	size_t i, j;

	if (n > countof(data->arg))
		return EOVERFLOW;

	if ((data = type->get_ex_data(obj, type->index)) && data->state) {
		for (i = 0; i < countof(data->arg); i++) {
			auxL_unref(L, &data->arg[i]);
		}
	} else {
		state = ex_getstate(L);

		if (!(data = malloc(sizeof *data)))
			return errno;

		if (!type->set_ex_data(obj, type->index, data))
			return auxL_EOPENSSL;

		data->state = state;
		data->refs = 1;
		for (i = 0; i < countof(data->arg); i++)
			data->arg[i] = LUA_NOREF;
		LIST_INSERT_HEAD(&state->data, data, le);
	}

	for (i = n, j = 0; i > 0 && j < countof(data->arg); i--, j++) {
		auxL_ref(L, -(int)i, &data->arg[j]);
	}

	lua_pop(L, n);

	return 0;
} /* ex_setdata() */

static void initall(lua_State *L);


/*
 * compat - Lua OpenSSL
 *
 * Bindings to our internal feature detection, compatability, and workaround
 * code.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int luaopen_sec_compat(lua_State *L) {
	initall(L);

	lua_newtable(L);
	lua_pushboolean(L, !!(compat.flags & COMPAT_X509_STORE_FREE_BUG));
	lua_setfield(L, -2, "X509_STORE_FREE_BUG");

	return 1;
} /* luaopen_sec_compat() */


/*
 * OPENSSL - openssl
 *
 * Miscellaneous global interfaces.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static int ossl_version(lua_State *L) {
	if (lua_isnoneornil(L, 1)) {
		auxL_pushunsigned(L, SSLeay());
	} else {
		lua_pushstring(L, SSLeay_version(auxL_checkinteger(L, 1, INT_MIN, INT_MAX)));
	}

	return 1;
} /* ossl_version() */

static const luaL_Reg ossl_globals[] = {
	{ "version", &ossl_version },
	{ NULL,      NULL },
};

/*
 * NOTE: Compile-time cipher exclusions from openssl-1.0.1i/util/mkdef.pl.
 */
static const char opensslconf_no[][20] = {
#ifdef OPENSSL_NO_RC2
	{ "NO_RC2" },
#endif
#ifdef OPENSSL_NO_RC4
	{ "NO_RC4" },
#endif
#ifdef OPENSSL_NO_RC5
	{ "NO_RC5" },
#endif
#ifdef OPENSSL_NO_IDEA
	{ "NO_IDEA" },
#endif
#ifdef OPENSSL_NO_DES
	{ "NO_DES" },
#endif
#ifdef OPENSSL_NO_BF
	{ "NO_BF" },
#endif
#ifdef OPENSSL_NO_CAST
	{ "NO_CAST" },
#endif
#ifdef OPENSSL_NO_WHIRLPOOL
	{ "NO_WHIRLPOOL" },
#endif
#ifdef OPENSSL_NO_CAMELLIA
	{ "NO_CAMELLIA" },
#endif
#ifdef OPENSSL_NO_SEED
	{ "NO_SEED" },
#endif
#ifdef OPENSSL_NO_MD2
	{ "NO_MD2" },
#endif
#ifdef OPENSSL_NO_MD4
	{ "NO_MD4" },
#endif
#ifdef OPENSSL_NO_MD5
	{ "NO_MD5" },
#endif
#ifdef OPENSSL_NO_SHA
	{ "NO_SHA" },
#endif
#ifdef OPENSSL_NO_RIPEMD
	{ "NO_RIPEMD" },
#endif
#ifdef OPENSSL_NO_MDC2
	{ "NO_MDC2" },
#endif
#ifdef OPENSSL_NO_RSA
	{ "NO_RSA" },
#endif
#ifdef OPENSSL_NO_DSA
	{ "NO_DSA" },
#endif
#ifdef OPENSSL_NO_DH
	{ "NO_DH" },
#endif
#ifdef OPENSSL_NO_HMAC
	{ "NO_HMAC" },
#endif
#ifdef OPENSSL_NO_AES
	{ "NO_AES" },
#endif
#ifdef OPENSSL_NO_KRB5
	{ "NO_KRB5" },
#endif
#ifdef OPENSSL_NO_EC
	{ "NO_EC" },
#endif
#ifdef OPENSSL_NO_ECDSA
	{ "NO_ECDSA" },
#endif
#ifdef OPENSSL_NO_ECDH
	{ "NO_ECDH" },
#endif
#ifdef OPENSSL_NO_ENGINE
	{ "NO_ENGINE" },
#endif
#ifdef OPENSSL_NO_HW
	{ "NO_HW" },
#endif
#ifdef OPENSSL_NO_FP_API
	{ "NO_FP_API" },
#endif
#ifdef OPENSSL_NO_STATIC_ENGINE
	{ "NO_STATIC_ENGINE" },
#endif
#ifdef OPENSSL_NO_GMP
	{ "NO_GMP" },
#endif
#ifdef OPENSSL_NO_DEPRECATED
	{ "NO_DEPRECATED" },
#endif
#ifdef OPENSSL_NO_RFC3779
	{ "NO_RFC3779" },
#endif
#ifdef OPENSSL_NO_PSK
	{ "NO_PSK" },
#endif
#ifdef OPENSSL_NO_TLSEXT
	{ "NO_TLSEXT" },
#endif
#ifdef OPENSSL_NO_CMS
	{ "NO_CMS" },
#endif
#ifdef OPENSSL_NO_CAPIENG
	{ "NO_CAPIENG" },
#endif
#ifdef OPENSSL_NO_JPAKE
	{ "NO_JPAKE" },
#endif
#ifdef OPENSSL_NO_SRP
	{ "NO_SRP" },
#endif
#ifdef OPENSSL_NO_SSL2
	{ "NO_SSL2" },
#endif
#ifdef OPENSSL_NO_EC2M
	{ "NO_EC2M" },
#endif
#ifdef OPENSSL_NO_NISTP_GCC
	{ "NO_NISTP_GCC" },
#endif
#ifdef OPENSSL_NO_NEXTPROTONEG
	{ "NO_NEXTPROTONEG" },
#endif
#ifdef OPENSSL_NO_SCTP
	{ "NO_SCTP" },
#endif
#ifdef OPENSSL_NO_UNIT_TEST
	{ "NO_UNIT_TEST" },
#endif
	{ "" } /* in case nothing is defined above */
}; /* opensslconf_no[] */

static const auxL_IntegerReg ssleay_version[] = {
#ifdef SSLEAY_VERSION_NUMBER
	{ "SSLEAY_VERSION_NUMBER", SSLEAY_VERSION_NUMBER },
#endif
#ifdef SSLEAY_VERSION
	{ "SSLEAY_VERSION", SSLEAY_VERSION },
#endif
#ifdef SSLEAY_OPTIONS
	{ "SSLEAY_OPTIONS", SSLEAY_OPTIONS },
#endif
#ifdef SSLEAY_CFLAGS
	{ "SSLEAY_CFLAGS", SSLEAY_CFLAGS },
#endif
#ifdef SSLEAY_BUILT_ON
	{ "SSLEAY_BUILT_ON", SSLEAY_BUILT_ON },
#endif
#ifdef SSLEAY_PLATFORM
	{ "SSLEAY_PLATFORM", SSLEAY_PLATFORM },
#endif
#ifdef SSLEAY_DIR
	{ "SSLEAY_DIR", SSLEAY_DIR },
#endif
	{ NULL, 0 },
};

int luaopen_sec(lua_State *L) {
	size_t i;

	luaL_newlib(L, ossl_globals);

	for (i = 0; i < countof(opensslconf_no); i++) {
		if (*opensslconf_no[i]) {
			lua_pushboolean(L, 1);
			lua_setfield(L, -2, opensslconf_no[i]);
		}
	}

	auxL_setintegers(L, ssleay_version);

	auxL_pushinteger(L, OPENSSL_VERSION_NUMBER);
	lua_setfield(L, -2, "VERSION_NUMBER");

	lua_pushstring(L, OPENSSL_VERSION_TEXT);
	lua_setfield(L, -2, "VERSION_TEXT");

	lua_pushstring(L, SHLIB_VERSION_HISTORY);
	lua_setfield(L, -2, "SHLIB_VERSION_HISTORY");

	lua_pushstring(L, SHLIB_VERSION_NUMBER);
	lua_setfield(L, -2, "SHLIB_VERSION_NUMBER");

#if defined LIBRESSL_VERSION_NUMBER
	auxL_pushinteger(L, LIBRESSL_VERSION_NUMBER);
	lua_setfield(L, -2, "LIBRESSL_VERSION_NUMBER");
#endif

	return 1;
} /* luaopen_sec() */


/*
 * BIGNUM - openssl.bignum
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static BIGNUM *bn_push(lua_State *L) {
	BIGNUM **ud = prepsimple(L, BIGNUM_CLASS);

	if (!(*ud = BN_new()))
		auxL_error(L, auxL_EOPENSSL, "bignum.new");

	return *ud;
} /* bn_push() */


#define checkbig_(a, b, c, ...) checkbig((a), (b), (c))
#define checkbig(...) checkbig_(__VA_ARGS__, &(_Bool){ 0 }, 0)

static BIGNUM *(checkbig)(lua_State *, int, _Bool *);

static int bn_new(lua_State *L) {
	int i, n;

	if ((n = lua_gettop(L)) > 0) {
		for (i = 1; i <= n; i++)
			checkbig(L, i);

		return n;
	} else {
		bn_push(L);

		return 1;
	}
} /* bn_new() */


static int bn_interpose(lua_State *L) {
	return interpose(L, BIGNUM_CLASS);
} /* bn_interpose() */


/* return integral part */
static inline double intof(double f) {
	return (isfinite(f))? floor(fabs(f)) : 0.0;
} /* intof() */


/* convert integral to BN_ULONG. returns success or failure. */
static _Bool int2ul(BN_ULONG *ul, double f) {
	int exp;

	frexp(f, &exp);

	if (exp > (int)sizeof *ul * 8)
		return 0;

	*ul = (BN_ULONG)f;

	return 1;
} /* int2ul() */


/* convert integral BIGNUM. returns success or failure. */
static _Bool int2bn(BIGNUM **bn, double q) {
	unsigned char nib[32], bin[32], *p;
	size_t i, n;
	double r;

	p = nib;

	while (q >= 1.0 && p < endof(nib)) {
		r = fmod(q, 256.0);
		*p++ = r;
		q = round((q - r) / 256.0);
	}

	n = p - nib;

	for (i = 0; i < n; i++) {
		bin[i] = *--p;
	}

	if (!(*bn = BN_bin2bn(bin, n, *bn)))
		return 0;

	return 1;
} /* int2bn() */


/* convert double to BIGNUM. returns success or failure. */
static _Bool f2bn(BIGNUM **bn, double f) {
	double i = intof(f);
	BN_ULONG lu;

	if (int2ul(&lu, i)) {
		if (!*bn && !(*bn = BN_new()))
			return 0;

		if (!BN_set_word(*bn, lu))
			return 0;
	} else if (!int2bn(bn, i))
		return 0;

	BN_set_negative(*bn, signbit(f));

	return 1;
} /* f2bn() */


static BIGNUM *(checkbig)(lua_State *L, int index, _Bool *lvalue) {
	BIGNUM **bn;
	const char *dec;
	size_t len;

	index = lua_absindex(L, index);

	switch (lua_type(L, index)) {
	case LUA_TSTRING:
		*lvalue = 0;

		dec = lua_tolstring(L, index, &len);

		luaL_argcheck(L, len > 0 && *dec, index, "invalid big number string");

		bn = prepsimple(L, BIGNUM_CLASS);

		if (!BN_dec2bn(bn, dec))
			auxL_error(L, auxL_EOPENSSL, "bignum");

		lua_replace(L, index);

		return *bn;
	case LUA_TNUMBER:
		*lvalue = 0;

		bn = prepsimple(L, BIGNUM_CLASS);

		if (!f2bn(bn, lua_tonumber(L, index)))
			auxL_error(L, auxL_EOPENSSL, "bignum");

		lua_replace(L, index);

		return *bn;
	default:
		*lvalue = 1;

		return checksimple(L, index, BIGNUM_CLASS);
	} /* switch() */
} /* checkbig() */


static void bn_prepops(lua_State *L, BIGNUM **r, BIGNUM **a, BIGNUM **b, _Bool commute) {
	_Bool lvalue = 1;

	lua_settop(L, 2); /* a, b */

	*a = checkbig(L, 1, &lvalue);

	if (!lvalue && commute)
		lua_pushvalue(L, 1);

	*b = checkbig(L, 2, &lvalue);

	if (!lvalue && commute && lua_gettop(L) < 3)
		lua_pushvalue(L, 2);

	if (lua_gettop(L) < 3)
		bn_push(L);

	*r = *(BIGNUM **)lua_touserdata(L, 3);
} /* bn_prepops() */


static int ctx__gc(lua_State *L) {
	BN_CTX **ctx = lua_touserdata(L, 1);

	if (*ctx) {
		BN_CTX_free(*ctx);
		*ctx = NULL;
	}

	return 0;
} /* ctx__gc() */

static BN_CTX *getctx(lua_State *L) {
	BN_CTX **ctx;

	lua_pushlightuserdata(L, (void *)&ctx__gc);
	lua_gettable(L, LUA_REGISTRYINDEX);

	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);

		ctx = prepsimple(L, NULL, &ctx__gc);

		if (!(*ctx = BN_CTX_new()))
			auxL_error(L, auxL_EOPENSSL, "bignum");

		lua_pushlightuserdata(L, (void *)&ctx__gc);
		lua_pushvalue(L, -2);
		lua_settable(L, LUA_REGISTRYINDEX);
	}

	ctx = lua_touserdata(L, -1);
	lua_pop(L, 1);

	return *ctx;
} /* getctx() */


static int bn__add(lua_State *L) {
	BIGNUM *r, *a, *b;

	bn_prepops(L, &r, &a, &b, 1);

	if (!BN_add(r, a, b))
		return auxL_error(L, auxL_EOPENSSL, "bignum:__add");

	return 1;
} /* bn__add() */


static int bn__sub(lua_State *L) {
	BIGNUM *r, *a, *b;

	bn_prepops(L, &r, &a, &b, 0);

	if (!BN_sub(r, a, b))
		return auxL_error(L, auxL_EOPENSSL, "bignum:__sub");

	return 1;
} /* bn__sub() */


static int bn__mul(lua_State *L) {
	BIGNUM *r, *a, *b;

	bn_prepops(L, &r, &a, &b, 1);

	if (!BN_mul(r, a, b, getctx(L)))
		return auxL_error(L, auxL_EOPENSSL, "bignum:__mul");

	return 1;
} /* bn__mul() */


static int bn__div(lua_State *L) {
	BIGNUM *r, *a, *b;

	bn_prepops(L, &r, &a, &b, 0);

	if (!BN_div(r, NULL, a, b, getctx(L)))
		return auxL_error(L, auxL_EOPENSSL, "bignum:__div");

	return 1;
} /* bn__div() */


static int bn__mod(lua_State *L) {
	BIGNUM *r, *a, *b;

	bn_prepops(L, &r, &a, &b, 0);

	if (!BN_mod(r, a, b, getctx(L)))
		return auxL_error(L, auxL_EOPENSSL, "bignum:__mod");

	return 1;
} /* bn__mod() */


static int bn__pow(lua_State *L) {
	BIGNUM *r, *a, *b;

	bn_prepops(L, &r, &a, &b, 0);

	if (!BN_exp(r, a, b, getctx(L)))
		return auxL_error(L, auxL_EOPENSSL, "bignum:__pow");

	return 1;
} /* bn__pow() */


static int bn__unm(lua_State *L) {
	BIGNUM *a = checksimple(L, 1, BIGNUM_CLASS);

	BN_set_negative(a, !BN_is_negative(a));

	return 1;
} /* bn__unm() */


static int bn__eq(lua_State *L) {
	BIGNUM *a = checksimple(L, 1, BIGNUM_CLASS);
	BIGNUM *b = checksimple(L, 2, BIGNUM_CLASS);

	lua_pushboolean(L, 0 == BN_cmp(a, b));

	return 1;
} /* bn__eq() */


static int bn__lt(lua_State *L) {
	BIGNUM *a = checksimple(L, 1, BIGNUM_CLASS);
	BIGNUM *b = checksimple(L, 2, BIGNUM_CLASS);
	int cmp = BN_cmp(a, b);

	lua_pushboolean(L, cmp == -1);

	return 1;
} /* bn__lt() */


static int bn__le(lua_State *L) {
	BIGNUM *a = checksimple(L, 1, BIGNUM_CLASS);
	BIGNUM *b = checksimple(L, 2, BIGNUM_CLASS);
	int cmp = BN_cmp(a, b);

	lua_pushboolean(L, cmp <= 0);

	return 1;
} /* bn__le() */


static int bn__gc(lua_State *L) {
	BIGNUM **ud = luaL_checkudata(L, 1, BIGNUM_CLASS);

	if (*ud) {
		BN_free(*ud);
		*ud = NULL;
	}

	return 0;
} /* bn__gc() */


static int bn__tostring(lua_State *L) {
	BIGNUM *bn = checksimple(L, 1, BIGNUM_CLASS);
	char *txt;

	if (!(txt = BN_bn2dec(bn)))
		return auxL_error(L, auxL_EOPENSSL, "bignum:__tostring");

	lua_pushstring(L, txt);

	return 1;
} /* bn__tostring() */


static const luaL_Reg bn_methods[] = {
	{ NULL,  NULL },
};

static const luaL_Reg bn_metatable[] = {
	{ "__add",      &bn__add },
	{ "__sub",      &bn__sub },
	{ "__mul",      &bn__mul },
	{ "__div",      &bn__div },
	{ "__mod",      &bn__mod },
	{ "__pow",      &bn__pow },
	{ "__unm",      &bn__unm },
	{ "__eq",       &bn__eq },
	{ "__lt",       &bn__lt },
	{ "__le",       &bn__le },
	{ "__gc",       &bn__gc },
	{ "__tostring", &bn__tostring },
	{ NULL,         NULL },
};


static const luaL_Reg bn_globals[] = {
	{ "new",       &bn_new },
	{ "interpose", &bn_interpose },
	{ NULL,        NULL },
};

int luaopen_sec_bignum(lua_State *L) {
	initall(L);

	luaL_newlib(L, bn_globals);

	return 1;
} /* luaopen_sec_bignum() */


/*
 * EVP_PKEY - openssl.pkey
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static int bio__gc(lua_State *L) {
	BIO **bio = lua_touserdata(L, 1);

	if (*bio) {
		BIO_free(*bio);
		*bio = NULL;
	}

	return 0;
} /* bio__gc() */

static BIO *getbio(lua_State *L) {
	BIO **bio;

	lua_pushlightuserdata(L, (void *)&bio__gc);
	lua_gettable(L, LUA_REGISTRYINDEX);

	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);

		bio = prepsimple(L, NULL, &bio__gc);

		if (!(*bio = BIO_new(BIO_s_mem())))
			auxL_error(L, auxL_EOPENSSL, "BIO_new");

		lua_pushlightuserdata(L, (void *)&bio__gc);
		lua_pushvalue(L, -2);
		lua_settable(L, LUA_REGISTRYINDEX);
	}

	bio = lua_touserdata(L, -1);
	lua_pop(L, 1);

	BIO_reset(*bio);

	return *bio;
} /* getbio() */


static int pk_new(lua_State *L) {
	EVP_PKEY **ud;

	/* #1 table or key; if key, #2 format and #3 type */
	lua_settop(L, 3);

	ud = prepsimple(L, PKEY_CLASS);

	if (lua_istable(L, 1) || lua_isnil(L, 1)) {
		int type = EVP_PKEY_RSA;
		unsigned bits = 1024;
		unsigned exp = 65537;
		int curve = NID_X9_62_prime192v1;
		const char *id;
		lua_Number n;

		if (!lua_istable(L, 1))
			goto creat;

		if (loadfield(L, 1, "type", LUA_TSTRING, &id)) {
			static const struct { int nid; const char *sn; } types[] = {
				{ EVP_PKEY_RSA, "RSA" },
				{ EVP_PKEY_DSA, "DSA" },
				{ EVP_PKEY_DH,  "DH" },
				{ EVP_PKEY_EC,  "EC" },
			};
			unsigned i;

			if (NID_undef == (type = EVP_PKEY_type(OBJ_sn2nid(id)))) {
				for (i = 0; i < countof(types); i++) {
					if (strieq(id, types[i].sn)) {
						type = types[i].nid;
						break;
					}
				}
			}

			luaL_argcheck(L, type != NID_undef, 1, lua_pushfstring(L, "%s: invalid key type", id));
		}

		if (loadfield(L, 1, "bits", LUA_TNUMBER, &n)) {
			luaL_argcheck(L, n > 0 && n < UINT_MAX, 1, lua_pushfstring(L, "%f: `bits' invalid", n));
			bits = (unsigned)n;
		}

		if (loadfield(L, 1, "exp", LUA_TNUMBER, &n)) {
			luaL_argcheck(L, n > 0 && n < UINT_MAX, 1, lua_pushfstring(L, "%f: `exp' invalid", n));
			exp = (unsigned)n;
		}

		if (loadfield(L, 1, "curve", LUA_TSTRING, &id)) {
			curve = OBJ_sn2nid(id);
			luaL_argcheck(L, curve != NID_undef, 1, lua_pushfstring(L, "%s: invalid curve", id));
		}

creat:
		if (!(*ud = EVP_PKEY_new()))
			return auxL_error(L, auxL_EOPENSSL, "pkey.new");

		switch (EVP_PKEY_type(type)) {
		case EVP_PKEY_RSA: {
			RSA *rsa;

			if (!(rsa = RSA_generate_key(bits, exp, 0, 0)))
				return auxL_error(L, auxL_EOPENSSL, "pkey.new");

			EVP_PKEY_set1_RSA(*ud, rsa);

			RSA_free(rsa);

			break;
		}
		case EVP_PKEY_DSA: {
			DSA *dsa;

			if (!(dsa = DSA_generate_parameters(bits, 0, 0, 0, 0, 0, 0)))
				return auxL_error(L, auxL_EOPENSSL, "pkey.new");

			if (!DSA_generate_key(dsa)) {
				DSA_free(dsa);
				return auxL_error(L, auxL_EOPENSSL, "pkey.new");
			}

			EVP_PKEY_set1_DSA(*ud, dsa);

			DSA_free(dsa);

			break;
		}
		case EVP_PKEY_DH: {
			DH *dh;

			if (!(dh = DH_generate_parameters(bits, exp, 0, 0)))
				return auxL_error(L, auxL_EOPENSSL, "pkey.new");

			if (!DH_generate_key(dh)) {
				DH_free(dh);
				return auxL_error(L, auxL_EOPENSSL, "pkey.new");
			}

			EVP_PKEY_set1_DH(*ud, dh);

			DH_free(dh);

			break;
		}
#ifndef OPENSSL_NO_EC
		case EVP_PKEY_EC: {
			EC_GROUP *grp;
			EC_KEY *key;

			if (!(grp = EC_GROUP_new_by_curve_name(curve)))
				return auxL_error(L, auxL_EOPENSSL, "pkey.new");

			EC_GROUP_set_asn1_flag(grp, OPENSSL_EC_NAMED_CURVE);

			/* compressed points patented */
			EC_GROUP_set_point_conversion_form(grp, POINT_CONVERSION_UNCOMPRESSED);

			if (!(key = EC_KEY_new())) {
				EC_GROUP_free(grp);
				return auxL_error(L, auxL_EOPENSSL, "pkey.new");
			}

			EC_KEY_set_group(key, grp);

			EC_GROUP_free(grp);

			if (!EC_KEY_generate_key(key)) {
				EC_KEY_free(key);
				return auxL_error(L, auxL_EOPENSSL, "pkey.new");
			}

			EVP_PKEY_set1_EC_KEY(*ud, key);

			EC_KEY_free(key);

			break;
		}
#endif
		default:
			return luaL_error(L, "%d: unknown EVP base type (%d)", EVP_PKEY_type(type), type);
		} /* switch() */
	} else if (lua_isstring(L, 1)) {
		int type = optencoding(L, 2, "*", X509_ANY|X509_PEM|X509_DER);
		int pubonly = 0, prvtonly = 0;
		const char *opt, *data;
		size_t len;
		BIO *bio;
		EVP_PKEY *pub = NULL, *prvt = NULL;
		int goterr = 0;

		/* check if specified publickey or privatekey */
		if ((opt = luaL_optstring(L, 3, NULL))) {
			if (xtolower(opt[0]) == 'p' && xtolower(opt[1]) == 'u') {
				pubonly = 1;
			} else if (xtolower(opt[0]) == 'p' && xtolower(opt[1]) == 'r') {
				prvtonly = 1;
			} else {
				return luaL_argerror(L, 3, lua_pushfstring(L, "invalid option %s", opt));
			}
		}

		data = luaL_checklstring(L, 1, &len);

		if (!(bio = BIO_new_mem_buf((void *)data, len)))
			return auxL_error(L, auxL_EOPENSSL, "pkey.new");

		if (type == X509_PEM || type == X509_ANY) {
			if (!prvtonly && !pub) {
				/*
				 * BIO_reset is a rewind for read-only
				 * memory buffers. See mem_ctrl in
				 * crypto/bio/bss_mem.c of OpenSSL source.
				 */
				BIO_reset(bio);

				if (!(pub = PEM_read_bio_PUBKEY(bio, NULL, 0, "")))
					goterr = 1;
			}

			if (!pubonly && !prvt) {
				BIO_reset(bio);

				if (!(prvt = PEM_read_bio_PrivateKey(bio, NULL, 0, "")))
					goterr = 1;
			}
		}

		if (type == X509_DER || type == X509_ANY) {
			if (!prvtonly && !pub) {
				BIO_reset(bio);

				if (!(pub = d2i_PUBKEY_bio(bio, NULL)))
					goterr = 1;
			}

			if (!pubonly && !prvt) {
				BIO_reset(bio);

				if (!(prvt = d2i_PrivateKey_bio(bio, NULL)))
					goterr = 1;
			}
		}

		if (prvt) {
#if 0
			/* TODO: Determine if this is necessary. */
			if (pub && EVP_PKEY_missing_parameters(prvt)) {
				if (!EVP_PKEY_copy_parameters(prvt, pub)) {
					/*
					 * NOTE: It's not necessarily true
					 * that any internal errors were
					 * set. But we fixed pusherror() to
					 * handle that situation.
					 */
					goterr = 1;

					goto done;
				}
			}
#endif

			*ud = prvt;
			prvt = NULL;
		} else if (pub) {
			*ud = pub;
			pub = NULL;
		}
done:
		BIO_free(bio);

		if (pub)
			EVP_PKEY_free(pub);

		if (prvt)
			EVP_PKEY_free(prvt);

		if (!*ud) {
			if (goterr)
				return auxL_error(L, auxL_EOPENSSL, "pkey.new");

			/* we should never get here */
			return luaL_error(L, "failed to load key for some unexpected reason");
		} else if (goterr) {
			/* clean up our mess from testing input formats */
			ERR_clear_error();
		}
	} else {
		return luaL_error(L, "%s: unknown key initializer", lua_typename(L, lua_type(L, 1)));
	}

	return 1;
} /* pk_new() */


static int pk_interpose(lua_State *L) {
	return interpose(L, PKEY_CLASS);
} /* pk_interpose() */


static int pk_type(lua_State *L) {
	EVP_PKEY *key = checksimple(L, 1, PKEY_CLASS);
	int nid = key->type;

	auxL_pushnid(L, nid);

	return 1;
} /* pk_type() */


static int pk_setPublicKey(lua_State *L) {
	EVP_PKEY **key = luaL_checkudata(L, 1, PKEY_CLASS);
	const char *data;
	size_t len;
	BIO *bio;
	int type, ok = 0;

	data = luaL_checklstring(L, 2, &len);
	type = optencoding(L, 3, "*", X509_ANY|X509_PEM|X509_DER);

	if (!(bio = BIO_new_mem_buf((void *)data, len)))
		return auxL_error(L, auxL_EOPENSSL, "pkey.new");

	if (type == X509_ANY || type == X509_PEM) {
		ok = !!PEM_read_bio_PUBKEY(bio, key, 0, "");
	}

	if (!ok && (type == X509_ANY || type == X509_DER)) {
		ok = !!d2i_PUBKEY_bio(bio, key);
	}

	BIO_free(bio);

	if (!ok)
		return auxL_error(L, auxL_EOPENSSL, "pkey.new");

	lua_pushboolean(L, 1);

	return 1;
} /* pk_setPublicKey() */


static int pk_setPrivateKey(lua_State *L) {
	EVP_PKEY **key = luaL_checkudata(L, 1, PKEY_CLASS);
	const char *data;
	size_t len;
	BIO *bio;
	int type, ok = 0;

	data = luaL_checklstring(L, 2, &len);
	type = optencoding(L, 3, "*", X509_ANY|X509_PEM|X509_DER);

	if (!(bio = BIO_new_mem_buf((void *)data, len)))
		return auxL_error(L, auxL_EOPENSSL, "pkey.new");

	if (type == X509_ANY || type == X509_PEM) {
		ok = !!PEM_read_bio_PrivateKey(bio, key, 0, "");
	}

	if (!ok && (type == X509_ANY || type == X509_DER)) {
		ok = !!d2i_PrivateKey_bio(bio, key);
	}

	BIO_free(bio);

	if (!ok)
		return auxL_error(L, auxL_EOPENSSL, "pkey.new");

	lua_pushboolean(L, 1);

	return 1;
} /* pk_setPrivateKey() */


static int pk_sign(lua_State *L) {
	EVP_PKEY *key = checksimple(L, 1, PKEY_CLASS);
	EVP_MD_CTX *md = luaL_checkudata(L, 2, DIGEST_CLASS);
	luaL_Buffer B;
	unsigned n;

	if (LUAL_BUFFERSIZE < EVP_PKEY_size(key))
		return luaL_error(L, "pkey:sign: LUAL_BUFFERSIZE(%u) < EVP_PKEY_size(%u)", (unsigned)LUAL_BUFFERSIZE, (unsigned)EVP_PKEY_size(key));

	luaL_buffinit(L, &B);
	n = LUAL_BUFFERSIZE;

	if (!EVP_SignFinal(md, (void *)luaL_prepbuffer(&B), &n, key))
		return auxL_error(L, auxL_EOPENSSL, "pkey:sign");

	luaL_addsize(&B, n);
	luaL_pushresult(&B);

	return 1;
} /* pk_sign() */


static int pk_verify(lua_State *L) {
	EVP_PKEY *key = checksimple(L, 1, PKEY_CLASS);
	size_t len;
	const void *sig = luaL_checklstring(L, 2, &len);
	EVP_MD_CTX *md = luaL_checkudata(L, 3, DIGEST_CLASS);

	switch (EVP_VerifyFinal(md, sig, len, key)) {
	case 0: /* WRONG */
		ERR_clear_error();
		lua_pushboolean(L, 0);

		break;
	case 1: /* OK */
		lua_pushboolean(L, 1);

		break;
	default:
		return auxL_error(L, auxL_EOPENSSL, "pkey:verify");
	}

	return 1;
} /* pk_verify() */


static int pk_toPEM(lua_State *L) {
	EVP_PKEY *key = checksimple(L, 1, PKEY_CLASS);
	int top, i, ok;
	BIO *bio;
	char *pem;
	long len;

	if (1 == (top = lua_gettop(L))) {
		lua_pushstring(L, "publickey");
		++top;
	}

	bio = getbio(L);

	for (i = 2; i <= top; i++) {
		static const char *const opts[] = {
			"public", "PublicKey",
			"private", "PrivateKey",
//			"params", "Parameters",
			NULL,
		};

		switch (checkoption(L, i, NULL, opts)) {
		case 0: case 1: /* public, PublicKey */
			if (!PEM_write_bio_PUBKEY(bio, key))
				return auxL_error(L, auxL_EOPENSSL, "pkey:__tostring");

			len = BIO_get_mem_data(bio, &pem);
			lua_pushlstring(L, pem, len);

			BIO_reset(bio);
			break;
		case 2: case 3: /* private, PrivateKey */
			if (!PEM_write_bio_PrivateKey(bio, key, 0, 0, 0, 0, 0))
				return auxL_error(L, auxL_EOPENSSL, "pkey:__tostring");

			len = BIO_get_mem_data(bio, &pem);
			lua_pushlstring(L, pem, len);

			break;
#if 0
		case 4: case 5: /* params, Parameters */
			/* EVP_PKEY_base_id not in OS X */
			switch (EVP_PKEY_type(key->type)) {
			case EVP_PKEY_RSA:
				break;
			case EVP_PKEY_DSA: {
				DSA *dsa = EVP_PKEY_get1_DSA(key);

				ok = !!PEM_write_bio_DSAparams(bio, dsa);

				DSA_free(dsa);

				if (!ok)
					return auxL_error(L, auxL_EOPENSSL, "pkey:__tostring");

				break;
			}
			case EVP_PKEY_DH: {
				DH *dh = EVP_PKEY_get1_DH(key);

				ok = !!PEM_write_bio_DHparams(bio, dh);

				DH_free(dh);

				if (!ok)
					return auxL_error(L, auxL_EOPENSSL, "pkey:__tostring");

				break;
			}
#ifndef OPENSSL_NO_EC
			case EVP_PKEY_EC: {
				EC_KEY *ec = EVP_PKEY_get1_EC_KEY(key);
				const EC_GROUP *grp = EC_KEY_get0_group(ec);

				ok = !!PEM_write_bio_ECPKParameters(bio, grp);

				EC_KEY_free(ec);

				if (!ok)
					return auxL_error(L, auxL_EOPENSSL, "pkey:__tostring");

				break;
			}
#endif
			default:
				return luaL_error(L, "%d: unknown EVP base type", EVP_PKEY_type(key->type));
			}

			lua_pushlstring(L, pem, len);

			BIO_reset(bio);

			break;
#endif
		default:
			lua_pushnil(L);

			break;
		} /* switch() */
	} /* for() */

	return lua_gettop(L) - top;
} /* pk_toPEM() */


static int pk__tostring(lua_State *L) {
	EVP_PKEY *key = checksimple(L, 1, PKEY_CLASS);
	int type = optencoding(L, 2, "pem", X509_PEM|X509_DER);
	BIO *bio = getbio(L);
	char *data;
	long len;

	switch (type) {
	case X509_PEM:
		if (!PEM_write_bio_PUBKEY(bio, key))
			return auxL_error(L, auxL_EOPENSSL, "pkey:__tostring");
		break;
	case X509_DER:
		if (!i2d_PUBKEY_bio(bio, key))
			return auxL_error(L, auxL_EOPENSSL, "pkey:__tostring");
		break;
	} /* switch() */

	len = BIO_get_mem_data(bio, &data);

	lua_pushlstring(L, data, len);

	return 1;
} /* pk__tostring() */


static int pk__gc(lua_State *L) {
	EVP_PKEY **ud = luaL_checkudata(L, 1, PKEY_CLASS);

	if (*ud) {
		EVP_PKEY_free(*ud);
		*ud = NULL;
	}

	return 0;
} /* pk__gc() */


static const luaL_Reg pk_methods[] = {
	{ "type",          &pk_type },
	{ "setPublicKey",  &pk_setPublicKey },
	{ "setPrivateKey", &pk_setPrivateKey },
	{ "sign",          &pk_sign },
	{ "verify",        &pk_verify },
	{ "toPEM",         &pk_toPEM },
	{ NULL,            NULL },
};

static const luaL_Reg pk_metatable[] = {
	{ "__tostring", &pk__tostring },
	{ "__gc",       &pk__gc },
	{ NULL,         NULL },
};


static const luaL_Reg pk_globals[] = {
	{ "new",       &pk_new },
	{ "interpose", &pk_interpose },
	{ NULL,        NULL },
};

int luaopen_sec_pkey(lua_State *L) {
	initall(L);

	luaL_newlib(L, pk_globals);

	return 1;
} /* luaopen_sec_pkey() */


/*
 * Deprecated module name.
 */
int luaopen_sec_pubkey(lua_State *L) {
	return luaopen_sec_pkey(L);
} /* luaopen_sec_pubkey() */


/*
 * X509_NAME - openssl.x509.name
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static X509_NAME *xn_dup(lua_State *L, X509_NAME *name) {
	X509_NAME **ud = prepsimple(L, X509_NAME_CLASS);

	if (!(*ud = X509_NAME_dup(name)))
		auxL_error(L, auxL_EOPENSSL, "x509.name.dup");

	return *ud;
} /* xn_dup() */


static int xn_new(lua_State *L) {
	X509_NAME **ud = prepsimple(L, X509_NAME_CLASS);

	if (!(*ud = X509_NAME_new()))
		return auxL_error(L, auxL_EOPENSSL, "x509.name.new");

	return 1;
} /* xn_new() */


static int xn_interpose(lua_State *L) {
	return interpose(L, X509_NAME_CLASS);
} /* xn_interpose() */


static int xn_add(lua_State *L) {
	X509_NAME *name = checksimple(L, 1, X509_NAME_CLASS);
	const char *nid = luaL_checkstring(L, 2);
	size_t len;
	const char *txt = luaL_checklstring(L, 3, &len);
	ASN1_OBJECT *obj;
	int ok;

	if (!(obj = OBJ_txt2obj(nid, 0)))
		return luaL_error(L, "x509.name:add: %s: invalid NID", nid);

	ok = !!X509_NAME_add_entry_by_OBJ(name, obj, MBSTRING_ASC, (unsigned char *)txt, len, -1, 0);

	ASN1_OBJECT_free(obj);

	if (!ok)
		return auxL_error(L, auxL_EOPENSSL, "x509.name:add");

	lua_pushvalue(L, 1);

	return 1;
} /* xn_add() */


static int xn_all(lua_State *L) {
	X509_NAME *name = checksimple(L, 1, X509_NAME_CLASS);
	int count = X509_NAME_entry_count(name);
	X509_NAME_ENTRY *entry;
	ASN1_OBJECT *obj;
	const char *id;
	char txt[256];
	int i, nid, len;

	lua_newtable(L);

	for (i = 0; i < count; i++) {
		if (!(entry = X509_NAME_get_entry(name, i)))
			continue;

		lua_newtable(L);

		obj = X509_NAME_ENTRY_get_object(entry);
		nid = OBJ_obj2nid(obj);

		if (0 > (len = OBJ_obj2txt(txt, sizeof txt, obj, 1)))
			return auxL_error(L, auxL_EOPENSSL, "x509.name:all");

		lua_pushlstring(L, txt, len);

		if (nid != NID_undef && ((id = OBJ_nid2ln(nid)) || (id = OBJ_nid2sn(nid))))
			lua_pushstring(L, id);
		else
			lua_pushvalue(L, -1);

		if (nid != NID_undef && (id = OBJ_nid2sn(nid)))
			lua_pushstring(L, id);
		else
			lua_pushvalue(L, -1);

		lua_setfield(L, -4, "sn");
		lua_setfield(L, -3, "ln");
		lua_setfield(L, -2, "id");

		len = ASN1_STRING_length(X509_NAME_ENTRY_get_data(entry));
		lua_pushlstring(L, (char *)ASN1_STRING_data(X509_NAME_ENTRY_get_data(entry)), len);

		lua_setfield(L, -2, "blob");

		lua_rawseti(L, -2, i + 1);
	}

	return 1;
} /* xn_all() */


static int xn__next(lua_State *L) {
	X509_NAME *name = checksimple(L, lua_upvalueindex(1), X509_NAME_CLASS);
	X509_NAME_ENTRY *entry;
	ASN1_OBJECT *obj;
	const char *id;
	char txt[256];
	int i, n, nid, len;

	lua_settop(L, 0);

	i = lua_tointeger(L, lua_upvalueindex(2));
	n = X509_NAME_entry_count(name);

	while (i < n) {
		if (!(entry = X509_NAME_get_entry(name, i++)))
			continue;

		obj = X509_NAME_ENTRY_get_object(entry);

		if (!(len = auxS_obj2txt(txt, sizeof txt, obj)))
			return auxL_error(L, auxL_EOPENSSL, "x509.name:__pairs");
		lua_pushlstring(L, txt, len);

		len = ASN1_STRING_length(X509_NAME_ENTRY_get_data(entry));
		lua_pushlstring(L, (char *)ASN1_STRING_data(X509_NAME_ENTRY_get_data(entry)), len);

		break;
	}

	lua_pushinteger(L, i);
	lua_replace(L, lua_upvalueindex(2));

	return lua_gettop(L);
} /* xn__next() */

static int xn__pairs(lua_State *L) {
	lua_settop(L, 1);
	lua_pushinteger(L, 0);

	lua_pushcclosure(L, &xn__next, 2);

	return 1;
} /* xn__pairs() */


static int xn__gc(lua_State *L) {
	X509_NAME **ud = luaL_checkudata(L, 1, X509_NAME_CLASS);

	if (*ud) {
		X509_NAME_free(*ud);
		*ud = NULL;
	}

	return 0;
} /* xn__gc() */


static int xn__tostring(lua_State *L) {
	X509_NAME *name = checksimple(L, 1, X509_NAME_CLASS);
	char txt[1024] = { 0 };

	/* FIXME: oneline is deprecated */
	X509_NAME_oneline(name, txt, sizeof txt);

	lua_pushstring(L, txt);

	return 1;
} /* xn__tostring() */


static const luaL_Reg xn_methods[] = {
	{ "add", &xn_add },
	{ "all", &xn_all },
	{ NULL,  NULL },
};

static const luaL_Reg xn_metatable[] = {
	{ "__pairs",    &xn__pairs },
	{ "__gc",       &xn__gc },
	{ "__tostring", &xn__tostring },
	{ NULL,         NULL },
};


static const luaL_Reg xn_globals[] = {
	{ "new",       &xn_new },
	{ "interpose", &xn_interpose },
	{ NULL,        NULL },
};

int luaopen_sec_x509_name(lua_State *L) {
	initall(L);

	luaL_newlib(L, xn_globals);

	return 1;
} /* luaopen_sec_x509_name() */


/*
 * GENERAL_NAMES - openssl.x509.altname
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static GENERAL_NAMES *gn_dup(lua_State *L, GENERAL_NAMES *gens) {
	GENERAL_NAMES **ud = prepsimple(L, X509_GENS_CLASS);

	if (!(*ud = sk_GENERAL_NAME_dup(gens)))
		auxL_error(L, auxL_EOPENSSL, "x509.altname.dup");

	return *ud;
} /* gn_dup() */


static int gn_new(lua_State *L) {
	GENERAL_NAMES **ud = prepsimple(L, X509_GENS_CLASS);

	if (!(*ud = sk_GENERAL_NAME_new_null()))
		return auxL_error(L, auxL_EOPENSSL, "x509.altname.new");

	return 1;
} /* gn_new() */


static int gn_interpose(lua_State *L) {
	return interpose(L, X509_GENS_CLASS);
} /* gn_interpose() */


static int gn_checktype(lua_State *L, int index) {
	static const struct { int type; const char *name; } table[] = {
		{ GEN_EMAIL,   "RFC822Name" },
		{ GEN_EMAIL,   "RFC822" },
		{ GEN_EMAIL,   "email" },
		{ GEN_URI,     "UniformResourceIdentifier" },
		{ GEN_URI,     "URI" },
		{ GEN_DNS,     "DNSName" },
		{ GEN_DNS,     "DNS" },
		{ GEN_IPADD,   "IPAddress" },
		{ GEN_IPADD,   "IP" },
		{ GEN_DIRNAME, "DirName" },
	};
	const char *type = luaL_checkstring(L, index);
	unsigned i;

	for (i = 0; i < countof(table); i++) {
		if (strieq(table[i].name, type))
			return table[i].type;
	}

	return luaL_error(L, "%s: invalid type", type), 0;
} /* gn_checktype() */


static int gn_add(lua_State *L) {
	GENERAL_NAMES *gens = checksimple(L, 1, X509_GENS_CLASS);
	int type = gn_checktype(L, 2);
	X509_NAME *name;
	size_t len;
	const char *txt;
	GENERAL_NAME *gen = NULL;
	union { struct in6_addr in6; struct in_addr in; } ip;

	switch (type) {
	case GEN_DIRNAME:
		name = checksimple(L, 3, X509_NAME_CLASS);

		if (!(gen = GENERAL_NAME_new()))
			goto error;

		gen->type = type;

		if (!(gen->d.dirn = X509_NAME_dup(name)))
			goto error;

		break;
	case GEN_IPADD:
		txt = luaL_checkstring(L, 3);

		if (strchr(txt, ':')) {
			if (1 != inet_pton(AF_INET6, txt, &ip.in6))
				return luaL_error(L, "%s: invalid address", txt);

			txt = (char *)ip.in6.s6_addr;
			len = 16;
		} else {
			if (1 != inet_pton(AF_INET, txt, &ip.in))
				return luaL_error(L, "%s: invalid address", txt);

			txt = (char *)&ip.in.s_addr;
			len = 4;
		}

		goto text;
	default:
		txt = luaL_checklstring(L, 3, &len);
text:
		if (!(gen = GENERAL_NAME_new()))
			goto error;

		gen->type = type;

		if (!(gen->d.ia5 = M_ASN1_IA5STRING_new()))
			goto error;

		if (!ASN1_STRING_set(gen->d.ia5, (unsigned char *)txt, len))
			goto error;
		break;
	} /* switch() */

	sk_GENERAL_NAME_push(gens, gen);

	lua_pushvalue(L, 1);

	return 1;
error:
	GENERAL_NAME_free(gen);

	return auxL_error(L, auxL_EOPENSSL, "x509.altname:add");
} /* gn_add() */


#define GN_PUSHSTRING(L, o) \
	lua_pushlstring((L), (char *)M_ASN1_STRING_data((o)), M_ASN1_STRING_length((o)))

static int gn__next(lua_State *L) {
	GENERAL_NAMES *gens = checksimple(L, lua_upvalueindex(1), X509_GENS_CLASS);
	int i = lua_tointeger(L, lua_upvalueindex(2));
	int n = sk_GENERAL_NAME_num(gens);

	lua_settop(L, 0);

	while (i < n) {
		GENERAL_NAME *name;
		const char *txt;
		size_t len;
		union { struct in_addr in; struct in6_addr in6; } ip;
		char buf[INET6_ADDRSTRLEN + 1];
		int af;

		if (!(name = sk_GENERAL_NAME_value(gens, i++)))
			continue;

		switch (name->type) {
		case GEN_EMAIL:
			lua_pushstring(L, "email");
			GN_PUSHSTRING(L, name->d.rfc822Name);

			break;
		case GEN_URI:
			lua_pushstring(L, "URI");
			GN_PUSHSTRING(L, name->d.uniformResourceIdentifier);

			break;
		case GEN_DNS:
			lua_pushstring(L, "DNS");
			GN_PUSHSTRING(L, name->d.dNSName);

			break;
		case GEN_IPADD:
			txt = (char *)M_ASN1_STRING_data(name->d.iPAddress);
			len = M_ASN1_STRING_length(name->d.iPAddress);

			switch (len) {
			case 16:
				memcpy(ip.in6.s6_addr, txt, 16);
				af = AF_INET6;

				break;
			case 4:
				memcpy(&ip.in.s_addr, txt, 4);
				af = AF_INET;

				break;
			default:
				continue;
			}

			if (!(txt = inet_ntop(af, &ip, buf, sizeof buf)))
				continue;

			len = strlen(txt);

			lua_pushstring(L, "IP");
			lua_pushlstring(L, txt, len);

			break;
		case GEN_DIRNAME:
			lua_pushstring(L, "DirName");
			xn_dup(L, name->d.dirn);

			break;
		default:
			continue;
		} /* switch() */

		break;
	} /* while() */

	lua_pushinteger(L, i);
	lua_replace(L, lua_upvalueindex(2));

	return lua_gettop(L);
} /* gn__next() */

static int gn__pairs(lua_State *L) {
	lua_settop(L, 1);
	lua_pushinteger(L, 0);
	lua_pushcclosure(L, &gn__next, 2);

	return 1;
} /* gn__pairs() */


static int gn__gc(lua_State *L) {
	GENERAL_NAMES **ud = luaL_checkudata(L, 1, X509_GENS_CLASS);

	if (*ud) {
		sk_GENERAL_NAME_pop_free(*ud, GENERAL_NAME_free);
		*ud = NULL;
	}

	return 0;
} /* gn__gc() */


static const luaL_Reg gn_methods[] = {
	{ "add", &gn_add },
	{ NULL,  NULL },
};

static const luaL_Reg gn_metatable[] = {
	{ "__pairs", &gn__pairs },
	{ "__gc",    &gn__gc },
	{ NULL,      NULL },
};


static const luaL_Reg gn_globals[] = {
	{ "new",       &gn_new },
	{ "interpose", &gn_interpose },
	{ NULL,        NULL },
};

int luaopen_sec_x509_altname(lua_State *L) {
	initall(L);

	luaL_newlib(L, gn_globals);

	return 1;
} /* luaopen_sec_x509_altname() */


/*
 * X509_EXTENSION - openssl.x509.extension
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static _Bool xe_new_isder(const char *value, _Bool *crit) {
	if (!strcmp(value, "critical,DER"))
		return (*crit = 1), 1;
	if (!strcmp(value, "DER"))
		return (*crit = 0), 1;

	return 0;
} /* xs_new_isder() */

static int xe_new(lua_State *L) {
	const char *name = luaL_checkstring(L, 1);
	const char *value = luaL_checkstring(L, 2);
	ASN1_OBJECT *obj = NULL;
	ASN1_STRING *oct = NULL;
	CONF *conf = NULL;
	X509V3_CTX cbuf = { 0 }, *ctx = NULL;
	X509_EXTENSION **ud;

	lua_settop(L, 3);
	ud = prepsimple(L, X509_EXT_CLASS);

	if (!lua_isnil(L, 3)) {
		size_t len;
		const char *cdata = luaL_checklstring(L, 3, &len);
		_Bool crit;

		if (xe_new_isder(value, &crit)) {
			if (!(obj = OBJ_txt2obj(name, 0)))
				goto error;
			if (!(oct = ASN1_STRING_new()))
				goto error;
			if (!ASN1_STRING_set(oct, cdata, len))
				goto error;
			if (!(*ud = X509_EXTENSION_create_by_OBJ(NULL, obj, crit, oct)))
				goto error;

			ASN1_OBJECT_free(obj);
			ASN1_STRING_free(oct);

			return 1;
		}

		BIO *bio = getbio(L);
		if (BIO_puts(bio, cdata) < 0)
			goto error;

		if (!(conf = NCONF_new(NULL)))
			goto error;
		if (!NCONF_load_bio(conf, bio, NULL))
			goto error;

		ctx = &cbuf;
		X509V3_set_nconf(ctx, conf);
	}

	/*
	 * NOTE: AFAICT neither name nor value are modified. The API just
	 * doesn't have the proper const-qualifiers. See
	 * crypto/x509v3/v3_conf.c in OpenSSL.
	 *
	 * Also seems to be okay to pass NULL conf. Both NCONF_get_section
	 * and sk_CONF_VALUE_num can handle NULL arguments. See do_ext_nconf
	 * in v3_conf.c.
	 */
	if (!(*ud = X509V3_EXT_nconf(conf, ctx, (char *)name, (char *)value)))
		goto error;

	if (conf)
		NCONF_free(conf);

	return 1;
error:
	if (obj)
		ASN1_OBJECT_free(obj);
	if (oct)
		ASN1_STRING_free(oct);
	if (conf)
		NCONF_free(conf);

	return auxL_error(L, auxL_EOPENSSL, "x509.extension.new");
} /* xe_new() */


static int xe_interpose(lua_State *L) {
	return interpose(L, X509_EXT_CLASS);
} /* xe_interpose() */


static int xe_getID(lua_State *L) {
	X509_EXTENSION *ext = checksimple(L, 1, X509_EXT_CLASS);
	ASN1_OBJECT *obj = X509_EXTENSION_get0_object(ext);
	char txt[256];
	int len;

	if (!(len = auxS_obj2id(txt, sizeof txt, obj)))
		return auxL_error(L, auxL_EOPENSSL, "x509.extension:getID");

	lua_pushlstring(L, txt, len);

	return 1;
} /* xe_getID() */


static int xe_getName(lua_State *L) {
	X509_EXTENSION *ext = checksimple(L, 1, X509_EXT_CLASS);
	char txt[256];
	int len;

	if (!(len = auxS_obj2txt(txt, sizeof txt, X509_EXTENSION_get0_object(ext))))
		return auxL_error(L, auxL_EOPENSSL, "x509.extension:getName");

	lua_pushlstring(L, txt, len);

	return 1;
} /* xe_getName() */


static int xe_getShortName(lua_State *L) {
	X509_EXTENSION *ext = checksimple(L, 1, X509_EXT_CLASS);
	char txt[256];
	int len;

	if (!(len = auxS_obj2sn(txt, sizeof txt, X509_EXTENSION_get0_object(ext))))
		return 0;

	lua_pushlstring(L, txt, len);

	return 1;
} /* xe_getShortName() */


static int xe_getLongName(lua_State *L) {
	X509_EXTENSION *ext = checksimple(L, 1, X509_EXT_CLASS);
	char txt[256];
	int len;

	if (!(len = auxS_obj2ln(txt, sizeof txt, X509_EXTENSION_get0_object(ext))))
		return 0;

	lua_pushlstring(L, txt, len);

	return 1;
} /* xe_getLongName() */


static int xe_getData(lua_State *L) {
	ASN1_STRING *data = X509_EXTENSION_get0_data(checksimple(L, 1, X509_EXT_CLASS));

	lua_pushlstring(L, (char *)ASN1_STRING_data(data), ASN1_STRING_length(data));

	return 1;
} /* xe_getData() */


static int xe_getCritical(lua_State *L) {
	lua_pushboolean(L, X509_EXTENSION_get_critical(checksimple(L, 1, X509_EXT_CLASS)));

	return 1;
} /* xe_getCritical() */


static int xe_text(lua_State *L) {
	X509_EXTENSION *ext = checksimple(L, 1, X509_EXT_CLASS);
	unsigned long flags = auxL_optunsigned(L, 2, 0, 0, ULONG_MAX);
	int indent = auxL_optinteger(L, 3, 0, 0, INT_MAX);
	BIO *bio = getbio(L);
	char *data;
	size_t len;

	if (!X509V3_EXT_print(bio, ext, flags, indent))
		return auxL_error(L, auxL_EOPENSSL, "x509.extension.text");

	len = BIO_get_mem_data(bio, &data);

	lua_pushlstring(L, data, len);

	return 1;
} /* xe_text() */


static int xe__gc(lua_State *L) {
	X509_EXTENSION **ud = luaL_checkudata(L, 1, X509_EXT_CLASS);

	if (*ud) {
		X509_EXTENSION_free(*ud);
		*ud = NULL;
	}

	return 0;
} /* xe__gc() */


static const luaL_Reg xe_methods[] = {
	{ "getID",        &xe_getID },
	{ "getName",      &xe_getName },
	{ "getShortName", &xe_getShortName },
	{ "getLongName",  &xe_getLongName },
	{ "getData",      &xe_getData },
	{ "getCritical",  &xe_getCritical },
	{ "text",         &xe_text },
	{ NULL,           NULL },
};

static const luaL_Reg xe_metatable[] = {
	{ "__gc", &xe__gc },
	{ NULL,   NULL },
};


static const luaL_Reg xe_globals[] = {
	{ "new",       &xe_new },
	{ "interpose", &xe_interpose },
	{ NULL,        NULL },
};

static const auxL_IntegerReg xe_textopts[] = {
	{ "UNKNOWN_MASK", X509V3_EXT_UNKNOWN_MASK },
	{ "DEFAULT", X509V3_EXT_DEFAULT },
	{ "ERROR_UNKNOWN", X509V3_EXT_ERROR_UNKNOWN },
	{ "PARSE_UNKNOWN", X509V3_EXT_PARSE_UNKNOWN },
	{ "DUMP_UNKNOWN", X509V3_EXT_DUMP_UNKNOWN },
};

int luaopen_sec_x509_extension(lua_State *L) {
	initall(L);

	luaL_newlib(L, xe_globals);
	auxL_setintegers(L, xe_textopts);

	return 1;
} /* luaopen_sec_x509_extension() */


/*
 * X509 - openssl.x509.cert
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static int xc_new(lua_State *L) {
	const char *data;
	size_t len;
	X509 **ud;

	lua_settop(L, 2);

	ud = prepsimple(L, X509_CERT_CLASS);

	if ((data = luaL_optlstring(L, 1, NULL, &len))) {
		int type = optencoding(L, 2, "*", X509_ANY|X509_PEM|X509_DER);
		BIO *tmp;
		int ok = 0;

		if (!(tmp = BIO_new_mem_buf((char *)data, len)))
			return auxL_error(L, auxL_EOPENSSL, "x509.cert.new");

		if (type == X509_PEM || type == X509_ANY) {
			ok = !!(*ud = PEM_read_bio_X509(tmp, NULL, 0, "")); /* no password */
		}

		if (!ok && (type == X509_DER || type == X509_ANY)) {
			ok = !!(*ud = d2i_X509_bio(tmp, NULL));
		}

		BIO_free(tmp);

		if (!ok)
			return auxL_error(L, auxL_EOPENSSL, "x509.cert.new");
	} else {
		if (!(*ud = X509_new()))
			return auxL_error(L, auxL_EOPENSSL, "x509.cert.new");

		X509_gmtime_adj(X509_get_notBefore(*ud), 0);
		X509_gmtime_adj(X509_get_notAfter(*ud), 0);
	}

	return 1;
} /* xc_new() */


static int xc_interpose(lua_State *L) {
	return interpose(L, X509_CERT_CLASS);
} /* xc_interpose() */


static int xc_getVersion(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);

	lua_pushinteger(L, X509_get_version(crt) + 1);

	return 1;
} /* xc_getVersion() */


static int xc_setVersion(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);
	int version = luaL_checkint(L, 2);

	if (!X509_set_version(crt, version - 1))
		return luaL_error(L, "x509.cert:setVersion: %d: invalid version", version);

	lua_pushboolean(L, 1);

	return 1;
} /* xc_setVersion() */


static int xc_getSerial(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);
	BIGNUM *serial = bn_push(L);
	ASN1_INTEGER *i;

	if ((i = X509_get_serialNumber(crt))) {
		if (!ASN1_INTEGER_to_BN(i, serial))
			return auxL_error(L, auxL_EOPENSSL, "x509.cert:getSerial");
	}

	return 1;
} /* xc_getSerial() */


static int xc_setSerial(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);
	ASN1_INTEGER *serial;

	if (!(serial = BN_to_ASN1_INTEGER(checkbig(L, 2), NULL)))
		goto error;

	if (!X509_set_serialNumber(crt, serial))
		goto error;

	ASN1_INTEGER_free(serial);

	lua_pushboolean(L, 1);

	return 1;
error:
	ASN1_INTEGER_free(serial);

	return auxL_error(L, auxL_EOPENSSL, "x509.cert:setSerial");
} /* xc_setSerial() */


static int xc_digest(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);
	const char *type = luaL_optstring(L, 2, "sha1");
	int format = luaL_checkoption(L, 3, "x", (const char *[]){ "s", "x", "n", NULL });
	const EVP_MD *ctx;
	unsigned char md[EVP_MAX_MD_SIZE];
	unsigned len;

	lua_settop(L, 3); /* self, type, hex */

	if (!(ctx = EVP_get_digestbyname(type)))
		return luaL_error(L, "x509.cert:digest: %s: invalid digest type", type);

	X509_digest(crt, ctx, md, &len);

	switch (format) {
	case 2: {
		BIGNUM *bn = bn_push(L);

		if (!BN_bin2bn(md, len, bn))
			return auxL_error(L, auxL_EOPENSSL, "x509.cert:digest");

		break;
	}
	case 1: {
		static const unsigned char x[16] = "0123456789abcdef";
		luaL_Buffer B;
		unsigned i;

#if LUA_VERSION_NUM < 502
		luaL_buffinit(L, &B);
#else
		luaL_buffinitsize(L, &B, 2 * len);
#endif

		for (i = 0; i < len; i++) {
			luaL_addchar(&B, x[0x0f & (md[i] >> 4)]);
			luaL_addchar(&B, x[0x0f & (md[i] >> 0)]);
		}

		luaL_pushresult(&B);

		break;
	}
	default:
		lua_pushlstring(L, (const char *)md, len);

		break;
	} /* switch() */

	return 1;
} /* xc_digest() */


static _Bool isleap(int year) {
	if (year >= 0)
		return !(year % 4) && ((year % 100) || !(year % 400));
	else
		return isleap(-(year + 1));
} /* isleap() */


static int yday(int year, int mon, int mday) {
	static const int past[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
	int yday = past[CLAMP(mon, 0, 11)] + CLAMP(mday, 1, 31) - 1;

	return yday + (mon > 1 && isleap(year));
} /* yday() */


static int tm_yday(const struct tm *tm) {
	return (tm->tm_yday)? tm->tm_yday : yday(1900 + tm->tm_year, tm->tm_mon, tm->tm_mday);
} /* tm_yday() */


static int leaps(int year) {
	if (year >= 0)
		return (year / 400) + (year / 4) - (year / 100);
	else
		return -(leaps(-(year + 1)) + 1);
} /* leaps() */


static double tm2unix(const struct tm *tm, int gmtoff) {
	int year = tm->tm_year + 1900;
	double ts;

	ts = 86400.0 * 365.0 * (year - 1970);
	ts += 86400.0 * (leaps(year - 1) - leaps(1969));
	ts += 86400 * tm_yday(tm);
	ts += 3600 * tm->tm_hour;
	ts += 60 * tm->tm_min;
	ts += CLAMP(tm->tm_sec, 0, 59);
	ts += (year < 1970)? gmtoff : -gmtoff;

	return ts;
} /* tm2unix() */


static _Bool scan(int *i, char **cp, int n, int signok) {
	int sign = 1;

	*i = 0;

	if (signok) {
		if (**cp == '-') {
			sign = -1;
			++*cp;
		} else if (**cp == '+') {
			++*cp;
		}
	}

	while (n-- > 0) {
		if (**cp < '0' || **cp > '9')
			return 0;

		*i *= 10;
		*i += *(*cp)++ - '0';
	}

	*i *= sign;

	return 1;
} /* scan() */


static double timeutc(ASN1_TIME *time) {
	char buf[32] = "", *cp;
	struct tm tm = { 0 };
	int gmtoff = 0, year, i;

	if (!ASN1_TIME_check(time))
		return 0;

	cp = strncpy(buf, (const char *)ASN1_STRING_data((ASN1_STRING *)time), sizeof buf - 1);

	if (ASN1_STRING_type(time) == V_ASN1_GENERALIZEDTIME) {
		if (!scan(&year, &cp, 4, 1))
			goto badfmt;
	} else {
		if (!scan(&year, &cp, 2, 0))
			goto badfmt;
		year += (year < 50)? 2000 : 1999;
	}

	tm.tm_year = year - 1900;

	if (!scan(&i, &cp, 2, 0))
		goto badfmt;

	tm.tm_mon = CLAMP(i, 1, 12) - 1;

	if (!scan(&i, &cp, 2, 0))
		goto badfmt;

	tm.tm_mday = CLAMP(i, 1, 31);

	tm.tm_yday = yday(year, tm.tm_mon, tm.tm_mday);

	if (!scan(&i, &cp, 2, 0))
		goto badfmt;

	tm.tm_hour = CLAMP(i, 0, 23);

	if (!scan(&i, &cp, 2, 0))
		goto badfmt;

	tm.tm_min = CLAMP(i, 0, 59);

	if (*cp >= '0' && *cp <= '9') {
		if (!scan(&i, &cp, 2, 0))
			goto badfmt;

		tm.tm_sec = CLAMP(i, 0, 59);
	}

	if (*cp == '+' || *cp == '-') {
		int sign = (*cp++ == '-')? -1 : 1;
		int hh, mm;

		if (!scan(&hh, &cp, 2, 0) || !scan(&mm, &cp, 2, 0))
			goto badfmt;

		gmtoff = (CLAMP(hh, 0, 23) * 3600)
		       + (CLAMP(mm, 0, 59) * 60);

		gmtoff *= sign;
	}
	
	return tm2unix(&tm, gmtoff);
badfmt:
	return INFINITY;
} /* timeutc() */


static int xc_getLifetime(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);
	double begin = INFINITY, end = INFINITY;
	ASN1_TIME *time;

	if ((time = X509_get_notBefore(crt)))
		begin = timeutc(time);

	if ((time = X509_get_notAfter(crt)))
		end = timeutc(time);

	if (isfinite(begin))
		lua_pushnumber(L, begin);
	else
		lua_pushnil(L);

	if (isfinite(end))
		lua_pushnumber(L, end);
	else
		lua_pushnil(L);

	if (isfinite(begin) && isfinite(end) && begin <= end)
		lua_pushnumber(L, fabs(end - begin));
	else
		lua_pushnumber(L, 0.0);

	return 3;
} /* xc_getLifetime() */


static int xc_setLifetime(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);
	double ut;
	const char *dt;

	lua_settop(L, 3);

	if (lua_isnumber(L, 2)) {
		ut = lua_tonumber(L, 2);

		if (!ASN1_TIME_set(X509_get_notBefore(crt), ut))
			return auxL_error(L, auxL_EOPENSSL, "x509.cert:setLifetime");
#if 0
	} else if ((dt = luaL_optstring(L, 2, 0))) {
		if (!ASN1_TIME_set_string(X509_get_notBefore(crt), dt))
			return auxL_error(L, auxL_EOPENSSL, "x509.cert:setLifetime");
#endif
	}

	if (lua_isnumber(L, 3)) {
		ut = lua_tonumber(L, 3);

		if (!ASN1_TIME_set(X509_get_notAfter(crt), ut))
			return auxL_error(L, auxL_EOPENSSL, "x509.cert:setLifetime");
#if 0
	} else if ((dt = luaL_optstring(L, 3, 0))) {
		if (!ASN1_TIME_set_string(X509_get_notAfter(crt), dt))
			return auxL_error(L, auxL_EOPENSSL, "x509.cert:setLifetime");
#endif
	}

	lua_pushboolean(L, 1);

	return 1;
} /* xc_setLifetime() */


static int xc_getIssuer(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);
	X509_NAME *name;

	if (!(name = X509_get_issuer_name(crt)))
		return 0;

	xn_dup(L, name);

	return 1;
} /* xc_getIssuer() */


static int xc_setIssuer(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);
	X509_NAME *name = checksimple(L, 2, X509_NAME_CLASS);

	if (!X509_set_issuer_name(crt, name))
		return auxL_error(L, auxL_EOPENSSL, "x509.cert:setIssuer");

	lua_pushboolean(L, 1);

	return 1;
} /* xc_setIssuer() */


static int xc_getSubject(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);
	X509_NAME *name;

	if (!(name = X509_get_subject_name(crt)))
		return 0;

	xn_dup(L, name);

	return 1;
} /* xc_getSubject() */


static int xc_setSubject(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);
	X509_NAME *name = checksimple(L, 2, X509_NAME_CLASS);

	if (!X509_set_subject_name(crt, name))
		return auxL_error(L, auxL_EOPENSSL, "x509.cert:setSubject");

	lua_pushboolean(L, 1);

	return 1;
} /* xc_setSubject() */


static void xc_setCritical(X509 *crt, int nid, _Bool yes) {
	X509_EXTENSION *ext;
	int loc;

	if ((loc = X509_get_ext_by_NID(crt, nid, -1)) >= 0
	&&  (ext = X509_get_ext(crt, loc)))
		X509_EXTENSION_set_critical(ext, yes);
} /* xc_setCritical() */


static _Bool xc_getCritical(X509 *crt, int nid) {
	X509_EXTENSION *ext;
	int loc;

	if ((loc = X509_get_ext_by_NID(crt, nid, -1)) >= 0
	&&  (ext = X509_get_ext(crt, loc)))
		return X509_EXTENSION_get_critical(ext);
	else
		return 0;
} /* xc_getCritical() */


static int xc_getIssuerAlt(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);
	GENERAL_NAMES *gens;

	if (!(gens = X509_get_ext_d2i(crt, NID_issuer_alt_name, 0, 0)))
		return 0;

	gn_dup(L, gens);

	return 1;
} /* xc_getIssuerAlt() */


static int xc_setIssuerAlt(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);
	GENERAL_NAMES *gens = checksimple(L, 2, X509_GENS_CLASS);

	if (!X509_add1_ext_i2d(crt, NID_issuer_alt_name, gens, 0, X509V3_ADD_REPLACE))
		return auxL_error(L, auxL_EOPENSSL, "x509.altname:setIssuerAlt");

	lua_pushboolean(L, 1);

	return 1;
} /* xc_setIssuerAlt() */


static int xc_getSubjectAlt(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);
	GENERAL_NAMES *gens;

	if (!(gens = X509_get_ext_d2i(crt, NID_subject_alt_name, 0, 0)))
		return 0;

	gn_dup(L, gens);

	return 1;
} /* xc_getSubjectAlt() */


static int xc_setSubjectAlt(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);
	GENERAL_NAMES *gens = checksimple(L, 2, X509_GENS_CLASS);

	if (!X509_add1_ext_i2d(crt, NID_subject_alt_name, gens, 0, X509V3_ADD_REPLACE))
		return auxL_error(L, auxL_EOPENSSL, "x509.altname:setSubjectAlt");

	lua_pushboolean(L, 1);

	return 1;
} /* xc_setSubjectAlt() */


static int xc_getIssuerAltCritical(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);

	lua_pushboolean(L, xc_getCritical(crt, NID_issuer_alt_name));

	return 1;
} /* xc_getIssuerAltCritical() */


static int xc_setIssuerAltCritical(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);

	luaL_checkany(L, 2);
	xc_setCritical(crt, NID_issuer_alt_name, lua_toboolean(L, 2));

	lua_pushboolean(L, 1);

	return 1;
} /* xc_setIssuerAltCritical() */


static int xc_getSubjectAltCritical(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);

	lua_pushboolean(L, xc_getCritical(crt, NID_subject_alt_name));

	return 1;
} /* xc_getSubjectAltCritical() */


static int xc_setSubjectAltCritical(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);

	luaL_checkany(L, 2);
	xc_setCritical(crt, NID_subject_alt_name, lua_toboolean(L, 2));

	lua_pushboolean(L, 1);

	return 1;
} /* xc_setSubjectAltCritical() */


static int xc_getBasicConstraint(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);
	BASIC_CONSTRAINTS *bs;
	int CA, pathLen;

	if (!(bs = X509_get_ext_d2i(crt, NID_basic_constraints, 0, 0))) {
		/* FIXME: detect error or just non-existent */

		if (lua_gettop(L) > 1)
			return 0;

		lua_newtable(L);

		return 1;
	}

	CA = bs->ca;
	pathLen = ASN1_INTEGER_get(bs->pathlen);

	BASIC_CONSTRAINTS_free(bs);

	if (lua_gettop(L) > 1) {
		int n = 0, i, top;

		for (i = 2, top = lua_gettop(L); i <= top; i++) {
			switch (checkoption(L, i, 0, (const char *[]){ "CA", "pathLen", "pathLenConstraint", NULL })) {
			case 0:
				lua_pushboolean(L, CA);
				n++;
				break;
			case 1:
				/* FALL THROUGH */
			case 2:
				lua_pushinteger(L, pathLen);
				n++;
				break;
			}
		}

		return n;
	} else {
		lua_newtable(L);

		lua_pushboolean(L, CA);
		lua_setfield(L, -2, "CA");

		lua_pushinteger(L, pathLen);
		lua_setfield(L, -2, "pathLen");

		return 1;
	}
} /* xc_getBasicConstraint() */


static int xc_setBasicConstraint(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);
	BASIC_CONSTRAINTS *bs = 0;
	int CA = -1, pathLen = -1;
	int critical = 0;

	luaL_checkany(L, 2);

	if (lua_istable(L, 2)) {
		lua_getfield(L, 2, "CA");
		if (!lua_isnil(L, -1))
			CA = lua_toboolean(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, 2, "pathLen");
		pathLen = luaL_optint(L, -1, pathLen);
		lua_pop(L, 1);

		lua_getfield(L, 2, "pathLenConstraint");
		pathLen = luaL_optint(L, -1, pathLen);
		lua_pop(L, 1);

		if (!(bs = BASIC_CONSTRAINTS_new()))
			goto error;
	} else {
		lua_settop(L, 3);

		switch (checkoption(L, 2, 0, (const char *[]){ "CA", "pathLen", "pathLenConstraint", NULL })) {
		case 0:
			luaL_checktype(L, 3, LUA_TBOOLEAN);
			CA = lua_toboolean(L, 3);

			break;
		case 1:
			/* FALL THROUGH */
		case 2:
			pathLen = luaL_checkint(L, 3);

			break;
		}

		if (!(bs = X509_get_ext_d2i(crt, NID_basic_constraints, &critical, 0))) {
			/* FIXME: detect whether error or just non-existent */
			if (!(bs = BASIC_CONSTRAINTS_new()))
				goto error;
		}
	}

	if (CA != -1)
		bs->ca = CA;

	if (pathLen >= 0) {
		ASN1_INTEGER_free(bs->pathlen);

		if (!(bs->pathlen = M_ASN1_INTEGER_new()))
			goto error;

		if (!ASN1_INTEGER_set(bs->pathlen, pathLen))
			goto error;
	}

	if (!X509_add1_ext_i2d(crt, NID_basic_constraints, bs, critical, X509V3_ADD_REPLACE))
		goto error;

	BASIC_CONSTRAINTS_free(bs);

	lua_pushboolean(L, 1);

	return 1;
error:
	BASIC_CONSTRAINTS_free(bs);

	return auxL_error(L, auxL_EOPENSSL, "x509.cert:setBasicConstraint");
} /* xc_setBasicConstraint() */


static int xc_getBasicConstraintsCritical(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);

	lua_pushboolean(L, xc_getCritical(crt, NID_basic_constraints));

	return 1;
} /* xc_getBasicConstraintsCritical() */


static int xc_setBasicConstraintsCritical(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);

	luaL_checkany(L, 2);
	xc_setCritical(crt, NID_basic_constraints, lua_toboolean(L, 2));

	lua_pushboolean(L, 1);

	return 1;
} /* xc_setBasicConstraintsCritical() */


static int xc_addExtension(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);
	X509_EXTENSION *ext = checksimple(L, 2, X509_EXT_CLASS);

	/* NOTE: Will dup extension in X509v3_add_ext. */
	if (!X509_add_ext(crt, ext, -1))
		return auxL_error(L, auxL_EOPENSSL, "x509.cert:addExtension");

	lua_pushboolean(L, 1);

	return 1;
} /* xc_addExtension() */


static int xc_getExtension(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);
	X509_EXTENSION *ext = NULL, **ud;
	int i;

	luaL_checkany(L, 2);

	if (lua_type(L, 2) == LUA_TNUMBER) {
		/* NB: Lua 1-based indexing */
		i = auxL_checkinteger(L, 2, 1, INT_MAX) - 1;
	} else {
		ASN1_OBJECT *obj;

		if (!auxS_txt2obj(&obj, luaL_checkstring(L, 2))) {
			goto error;
		} else if (!obj) {
			goto undef;
		}

		i = X509_get_ext_by_OBJ(crt, obj, -1);

		ASN1_OBJECT_free(obj);
	}

	ud = prepsimple(L, X509_EXT_CLASS);

	if (i < 0 || !(ext = X509_get0_ext(crt, i)))
		goto undef;

	if (!(*ud = X509_EXTENSION_dup(ext)))
		goto error;

	return 1;
undef:
	return 0;
error:
	return auxL_error(L, auxL_EOPENSSL, "x509.cert:getExtension");
} /* xc_getExtension() */


static int xc_getExtensionCount(lua_State *L) {
	auxL_pushinteger(L, X509_get_ext_count(checksimple(L, 1, X509_CERT_CLASS)));

	return 1;
} /* xc_getExtensionCount() */


static int xc_isIssuedBy(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);
	X509 *issuer = checksimple(L, 2, X509_CERT_CLASS);
	EVP_PKEY *key;
	int ok, why = 0;

	ERR_clear_error();

	if (X509_V_OK != (why = X509_check_issued(issuer, crt)))
		goto done;

	if (!(key = X509_get_pubkey(issuer))) {
		why = X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY;
		goto done;
	}

	ok = (1 == X509_verify(crt, key));

	EVP_PKEY_free(key);

	if (!ok)
		why = X509_V_ERR_CERT_SIGNATURE_FAILURE;

done:
	if (why != X509_V_OK) {
		lua_pushboolean(L, 0);
		lua_pushstring(L, X509_verify_cert_error_string(why));

		return 2;
	} else {
		lua_pushboolean(L, 1);

		return 1;
	}
} /* xc_isIssuedBy() */


static int xc_getPublicKey(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);
	EVP_PKEY **key = prepsimple(L, PKEY_CLASS);

	if (!(*key = X509_get_pubkey(crt)))
		return auxL_error(L, auxL_EOPENSSL, "x509.cert:getPublicKey");

	return 1;
} /* xc_getPublicKey() */


static int xc_setPublicKey(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);
	EVP_PKEY *key = checksimple(L, 2, PKEY_CLASS);

	if (!X509_set_pubkey(crt, key))
		return auxL_error(L, auxL_EOPENSSL, "x509.cert:setPublicKey");

	lua_pushboolean(L, 1);

	return 1;
} /* xc_setPublicKey() */


static int xc_getPublicKeyDigest(lua_State *L) {
	ASN1_BIT_STRING *pk = X509_get0_pubkey_bitstr(checksimple(L, 1, X509_CERT_CLASS));
	const char *id = luaL_optstring(L, 2, "sha1");
	const EVP_MD *md;
	unsigned char digest[EVP_MAX_MD_SIZE];
	unsigned int len;

	if (!(md = EVP_get_digestbyname(id)))
		return luaL_error(L, "x509.cert:getPublicKeyDigest: %s: invalid digest type", id);

	if (!EVP_Digest(pk->data, pk->length, digest, &len, md, NULL))
		return auxL_error(L, auxL_EOPENSSL, "x509.cert:getPublicKeyDigest");

	lua_pushlstring(L, (char *)digest, len);

	return 1;
} /* xc_getPublicKeyDigest() */


static const EVP_MD *xc_signature(lua_State *L, int index, EVP_PKEY *key) {
	const char *id;
	const EVP_MD *md;

	if ((id = luaL_optstring(L, index, NULL)))
		return ((md = EVP_get_digestbyname(id)))? md : EVP_md_null();

	switch (EVP_PKEY_type(key->type)) {
	case EVP_PKEY_RSA:
		return EVP_sha1();
	case EVP_PKEY_DSA:
		return EVP_dss1();
	case EVP_PKEY_EC:
		return EVP_ecdsa();
	default:
		return EVP_md_null();
	}
} /* xc_signature() */

static int xc_sign(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);
	EVP_PKEY *key = checksimple(L, 2, PKEY_CLASS);

	if (!X509_sign(crt, key, xc_signature(L, 3, key)))
		return auxL_error(L, auxL_EOPENSSL, "x509.cert:sign");

	lua_pushboolean(L, 1);

	return 1;
} /* xc_sign() */


static int xc_text(lua_State *L) {
	static const struct { const char *kw; unsigned int flag; } map[] = {
		{ "no_header", X509_FLAG_NO_HEADER },
		{ "no_version", X509_FLAG_NO_VERSION },
		{ "no_serial", X509_FLAG_NO_SERIAL },
		{ "no_signame", X509_FLAG_NO_SIGNAME },
		{ "no_validity", X509_FLAG_NO_VALIDITY },
		{ "no_subject", X509_FLAG_NO_SUBJECT },
		{ "no_issuer", X509_FLAG_NO_ISSUER },
		{ "no_pubkey", X509_FLAG_NO_PUBKEY },
		{ "no_extensions", X509_FLAG_NO_EXTENSIONS },
		{ "no_sigdump", X509_FLAG_NO_SIGDUMP },
		{ "no_aux", X509_FLAG_NO_AUX },
		{ "no_attributes", X509_FLAG_NO_ATTRIBUTES },
		{ "ext_default", X509V3_EXT_DEFAULT },
		{ "ext_error", X509V3_EXT_ERROR_UNKNOWN },
		{ "ext_parse", X509V3_EXT_PARSE_UNKNOWN },
		{ "ext_dump", X509V3_EXT_DUMP_UNKNOWN }
	};

	lua_settop(L, 2);

	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);

	unsigned int flags = 0;
	const char *kw;
	int found;
	unsigned int i;

	BIO *bio = getbio(L);
	char *data;
	long len;

	if (!lua_isnil(L, 2)) {
		lua_pushnil(L);
		while (lua_next(L, 2)) {
			kw = luaL_checkstring(L, -1);
			found = 0;
			for (i = 0; i < countof(map); i++)
				if (!strcmp(kw, map[i].kw)) {
					flags |= map[i].flag;
					found = 1;
				}
			if (!found)
				luaL_argerror(L, 2, lua_pushfstring(L, "invalid flag: %s", kw));
			lua_pop(L, 1);
		}
	}

	if (!X509_print_ex(bio, crt, 0, flags))
		return auxL_error(L, auxL_EOPENSSL, "x509.cert:text");

	len = BIO_get_mem_data(bio, &data);

	lua_pushlstring(L, data, len);

	return 1;
} /* xc_text() */


static int xc__tostring(lua_State *L) {
	X509 *crt = checksimple(L, 1, X509_CERT_CLASS);
	int type = optencoding(L, 2, "pem", X509_PEM|X509_DER);
	BIO *bio = getbio(L);
	char *data;
	long len;

	switch (type) {
	case X509_PEM:
		if (!PEM_write_bio_X509(bio, crt))
			return auxL_error(L, auxL_EOPENSSL, "x509.cert:__tostring");
		break;
	case X509_DER:
		if (!i2d_X509_bio(bio, crt))
			return auxL_error(L, auxL_EOPENSSL, "x509.cert:__tostring");
		break;
	} /* switch() */

	len = BIO_get_mem_data(bio, &data);

	lua_pushlstring(L, data, len);

	return 1;
} /* xc__tostring() */


static int xc__gc(lua_State *L) {
	X509 **ud = luaL_checkudata(L, 1, X509_CERT_CLASS);

	if (*ud) {
		X509_free(*ud);
		*ud = NULL;
	}

	return 0;
} /* xc__gc() */


static const luaL_Reg xc_methods[] = {
	{ "getVersion",    &xc_getVersion },
	{ "setVersion",    &xc_setVersion },
	{ "getSerial",     &xc_getSerial },
	{ "setSerial",     &xc_setSerial },
	{ "digest",        &xc_digest },
	{ "getLifetime",   &xc_getLifetime },
	{ "setLifetime",   &xc_setLifetime },
	{ "getIssuer",     &xc_getIssuer },
	{ "setIssuer",     &xc_setIssuer },
	{ "getSubject",    &xc_getSubject },
	{ "setSubject",    &xc_setSubject },
	{ "getIssuerAlt",  &xc_getIssuerAlt },
	{ "setIssuerAlt",  &xc_setIssuerAlt },
	{ "getSubjectAlt", &xc_getSubjectAlt },
	{ "setSubjectAlt", &xc_setSubjectAlt },
	{ "getIssuerAltCritical",  &xc_getIssuerAltCritical },
	{ "setIssuerAltCritical",  &xc_setIssuerAltCritical },
	{ "getSubjectAltCritical", &xc_getSubjectAltCritical },
	{ "setSubjectAltCritical", &xc_setSubjectAltCritical },
	{ "getBasicConstraints", &xc_getBasicConstraint },
	{ "getBasicConstraint",  &xc_getBasicConstraint },
	{ "setBasicConstraints", &xc_setBasicConstraint },
	{ "setBasicConstraint",  &xc_setBasicConstraint },
	{ "getBasicConstraintsCritical", &xc_getBasicConstraintsCritical },
	{ "setBasicConstraintsCritical", &xc_setBasicConstraintsCritical },
	{ "addExtension",  &xc_addExtension },
	{ "getExtension",  &xc_getExtension },
	{ "getExtensionCount", &xc_getExtensionCount },
	{ "isIssuedBy",    &xc_isIssuedBy },
	{ "getPublicKey",  &xc_getPublicKey },
	{ "setPublicKey",  &xc_setPublicKey },
	{ "getPublicKeyDigest", &xc_getPublicKeyDigest },
	{ "sign",          &xc_sign },
	{ "text",          &xc_text },
	{ "tostring",      &xc__tostring },
	{ NULL,            NULL },
};

static const luaL_Reg xc_metatable[] = {
	{ "__tostring", &xc__tostring },
	{ "__gc",       &xc__gc },
	{ NULL,         NULL },
};


static const luaL_Reg xc_globals[] = {
	{ "new",       &xc_new },
	{ "interpose", &xc_interpose },
	{ NULL,        NULL },
};

int luaopen_sec_x509_cert(lua_State *L) {
	initall(L);

	luaL_newlib(L, xc_globals);

	return 1;
} /* luaopen_sec_x509_cert() */


/*
 * X509_REQ - openssl.x509.csr
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static int xr_new(lua_State *L) {
	const char *data;
	size_t len;
	X509_REQ **ud;
	X509 *crt;

	lua_settop(L, 2);

	ud = prepsimple(L, X509_CSR_CLASS);

	if ((crt = testsimple(L, 1, X509_CERT_CLASS))) {
		if (!(*ud = X509_to_X509_REQ(crt, 0, 0)))
			return auxL_error(L, auxL_EOPENSSL, "x509.csr.new");
	} else if ((data = luaL_optlstring(L, 1, NULL, &len))) {
		int type = optencoding(L, 2, "*", X509_ANY|X509_PEM|X509_DER);
		BIO *tmp;
		int ok = 0;

		if (!(tmp = BIO_new_mem_buf((char *)data, len)))
			return auxL_error(L, auxL_EOPENSSL, "x509.csr.new");

		if (type == X509_PEM || type == X509_ANY) {
			ok = !!(*ud = PEM_read_bio_X509_REQ(tmp, NULL, 0, "")); /* no password */
		}

		if (!ok && (type == X509_DER || type == X509_ANY)) {
			ok = !!(*ud = d2i_X509_REQ_bio(tmp, NULL));
		}

		BIO_free(tmp);

		if (!ok)
			return auxL_error(L, auxL_EOPENSSL, "x509.csr.new");
	} else {
		if (!(*ud = X509_REQ_new()))
			return auxL_error(L, auxL_EOPENSSL, "x509.csr.new");
	}

	return 1;
} /* xr_new() */


static int xr_interpose(lua_State *L) {
	return interpose(L, X509_CSR_CLASS);
} /* xr_interpose() */


static int xr_getVersion(lua_State *L) {
	X509_REQ *csr = checksimple(L, 1, X509_CSR_CLASS);

	lua_pushinteger(L, X509_REQ_get_version(csr) + 1);

	return 1;
} /* xr_getVersion() */


static int xr_setVersion(lua_State *L) {
	X509_REQ *csr = checksimple(L, 1, X509_CSR_CLASS);
	int version = luaL_checkint(L, 2);

	if (!X509_REQ_set_version(csr, version - 1))
		return luaL_error(L, "x509.csr:setVersion: %d: invalid version", version);

	lua_pushboolean(L, 1);

	return 1;
} /* xr_setVersion() */


static int xr_getSubject(lua_State *L) {
	X509_REQ *crt = checksimple(L, 1, X509_CSR_CLASS);
	X509_NAME *name;

	if (!(name = X509_REQ_get_subject_name(crt)))
		return 0;

	xn_dup(L, name);

	return 1;
} /* xr_getSubject() */


static int xr_setSubject(lua_State *L) {
	X509_REQ *csr = checksimple(L, 1, X509_CSR_CLASS);
	X509_NAME *name = checksimple(L, 2, X509_NAME_CLASS);

	if (!X509_REQ_set_subject_name(csr, name))
		return auxL_error(L, auxL_EOPENSSL, "x509.csr:setSubject");

	lua_pushboolean(L, 1);

	return 1;
} /* xr_setSubject() */


static int xr_getPublicKey(lua_State *L) {
	X509_REQ *csr = checksimple(L, 1, X509_CSR_CLASS);
	EVP_PKEY **key = prepsimple(L, PKEY_CLASS);

	if (!(*key = X509_REQ_get_pubkey(csr)))
		return auxL_error(L, auxL_EOPENSSL, "x509.cert:getPublicKey");

	return 1;
} /* xr_getPublicKey() */


static int xr_setPublicKey(lua_State *L) {
	X509_REQ *csr = checksimple(L, 1, X509_CSR_CLASS);
	EVP_PKEY *key = checksimple(L, 2, PKEY_CLASS);

	if (!X509_REQ_set_pubkey(csr, key))
		return auxL_error(L, auxL_EOPENSSL, "x509.csr:setPublicKey");

	lua_pushboolean(L, 1);

	return 1;
} /* xr_setPublicKey() */


static int xr_sign(lua_State *L) {
	X509_REQ *csr = checksimple(L, 1, X509_CSR_CLASS);
	EVP_PKEY *key = checksimple(L, 2, PKEY_CLASS);

	if (!X509_REQ_sign(csr, key, xc_signature(L, 3, key)))
		return auxL_error(L, auxL_EOPENSSL, "x509.csr:sign");

	lua_pushboolean(L, 1);

	return 1;
} /* xr_sign() */


static int xr__tostring(lua_State *L) {
	X509_REQ *csr = checksimple(L, 1, X509_CSR_CLASS);
	int type = optencoding(L, 2, "pem", X509_PEM|X509_DER);
	BIO *bio = getbio(L);
	char *data;
	long len;

	switch (type) {
	case X509_PEM:
		if (!PEM_write_bio_X509_REQ(bio, csr))
			return auxL_error(L, auxL_EOPENSSL, "x509.csr:__tostring");
		break;
	case X509_DER:
		if (!i2d_X509_REQ_bio(bio, csr))
			return auxL_error(L, auxL_EOPENSSL, "x509.csr:__tostring");
		break;
	} /* switch() */

	len = BIO_get_mem_data(bio, &data);

	lua_pushlstring(L, data, len);

	return 1;
} /* xr__tostring() */


static int xr__gc(lua_State *L) {
	X509_REQ **ud = luaL_checkudata(L, 1, X509_CSR_CLASS);

	if (*ud) {
		X509_REQ_free(*ud);
		*ud = NULL;
	}

	return 0;
} /* xr__gc() */

static const luaL_Reg xr_methods[] = {
	{ "getVersion",   &xr_getVersion },
	{ "setVersion",   &xr_setVersion },
	{ "getSubject",   &xr_getSubject },
	{ "setSubject",   &xr_setSubject },
	{ "getPublicKey", &xr_getPublicKey },
	{ "setPublicKey", &xr_setPublicKey },
	{ "sign",         &xr_sign },
	{ "tostring",     &xr__tostring },
	{ NULL,           NULL },
};

static const luaL_Reg xr_metatable[] = {
	{ "__tostring", &xr__tostring },
	{ "__gc",       &xr__gc },
	{ NULL,         NULL },
};


static const luaL_Reg xr_globals[] = {
	{ "new",       &xr_new },
	{ "interpose", &xr_interpose },
	{ NULL,        NULL },
};

int luaopen_sec_x509_csr(lua_State *L) {
	initall(L);

	luaL_newlib(L, xr_globals);

	return 1;
} /* luaopen_sec_x509_csr() */


/*
 * X509_CRL - openssl.x509.crl
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static int xx_new(lua_State *L) {
	const char *data;
	size_t len;
	X509_CRL **ud;

	lua_settop(L, 2);

	ud = prepsimple(L, X509_CRL_CLASS);

	if ((data = luaL_optlstring(L, 1, NULL, &len))) {
		int type = optencoding(L, 2, "*", X509_ANY|X509_PEM|X509_DER);
		BIO *tmp;
		int ok = 0;

		if (!(tmp = BIO_new_mem_buf((char *)data, len)))
			return auxL_error(L, auxL_EOPENSSL, "x509.crl.new");

		if (type == X509_PEM || type == X509_ANY) {
			ok = !!(*ud = PEM_read_bio_X509_CRL(tmp, NULL, 0, "")); /* no password */
		}

		if (!ok && (type == X509_DER || type == X509_ANY)) {
			ok = !!(*ud = d2i_X509_CRL_bio(tmp, NULL));
		}

		BIO_free(tmp);

		if (!ok)
			return auxL_error(L, auxL_EOPENSSL, "x509.crl.new");
	} else {
		if (!(*ud = X509_CRL_new()))
			return auxL_error(L, auxL_EOPENSSL, "x509.crl.new");

		X509_gmtime_adj(X509_CRL_get_lastUpdate(*ud), 0);
	}

	return 1;
} /* xx_new() */


static int xx_interpose(lua_State *L) {
	return interpose(L, X509_CRL_CLASS);
} /* xx_interpose() */


static int xx_getVersion(lua_State *L) {
	X509_CRL *crl = checksimple(L, 1, X509_CRL_CLASS);

	lua_pushinteger(L, X509_CRL_get_version(crl) + 1);

	return 1;
} /* xx_getVersion() */


static int xx_setVersion(lua_State *L) {
	X509_CRL *crl = checksimple(L, 1, X509_CRL_CLASS);
	int version = luaL_checkint(L, 2);

	if (!X509_CRL_set_version(crl, version - 1))
		return luaL_error(L, "x509.crl:setVersion: %d: invalid version", version);

	lua_pushboolean(L, 1);

	return 1;
} /* xx_setVersion() */


static int xx_getLastUpdate(lua_State *L) {
	X509_CRL *crl = checksimple(L, 1, X509_CRL_CLASS);
	double updated = INFINITY;
	ASN1_TIME *time;

	if ((time = X509_CRL_get_lastUpdate(crl)))
		updated = timeutc(time);

	if (isfinite(updated))
		lua_pushnumber(L, updated);
	else
		lua_pushnil(L);

	return 1;
} /* xx_getLastUpdate() */


static int xx_setLastUpdate(lua_State *L) {
	X509_CRL *crl = checksimple(L, 1, X509_CRL_CLASS);
	double updated = luaL_checknumber(L, 2);

	/* lastUpdate always present */
	if (!ASN1_TIME_set(X509_CRL_get_lastUpdate(crl), updated))
		return auxL_error(L, auxL_EOPENSSL, "x509.crl:setLastUpdate");

	lua_pushboolean(L, 1);

	return 1;
} /* xx_setLastUpdate() */


static int xx_getNextUpdate(lua_State *L) {
	X509_CRL *crl = checksimple(L, 1, X509_CRL_CLASS);
	double updateby = INFINITY;
	ASN1_TIME *time;

	if ((time = X509_CRL_get_nextUpdate(crl)))
		updateby = timeutc(time);

	if (isfinite(updateby))
		lua_pushnumber(L, 1);
	else
		lua_pushnil(L);

	return 1;
} /* xx_getNextUpdate() */


static int xx_setNextUpdate(lua_State *L) {
	X509_CRL *crl = checksimple(L, 1, X509_CRL_CLASS);
	double updateby = luaL_checknumber(L, 2);
	ASN1_TIME *time = NULL;

	if (X509_CRL_get_nextUpdate(crl)) {
		if (!ASN1_TIME_set(X509_CRL_get_nextUpdate(crl), updateby))
			goto error;
	} else {
		if (!(time = ASN1_TIME_new()))
			goto error;

		if (!(ASN1_TIME_set(time, updateby)))
			goto error;

		if (!X509_CRL_set_nextUpdate(crl, time))
			goto error;

		time = NULL;
	}

	lua_pushboolean(L, 1);

	return 1;
error:
	if (time)
		ASN1_TIME_free(time);

	return auxL_error(L, auxL_EOPENSSL, "x509.crl:setNextUpdate");
} /* xx_setNextUpdate() */


static int xx_getIssuer(lua_State *L) {
	X509_CRL *crl = checksimple(L, 1, X509_CRL_CLASS);
	X509_NAME *name;

	if (!(name = X509_CRL_get_issuer(crl)))
		return 0;

	xn_dup(L, name);

	return 1;
} /* xx_getIssuer() */


static int xx_setIssuer(lua_State *L) {
	X509_CRL *crl = checksimple(L, 1, X509_CRL_CLASS);
	X509_NAME *name = checksimple(L, 2, X509_NAME_CLASS);

	if (!X509_CRL_set_issuer_name(crl, name))
		return auxL_error(L, auxL_EOPENSSL, "x509.crl:setIssuer");

	lua_pushboolean(L, 1);

	return 1;
} /* xx_setIssuer() */


static int xx_add(lua_State *L) {
	X509_CRL *crl = checksimple(L, 1, X509_CRL_CLASS);
	BIGNUM *bn = checkbig(L, 2);
	double ut = luaL_optnumber(L, 3, time(NULL));
	X509_REVOKED *rev = NULL;
	ASN1_INTEGER *serial = NULL;
	ASN1_TIME *date = NULL;

	if (!(rev = X509_REVOKED_new()))
		goto error;

	if (!(serial = BN_to_ASN1_INTEGER(bn, NULL)))
		goto error;

	if (!X509_REVOKED_set_serialNumber(rev, serial)) /* duplicates serial */
		goto error;

	ASN1_INTEGER_free(serial);
	serial = NULL;

	if (!(date = ASN1_TIME_new()))
		goto error;

	if (!ASN1_TIME_set(date, ut))
		goto error;

	if (!X509_REVOKED_set_revocationDate(rev, date)) /* duplicates date */
		goto error;

	ASN1_TIME_free(date);
	date = NULL;

	if (!X509_CRL_add0_revoked(crl, rev)) /* takes ownership of rev */
		goto error;

	lua_pushboolean(L, 1);

	return 1;
error:
	if (date)
		ASN1_TIME_free(date);
	if (serial)
		ASN1_INTEGER_free(serial);
	if (rev)
		X509_REVOKED_free(rev);

	return auxL_error(L, auxL_EOPENSSL, "x509.crl:add");
} /* xx_add() */


static int xx_addExtension(lua_State *L) {
	X509_CRL *crl = checksimple(L, 1, X509_CRL_CLASS);
	X509_EXTENSION *ext = checksimple(L, 2, X509_EXT_CLASS);

	/* NOTE: Will dup extension in X509v3_add_ext. */
	if (!X509_CRL_add_ext(crl, ext, -1))
		return auxL_error(L, auxL_EOPENSSL, "x509.crl:addExtension");

	lua_pushboolean(L, 1);

	return 1;
} /* xx_addExtension() */


static int xx_getExtension(lua_State *L) {
	X509_CRL *crl = checksimple(L, 1, X509_CRL_CLASS);
	X509_EXTENSION *ext = NULL, **ud;
	int i;

	luaL_checkany(L, 2);

	if (lua_type(L, 2) == LUA_TNUMBER) {
		/* NB: Lua 1-based indexing */
		i = auxL_checkinteger(L, 2, 1, INT_MAX) - 1;
	} else {
		ASN1_OBJECT *obj;

		if (!auxS_txt2obj(&obj, luaL_checkstring(L, 2))) {
			goto error;
		} else if (!obj) {
			goto undef;
		}

		i = X509_CRL_get_ext_by_OBJ(crl, obj, -1);

		ASN1_OBJECT_free(obj);
	}

	ud = prepsimple(L, X509_EXT_CLASS);

	if (i < 0 || !(ext = X509_CRL_get0_ext(crl, i)))
		goto undef;

	if (!(*ud = X509_EXTENSION_dup(ext)))
		goto error;

	return 1;
undef:
	return 0;
error:
	return auxL_error(L, auxL_EOPENSSL, "x509.crl:getExtension");
} /* xx_getExtension() */


static int xx_getExtensionCount(lua_State *L) {
	auxL_pushinteger(L, X509_CRL_get_ext_count(checksimple(L, 1, X509_CRL_CLASS)));

	return 1;
} /* xx_getExtensionCount() */


static int xx_sign(lua_State *L) {
	X509_CRL *crl = checksimple(L, 1, X509_CRL_CLASS);
	EVP_PKEY *key = checksimple(L, 2, PKEY_CLASS);

	if (!X509_CRL_sign(crl, key, xc_signature(L, 3, key)))
		return auxL_error(L, auxL_EOPENSSL, "x509.crl:sign");

	lua_pushboolean(L, 1);

	return 1;
} /* xx_sign() */


static int xx_text(lua_State *L) {
	X509_CRL *crl = checksimple(L, 1, X509_CRL_CLASS);

	BIO *bio = getbio(L);
	char *data;
	long len;

	if (!X509_CRL_print(bio, crl))
		return auxL_error(L, auxL_EOPENSSL, "x509.crl:text");

	len = BIO_get_mem_data(bio, &data);

	lua_pushlstring(L, data, len);

	return 1;
} /* xx_text() */


static int xx__tostring(lua_State *L) {
	X509_CRL *crl = checksimple(L, 1, X509_CRL_CLASS);
	int type = optencoding(L, 2, "pem", X509_PEM|X509_DER);
	BIO *bio = getbio(L);
	char *data;
	long len;

	switch (type) {
	case X509_PEM:
		if (!PEM_write_bio_X509_CRL(bio, crl))
			return auxL_error(L, auxL_EOPENSSL, "x509.crl:__tostring");
		break;
	case X509_DER:
		if (!i2d_X509_CRL_bio(bio, crl))
			return auxL_error(L, auxL_EOPENSSL, "x509.crl:__tostring");
		break;
	} /* switch() */

	len = BIO_get_mem_data(bio, &data);

	lua_pushlstring(L, data, len);

	return 1;
} /* xx__tostring() */


static int xx__gc(lua_State *L) {
	X509_CRL **ud = luaL_checkudata(L, 1, X509_CRL_CLASS);

	if (*ud) {
		X509_CRL_free(*ud);
		*ud = NULL;
	}

	return 0;
} /* xx__gc() */

static const luaL_Reg xx_methods[] = {
	{ "getVersion",     &xx_getVersion },
	{ "setVersion",     &xx_setVersion },
	{ "getLastUpdate",  &xx_getLastUpdate },
	{ "setLastUpdate",  &xx_setLastUpdate },
	{ "getNextUpdate",  &xx_getNextUpdate },
	{ "setNextUpdate",  &xx_setNextUpdate },
	{ "getIssuer",      &xx_getIssuer },
	{ "setIssuer",      &xx_setIssuer },
	{ "add",            &xx_add },
	{ "addExtension",   &xx_addExtension },
	{ "getExtension",   &xx_getExtension },
	{ "getExtensionCount", &xx_getExtensionCount },
	{ "sign",           &xx_sign },
	{ "text",           &xx_text },
	{ "tostring",       &xx__tostring },
	{ NULL,             NULL },
};

static const luaL_Reg xx_metatable[] = {
	{ "__tostring", &xx__tostring },
	{ "__gc",       &xx__gc },
	{ NULL,         NULL },
};


static const luaL_Reg xx_globals[] = {
	{ "new",       &xx_new },
	{ "interpose", &xx_interpose },
	{ NULL,        NULL },
};

int luaopen_sec_x509_crl(lua_State *L) {
	initall(L);

	luaL_newlib(L, xx_globals);

	return 1;
} /* luaopen_sec_x509_crl() */


/*
 * STACK_OF(X509) - openssl.x509.chain
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void xl_dup(lua_State *L, STACK_OF(X509) *src, _Bool copy) {
	STACK_OF(X509) **dst = prepsimple(L, X509_CHAIN_CLASS);
	X509 *crt;
	int i, n;

	if (copy) {
		if (!(*dst = sk_X509_new_null()))
			goto error;

		n = sk_X509_num(src);

		for (i = 0; i < n; i++) {
			if (!(crt = sk_X509_value(src, i)))
				continue;

			if (!(crt = X509_dup(crt)))
				goto error;

			if (!sk_X509_push(*dst, crt)) {
				X509_free(crt);
				goto error;
			}
		}
	} else {
		if (!(*dst = sk_X509_dup(src)))
			goto error;

		n = sk_X509_num(*dst);

		for (i = 0; i < n; i++) {
			if (!(crt = sk_X509_value(*dst, i)))
				continue;
			CRYPTO_add(&crt->references, 1, CRYPTO_LOCK_X509);
		}
	}

	return;
error:
	auxL_error(L, auxL_EOPENSSL, "sk_X509_dup");
} /* xl_dup() */


static int xl_new(lua_State *L) {
	STACK_OF(X509) **chain = prepsimple(L, X509_CHAIN_CLASS);

	if (!(*chain = sk_X509_new_null()))
		return auxL_error(L, auxL_EOPENSSL, "x509.chain.new");

	return 1;
} /* xl_new() */


static int xl_interpose(lua_State *L) {
	return interpose(L, X509_CHAIN_CLASS);
} /* xl_interpose() */


static int xl_add(lua_State *L) {
	STACK_OF(X509) *chain = checksimple(L, 1, X509_CHAIN_CLASS);
	X509 *crt = checksimple(L, 2, X509_CERT_CLASS);
	X509 *dup;

	if (!(dup = X509_dup(crt)))
		return auxL_error(L, auxL_EOPENSSL, "x509.chain:add");

	if (!sk_X509_push(chain, dup)) {
		X509_free(dup);
		return auxL_error(L, auxL_EOPENSSL, "x509.chain:add");
	}

	lua_pushvalue(L, 1);

	return 1;
} /* xl_add() */


static int xl__next(lua_State *L) {
	STACK_OF(X509) *chain = checksimple(L, lua_upvalueindex(1), X509_CHAIN_CLASS);
	int i = lua_tointeger(L, lua_upvalueindex(2));
	int n = sk_X509_num(chain);

	lua_settop(L, 0);

	while (i < n) {
		X509 *crt, **ret;

		if (!(crt = sk_X509_value(chain, i++)))
			continue;

		lua_pushinteger(L, i);

		ret = prepsimple(L, X509_CERT_CLASS);

		if (!(*ret = X509_dup(crt)))
			return auxL_error(L, auxL_EOPENSSL, "x509.chain:__next");

		break;
	}

	lua_pushinteger(L, i);
	lua_replace(L, lua_upvalueindex(2));

	return lua_gettop(L);
} /* xl__next() */

static int xl__pairs(lua_State *L) {
	lua_settop(L, 1);
	lua_pushinteger(L, 0);
	lua_pushcclosure(L, &xl__next, 2);

	return 1;
} /* xl__pairs() */


static int xl__gc(lua_State *L) {
	STACK_OF(X509) **chain = luaL_checkudata(L, 1, X509_CHAIN_CLASS);

	if (*chain) {
		sk_X509_pop_free(*chain, X509_free);
		*chain = NULL;
	}

	return 0;
} /* xl__gc() */


static const luaL_Reg xl_methods[] = {
	{ "add", &xl_add },
	{ NULL,  NULL },
};

static const luaL_Reg xl_metatable[] = {
	{ "__pairs",  &xl__pairs },
	{ "__ipairs", &xl__pairs },
	{ "__gc",     &xl__gc },
	{ NULL,       NULL },
};

static const luaL_Reg xl_globals[] = {
	{ "new",       &xl_new },
	{ "interpose", &xl_interpose },
	{ NULL,        NULL },
};

int luaopen_sec_x509_chain(lua_State *L) {
	initall(L);

	luaL_newlib(L, xl_globals);

	return 1;
} /* luaopen_sec_x509_chain() */


/*
 * X509_STORE - openssl.x509.store
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static int xs_new(lua_State *L) {
	X509_STORE **ud = prepsimple(L, X509_STORE_CLASS);

	if (!(*ud = X509_STORE_new()))
		return auxL_error(L, auxL_EOPENSSL, "x509.store");

	return 1;
} /* xs_new() */


static int xs_interpose(lua_State *L) {
	return interpose(L, X509_STORE_CLASS);
} /* xs_interpose() */


static int xs_add(lua_State *L) {
	X509_STORE *store = checksimple(L, 1, X509_STORE_CLASS);
	int i, top = lua_gettop(L);

	for (i = 2; i <= top; i++) {
		if (lua_isuserdata(L, i)) {
			X509 *crt = checksimple(L, i, X509_CERT_CLASS);
			X509 *dup;

			if (!(dup = X509_dup(crt)))
				return auxL_error(L, auxL_EOPENSSL, "x509.store:add");

			if (!X509_STORE_add_cert(store, dup)) {
				X509_free(dup);
				return auxL_error(L, auxL_EOPENSSL, "x509.store:add");
			}
		} else {
			const char *path = luaL_checkstring(L, i);
			struct stat st;
			int ok;

			if (0 != stat(path, &st))
				return luaL_error(L, "%s: %s", path, aux_strerror(errno));

			if (S_ISDIR(st.st_mode))
				ok = X509_STORE_load_locations(store, NULL, path);
			else
				ok = X509_STORE_load_locations(store, path, NULL);

			if (!ok)
				return auxL_error(L, auxL_EOPENSSL, "x509.store:add");
		}
	}

	lua_pushvalue(L, 1);

	return 1;
} /* xs_add() */


static int xs_verify(lua_State *L) {
	X509_STORE *store = checksimple(L, 1, X509_STORE_CLASS);
	X509 *crt = checksimple(L, 2, X509_CERT_CLASS);
	STACK_OF(X509) *chain = NULL, **proof;
	X509_STORE_CTX ctx;
	int ok, why;

	/* pre-allocate space for a successful return */
	lua_settop(L, 3);
	proof = prepsimple(L, X509_CHAIN_CLASS);

	if (!lua_isnoneornil(L, 3)) {
		X509 *elm;
		int i, n;

		if (!(chain = sk_X509_dup(checksimple(L, 3, X509_CHAIN_CLASS))))
			return auxL_error(L, auxL_EOPENSSL, "x509.store:verify");

		n = sk_X509_num(chain);

		for (i = 0; i < n; i++) {
			if (!(elm = sk_X509_value(chain, i)))
				continue;
			CRYPTO_add(&elm->references, 1, CRYPTO_LOCK_X509);
		}
	}

	if (!X509_STORE_CTX_init(&ctx, store, crt, chain)) {
		sk_X509_pop_free(chain, X509_free);
		return auxL_error(L, auxL_EOPENSSL, "x509.store:verify");
	}

	ERR_clear_error();

	ok = X509_verify_cert(&ctx);

	switch (ok) {
	case 1: /* verified */
		*proof = X509_STORE_CTX_get1_chain(&ctx);

		X509_STORE_CTX_cleanup(&ctx);

		if (!*proof)
			return auxL_error(L, auxL_EOPENSSL, "x509.store:verify");

		lua_pushboolean(L, 1);
		lua_pushvalue(L, -2);

		return 2;
	case 0: /* not verified */
		why = X509_STORE_CTX_get_error(&ctx);

		X509_STORE_CTX_cleanup(&ctx);

		lua_pushboolean(L, 0);
		lua_pushstring(L, X509_verify_cert_error_string(why));

		return 2;
	default:
		X509_STORE_CTX_cleanup(&ctx);

		return auxL_error(L, auxL_EOPENSSL, "x509.store:verify");
	}
} /* xs_verify() */


static int xs__gc(lua_State *L) {
	X509_STORE **ud = luaL_checkudata(L, 1, X509_STORE_CLASS);

	if (*ud) {
		X509_STORE_free(*ud);
		*ud = NULL;
	}

	return 0;
} /* xs__gc() */


static const luaL_Reg xs_methods[] = {
	{ "add",    &xs_add },
	{ "verify", &xs_verify },
	{ NULL,     NULL },
};

static const luaL_Reg xs_metatable[] = {
	{ "__gc", &xs__gc },
	{ NULL,   NULL },
};

static const luaL_Reg xs_globals[] = {
	{ "new",       &xs_new },
	{ "interpose", &xs_interpose },
	{ NULL,        NULL },
};

int luaopen_sec_x509_store(lua_State *L) {
	initall(L);

	luaL_newlib(L, xs_globals);

	return 1;
} /* luaopen_sec_x509_store() */


/*
 * X509_STORE_CTX - openssl.x509.store.context
 *
 * This object is intended to be a temporary container in OpenSSL, so the
 * memory management is quite clumsy. In particular, it doesn't take
 * ownership of the X509_STORE object, which means the reference must be
 * held externally for the life of the X509_STORE_CTX object.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#if 0
static int stx_new(lua_State *L) {
	X509_STORE_CTX **ud = prepsimple(L, X509_STCTX_CLASS);
	STACK_OF(X509) *chain;

	if (!(*ud = X509_STORE_CTX_new()))
		return auxL_error(L, auxL_EOPENSSL, "x509.store.context");

	return 1;
} /* stx_new() */


static int stx_interpose(lua_State *L) {
	return interpose(L, X509_STCTX_CLASS);
} /* stx_interpose() */


static int stx_add(lua_State *L) {
	X509_STORE_CTX *ctx = checksimple(L, 1, X509_STCTX_CLASS);

	return 0;
} /* stx_add() */


static int stx__gc(lua_State *L) {
	X509_STORE **ud = luaL_checkudata(L, 1, X509_STORE_CLASS);

	if (*ud) {
		X509_STORE_free(*ud);
		*ud = NULL;
	}

	return 0;
} /* stx__gc() */


static const luaL_Reg stx_methods[] = {
	{ "add", &stx_add },
	{ NULL,  NULL },
};

static const luaL_Reg stx_metatable[] = {
	{ "__gc", &stx__gc },
	{ NULL,   NULL },
};

static const luaL_Reg stx_globals[] = {
	{ "new",       &stx_new },
	{ "interpose", &stx_interpose },
	{ NULL,        NULL },
};

int luaopen_sec_x509_store_context(lua_State *L) {
	initall(L);

	luaL_newlib(L, stx_globals);

	return 1;
} /* luaopen_sec_x509_store_context() */
#endif


/*
 * PKCS12 - openssl.pkcs12
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static int p12_new(lua_State *L) {
	char *pass = NULL;
	loadfield(L, 1, "password", LUA_TSTRING, &pass);

	EVP_PKEY *key = loadfield_udata(L, 1, "key", PKEY_CLASS);
	STACK_OF(X509) *certs = loadfield_udata(L, 1, "certs", X509_CHAIN_CLASS);

	PKCS12 **ud = prepsimple(L, PKCS12_CLASS);

	int i;
	int no_kcert = 0;
	X509 *cert = NULL;
	X509 *kcert = NULL;
	STACK_OF(X509) *ca;

	if (!(ca = sk_X509_new_null()))
		goto error;

	for (i = 0; i < sk_X509_num(certs); i++) {
		cert = sk_X509_value(certs, i);
		if (key && X509_check_private_key(cert, key)) {
			if (!(kcert = X509_dup(cert)))
				goto error;
			X509_keyid_set1(kcert, NULL, 0);
			X509_alias_set1(kcert, NULL, 0);
		}
		else sk_X509_push(ca, cert);
	}
	if (key && !kcert) {
		no_kcert = 1;
		goto error;
	}

	if (!(*ud = PKCS12_create(pass, NULL, key, kcert, ca, 0, 0, 0, 0, 0)))
		goto error;

	if (kcert)
		X509_free(kcert);
	sk_X509_free(ca);

	return 1;

error:
	if (kcert)
		X509_free(kcert);
	if (ca)
		sk_X509_free(ca);

	if (no_kcert)
		luaL_argerror(L, 1, lua_pushfstring(L, "certificate matching the key not found"));

	return auxL_error(L, auxL_EOPENSSL, "pkcs12.new");
} /* p12_new() */


static int p12_interpose(lua_State *L) {
	return interpose(L, PKCS12_CLASS);
} /* p12_interpose() */


static int p12__tostring(lua_State *L) {
	PKCS12 *p12 = checksimple(L, 1, PKCS12_CLASS);
	BIO *bio = getbio(L);
	char *data;
	long len;

	if (!i2d_PKCS12_bio(bio, p12))
		return auxL_error(L, auxL_EOPENSSL, "pkcs12:__tostring");

	len = BIO_get_mem_data(bio, &data);

	lua_pushlstring(L, data, len);

	return 1;
} /* p12__tostring() */


static int p12__gc(lua_State *L) {
	PKCS12 **ud = luaL_checkudata(L, 1, PKCS12_CLASS);

	if (*ud) {
		PKCS12_free(*ud);
		*ud = NULL;
	}

	return 0;
} /* p12__gc() */


static const luaL_Reg p12_methods[] = {
	{ "tostring", &p12__tostring },
	{ NULL,         NULL },
};

static const luaL_Reg p12_metatable[] = {
	{ "__tostring", &p12__tostring },
	{ "__gc",       &p12__gc },
	{ NULL,         NULL },
};

static const luaL_Reg p12_globals[] = {
	{ "new",       &p12_new },
	{ "interpose", &p12_interpose },
	{ NULL,        NULL },
};

int luaopen_sec_pkcs12(lua_State *L) {
	initall(L);

	luaL_newlib(L, p12_globals);

	return 1;
} /* luaopen_sec_pkcs12() */


/*
 * SSL_CTX - openssl.ssl.context
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * NOTE: TLS methods and flags were added in tandem. For example, if the
 * macro SSL_OP_NO_TLSv1_1 is defined we know TLSv1_1_server_method is also
 * declared and defined.
 */
static int sx_new(lua_State *L) {
	static const char *const opts[] = {
		[0] = "SSL",
		[1] = "TLS",
		[2] = "SSLv2",
		[3] = "SSLv3",
		[4] = "SSLv23",
		[5] = "TLSv1", [6] = "TLSv1.0",
		[7] = "TLSv1_1", [8] = "TLSv1.1",
		[9] = "TLSv1_2", [10] = "TLSv1.2",
		[11] = "DTLS",
		[12] = "DTLSv1", [13] = "DTLSv1.0",
		[14] = "DTLSv1_2", [15] = "DTLSv1.2",
		NULL
	};
	/* later versions of SSL declare a const qualifier on the return type */
	__typeof__(&TLSv1_client_method) method = &TLSv1_client_method;
	_Bool srv;
	SSL_CTX **ud;
	int options = 0;

	lua_settop(L, 2);
	srv = lua_toboolean(L, 2);

	switch (checkoption(L, 1, "TLS", opts)) {
	case 0: /* SSL */
		method = (srv)? &SSLv23_server_method : &SSLv23_client_method;
		options = SSL_OP_NO_SSLv2;
		break;
	case 1: /* TLS */
		method = (srv)? &SSLv23_server_method : &SSLv23_client_method;
		options = SSL_OP_NO_SSLv2|SSL_OP_NO_SSLv3;
		break;
#ifndef OPENSSL_NO_SSL2
	case 2: /* SSLv2 */
		method = (srv)? &SSLv2_server_method : &SSLv2_client_method;
		break;
#endif
	case 3: /* SSLv3 */
		method = (srv)? &SSLv3_server_method : &SSLv3_client_method;
		break;
	case 4: /* SSLv23 */
		method = (srv)? &SSLv23_server_method : &SSLv23_client_method;
		break;
	case 5: /* TLSv1 */
	case 6: /* TLSv1.0 */
		method = (srv)? &TLSv1_server_method : &TLSv1_client_method;
		break;
#if defined SSL_OP_NO_TLSv1_1
	case 7: /* TLSv1_1 */
	case 8: /* TLSv1.1 */
		method = (srv)? &TLSv1_1_server_method : &TLSv1_1_client_method;
		break;
#endif
#if defined SSL_OP_NO_TLSv1_2
	case 9: /* TLSv1_2 */
	case 10: /* TLSv1.2 */
		method = (srv)? &TLSv1_2_server_method : &TLSv1_2_client_method;
		break;
#endif
#if HAVE_DTLS_CLIENT_METHOD
	case 11: /* DTLS */
		method = (srv)? &DTLS_server_method : &DTLS_client_method;
		break;
#endif
#if HAVE_DTLSV1_CLIENT_METHOD
	case 12: /* DTLSv1 */
	case 13: /* DTLSv1.0 */
		method = (srv)? &DTLSv1_server_method : &DTLSv1_client_method;
		break;
#endif
#if HAVE_DTLSV1_2_CLIENT_METHOD
	case 14: /* DTLSv1_2 */
	case 15: /* DTLSv1.2 */
		method = (srv)? &DTLSv1_server_method : &DTLSv1_client_method;
		break;
#endif
	default:
		return badoption(L, 1, NULL);
	}

	ud = prepsimple(L, SSL_CTX_CLASS);

	if (!(*ud = SSL_CTX_new(method())))
		return auxL_error(L, auxL_EOPENSSL, "ssl.context.new");

	SSL_CTX_set_options(*ud, options);

	return 1;
} /* sx_new() */


static int sx_interpose(lua_State *L) {
	return interpose(L, SSL_CTX_CLASS);
} /* sx_interpose() */


static int sx_setOptions(lua_State *L) {
	SSL_CTX *ctx = checksimple(L, 1, SSL_CTX_CLASS);
	auxL_Integer options = auxL_checkinteger(L, 2);

	auxL_pushinteger(L, SSL_CTX_set_options(ctx, options));

	return 1;
} /* sx_setOptions() */


static int sx_getOptions(lua_State *L) {
	SSL_CTX *ctx = checksimple(L, 1, SSL_CTX_CLASS);

	auxL_pushinteger(L, SSL_CTX_get_options(ctx));

	return 1;
} /* sx_getOptions() */


static int sx_clearOptions(lua_State *L) {
	SSL_CTX *ctx = checksimple(L, 1, SSL_CTX_CLASS);
	auxL_Integer options = auxL_checkinteger(L, 2);

	auxL_pushinteger(L, SSL_CTX_clear_options(ctx, options));

	return 1;
} /* sx_clearOptions() */


static int sx_setStore(lua_State *L) {
	SSL_CTX *ctx = checksimple(L, 1, SSL_CTX_CLASS);
	X509_STORE *store = checksimple(L, 2, X509_STORE_CLASS);

	SSL_CTX_set1_cert_store(ctx, store);

	lua_pushboolean(L, 1);

	return 1;
} /* sx_setStore() */


static int sx_setVerify(lua_State *L) {
	SSL_CTX *ctx = checksimple(L, 1, SSL_CTX_CLASS);
	int mode = luaL_optint(L, 2, -1);
	int depth = luaL_optint(L, 3, -1);

	if (mode != -1)
		SSL_CTX_set_verify(ctx, mode, 0);

	if (depth != -1)
		SSL_CTX_set_verify_depth(ctx, depth);

	lua_pushboolean(L, 1);

	return 1;
} /* sx_setVerify() */


static int sx_getVerify(lua_State *L) {
	SSL_CTX *ctx = checksimple(L, 1, SSL_CTX_CLASS);

	lua_pushinteger(L, SSL_CTX_get_verify_mode(ctx));
	lua_pushinteger(L, SSL_CTX_get_verify_depth(ctx));

	return 2;
} /* sx_getVerify() */


static int sx_setCertificate(lua_State *L) {
	SSL_CTX *ctx = checksimple(L, 1, SSL_CTX_CLASS);
	X509 *crt = X509_dup(checksimple(L, 2, X509_CERT_CLASS));
	int ok;

	ok = SSL_CTX_use_certificate(ctx, crt);
	X509_free(crt);

	if (!ok)
		return auxL_error(L, auxL_EOPENSSL, "ssl.context:setCertificate");

	lua_pushboolean(L, 1);

	return 1;
} /* sx_setCertificate() */


static int sx_setPrivateKey(lua_State *L) {
	SSL_CTX *ctx = checksimple(L, 1, SSL_CTX_CLASS);
	EVP_PKEY *key = checksimple(L, 2, PKEY_CLASS);

	/*
	 * NOTE: No easy way to dup the key, but a shared reference should
	 * be okay as keys are less mutable than certificates.
	 *
	 * FIXME: SSL_CTX_use_PrivateKey will return true even if the
	 * EVP_PKEY object has no private key. Instead, we'll just get a
	 * segfault during the SSL handshake. We need to check that a
	 * private key is actually defined in the object.
	 */
	if (!SSL_CTX_use_PrivateKey(ctx, key))
		return auxL_error(L, auxL_EOPENSSL, "ssl.context:setPrivateKey");

	lua_pushboolean(L, 1);

	return 1;
} /* sx_setPrivateKey() */


static int sx_setCipherList(lua_State *L) {
	SSL_CTX *ctx = checksimple(L, 1, SSL_CTX_CLASS);
	const char *ciphers = luaL_checkstring(L, 2);

	if (!SSL_CTX_set_cipher_list(ctx, ciphers))
		return auxL_error(L, auxL_EOPENSSL, "ssl.context:setCipherList");

	lua_pushboolean(L, 1);

	return 1;
} /* sx_setCipherList() */


static int sx_setEphemeralKey(lua_State *L) {
	SSL_CTX *ctx = checksimple(L, 1, SSL_CTX_CLASS);
	EVP_PKEY *key = checksimple(L, 2, PKEY_CLASS);
	void *tmp;

	/*
	 * NOTE: SSL_CTX_set_tmp duplicates the keys, so we don't need to
	 * worry about lifetimes. EVP_PKEY_get0 doesn't increment the
	 * reference count.
	 */
	switch (EVP_PKEY_base_id(key)) {
	case EVP_PKEY_RSA:
		if (!(tmp = EVP_PKEY_get0(key)))
			return auxL_error(L, auxL_EOPENSSL, "ssl.context:setEphemeralKey");

		if (!SSL_CTX_set_tmp_rsa(ctx, tmp))
			return auxL_error(L, auxL_EOPENSSL, "ssl.context:setEphemeralKey");

		break;
	case EVP_PKEY_DH:
		if (!(tmp = EVP_PKEY_get0(key)))
			return auxL_error(L, auxL_EOPENSSL, "ssl.context:setEphemeralKey");

		if (!SSL_CTX_set_tmp_dh(ctx, tmp))
			return auxL_error(L, auxL_EOPENSSL, "ssl.context:setEphemeralKey");

		break;
	case EVP_PKEY_EC:
		if (!(tmp = EVP_PKEY_get0(key)))
			return auxL_error(L, auxL_EOPENSSL, "ssl.context:setEphemeralKey");

		if (!SSL_CTX_set_tmp_ecdh(ctx, tmp))
			return auxL_error(L, auxL_EOPENSSL, "ssl.context:setEphemeralKey");

		break;
	default:
		return luaL_error(L, "%d: unsupported EVP base type", EVP_PKEY_base_id(key));
	} /* switch() */

	lua_pushboolean(L, 1);

	return 1;
} /* sx_setEphemeralKey() */


#if HAVE_SSL_CTX_SET_ALPN_PROTOS
static int sx_setAlpnProtos(lua_State *L) {
	SSL_CTX *ctx = checksimple(L, 1, SSL_CTX_CLASS);
	luaL_Buffer B;
	size_t len;
	const char *tmp;

	luaL_buffinit(L, &B);
	checkprotos(&B, L, 2);
	luaL_pushresult(&B);
	tmp = lua_tolstring(L, -1, &len);

	/* OpenSSL 1.0.2 doesn't update the error stack on failure. */
	ERR_clear_error();
	if (0 != SSL_CTX_set_alpn_protos(ctx, (const unsigned char*)tmp, len)) {
		if (!ERR_peek_error()) {
			return luaL_error(L, "unable to set ALPN protocols: %s", aux_strerror(ENOMEM));
		} else {
			return auxL_error(L, auxL_EOPENSSL, "ssl.context:setAlpnProtos");
		}
	}

	lua_pushboolean(L, 1);

	return 1;
} /* sx_setAlpnProtos() */
#endif

#if HAVE_SSL_CTX_SET_ALPN_SELECT_CB
static SSL *ssl_push(lua_State *, SSL *);

static int sx_setAlpnSelect_cb(SSL *ssl, const unsigned char **out, unsigned char *outlen, const unsigned char *in, unsigned int inlen, void *_ctx) {
	SSL_CTX *ctx = _ctx;
	lua_State *L = NULL;
	size_t n, protolen, tmpsiz;
	int otop, status;
	const void *proto;
	void *tmpbuf;

	*out = NULL;
	*outlen = 0;

	/* expect at least two values: return buffer and closure */
	if ((n = ex_getdata(&L, EX_SSL_CTX_ALPN_SELECT_CB, ctx)) < 2)
		return SSL_TLSEXT_ERR_ALERT_FATAL;

	otop = lua_gettop(L) - n;

	/* TODO: Install temporary panic handler to catch OOM errors */

	/* pass SSL object as 1st argument */
	ssl_push(L, ssl);
	lua_insert(L, otop + 3);

	/* pass table of protocol names as 2nd argument */
	pushprotos(L, in, inlen);
	lua_insert(L, otop + 4);

	if (LUA_OK != (status = lua_pcall(L, 2 + (n - 2), 1, 0)))
		goto fatal;

	/* did we get a string result? */
	if (!(proto = lua_tolstring(L, -1, &protolen)))
		goto noack;

	/* will it fit in our return buffer? */
	if (!(tmpbuf = lua_touserdata(L, otop + 1)))
		goto fatal;

	tmpsiz = lua_rawlen(L, otop + 1);

	if (protolen > tmpsiz)
		goto fatal;

	memcpy(tmpbuf, proto, protolen);

	/*
	 * NB: Our return buffer is anchored using the luaL_ref API, so even
	 * once we pop the stack it will remain valid.
	 */
	*out = tmpbuf;
	*outlen = protolen;

	lua_settop(L, otop);

	return SSL_TLSEXT_ERR_OK;
fatal:
	lua_settop(L, otop);

	return SSL_TLSEXT_ERR_ALERT_FATAL;
noack:
	lua_settop(L, otop);

	return SSL_TLSEXT_ERR_NOACK;
} /* sx_setAlpnSelect_cb() */

static int sx_setAlpnSelect(lua_State *L) {
	SSL_CTX *ctx = checksimple(L, 1, SSL_CTX_CLASS);
	int error;

	luaL_checktype(L, 2, LUA_TFUNCTION);

	/* allocate space to store the selected protocol in our callback */
	lua_newuserdata(L, UCHAR_MAX);
	lua_insert(L, 2);

	if ((error = ex_setdata(L, EX_SSL_CTX_ALPN_SELECT_CB, ctx, lua_gettop(L) - 1))) {
		if (error > 0) {
			return luaL_error(L, "unable to set ALPN protocol selection callback: %s", aux_strerror(error));
		} else if (error == auxL_EOPENSSL && !ERR_peek_error()) {
			return luaL_error(L, "unable to set ALPN protocol selection callback: Unknown internal error");
		} else {
			return auxL_error(L, error, "ssl.context:setAlpnSelect");
		}
	}

	SSL_CTX_set_alpn_select_cb(ctx, &sx_setAlpnSelect_cb, ctx);

	lua_pushboolean(L, 1);

	return 1;
} /* sx_setAlpnSelect() */
#endif


static int sx__gc(lua_State *L) {
	SSL_CTX **ud = luaL_checkudata(L, 1, SSL_CTX_CLASS);

	if (*ud) {
		SSL_CTX_free(*ud);
		*ud = NULL;
	}

	return 0;
} /* sx__gc() */


static const luaL_Reg sx_methods[] = {
	{ "setOptions",       &sx_setOptions },
	{ "getOptions",       &sx_getOptions },
	{ "clearOptions",     &sx_clearOptions },
	{ "setStore",         &sx_setStore },
	{ "setVerify",        &sx_setVerify },
	{ "getVerify",        &sx_getVerify },
	{ "setCertificate",   &sx_setCertificate },
	{ "setPrivateKey",    &sx_setPrivateKey },
	{ "setCipherList",    &sx_setCipherList },
	{ "setEphemeralKey",  &sx_setEphemeralKey },
#if HAVE_SSL_CTX_SET_ALPN_PROTOS
	{ "setAlpnProtos",    &sx_setAlpnProtos },
#endif
#if HAVE_SSL_CTX_SET_ALPN_SELECT_CB
	{ "setAlpnSelect",    &sx_setAlpnSelect },
#endif
	{ NULL, NULL },
};

static const luaL_Reg sx_metatable[] = {
	{ "__gc", &sx__gc },
	{ NULL,   NULL },
};

static const luaL_Reg sx_globals[] = {
	{ "new",       &sx_new },
	{ "interpose", &sx_interpose },
	{ NULL,        NULL },
};

static const auxL_IntegerReg sx_verify[] = {
	{ "VERIFY_NONE", SSL_VERIFY_NONE },
	{ "VERIFY_PEER", SSL_VERIFY_PEER },
	{ "VERIFY_FAIL_IF_NO_PEER_CERT", SSL_VERIFY_FAIL_IF_NO_PEER_CERT },
	{ "VERIFY_CLIENT_ONCE", SSL_VERIFY_CLIENT_ONCE },
	{ NULL, 0 },
};

static const auxL_IntegerReg sx_option[] = {
	{ "OP_MICROSOFT_SESS_ID_BUG", SSL_OP_MICROSOFT_SESS_ID_BUG },
	{ "OP_NETSCAPE_CHALLENGE_BUG", SSL_OP_NETSCAPE_CHALLENGE_BUG },
	{ "OP_LEGACY_SERVER_CONNECT", SSL_OP_LEGACY_SERVER_CONNECT },
	{ "OP_NETSCAPE_REUSE_CIPHER_CHANGE_BUG", SSL_OP_NETSCAPE_REUSE_CIPHER_CHANGE_BUG },
	{ "OP_SSLREF2_REUSE_CERT_TYPE_BUG", SSL_OP_SSLREF2_REUSE_CERT_TYPE_BUG },
	{ "OP_MICROSOFT_BIG_SSLV3_BUFFER", SSL_OP_MICROSOFT_BIG_SSLV3_BUFFER },
	{ "OP_MSIE_SSLV2_RSA_PADDING", SSL_OP_MSIE_SSLV2_RSA_PADDING },
	{ "OP_SSLEAY_080_CLIENT_DH_BUG", SSL_OP_SSLEAY_080_CLIENT_DH_BUG },
	{ "OP_TLS_D5_BUG", SSL_OP_TLS_D5_BUG },
	{ "OP_TLS_BLOCK_PADDING_BUG", SSL_OP_TLS_BLOCK_PADDING_BUG },
#if defined SSL_OP_NO_TLSv1_1
	{ "OP_NO_TLSv1_1", SSL_OP_NO_TLSv1_1 },
#endif
	{ "OP_DONT_INSERT_EMPTY_FRAGMENTS", SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS },
	{ "OP_ALL", SSL_OP_ALL },
	{ "OP_NO_QUERY_MTU", SSL_OP_NO_QUERY_MTU },
	{ "OP_COOKIE_EXCHANGE", SSL_OP_COOKIE_EXCHANGE },
	{ "OP_NO_TICKET", SSL_OP_NO_TICKET },
	{ "OP_CISCO_ANYCONNECT", SSL_OP_CISCO_ANYCONNECT },
	{ "OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION", SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION },
#if defined SSL_OP_NO_COMPRESSION
	{ "OP_NO_COMPRESSION", SSL_OP_NO_COMPRESSION },
#endif
	{ "OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION", SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION },
	{ "OP_SINGLE_ECDH_USE", SSL_OP_SINGLE_ECDH_USE },
	{ "OP_SINGLE_DH_USE", SSL_OP_SINGLE_DH_USE },
	{ "OP_EPHEMERAL_RSA", SSL_OP_EPHEMERAL_RSA },
	{ "OP_CIPHER_SERVER_PREFERENCE", SSL_OP_CIPHER_SERVER_PREFERENCE },
	{ "OP_TLS_ROLLBACK_BUG", SSL_OP_TLS_ROLLBACK_BUG },
	{ "OP_NO_SSLv2", SSL_OP_NO_SSLv2 },
	{ "OP_NO_SSLv3", SSL_OP_NO_SSLv3 },
	{ "OP_NO_TLSv1", SSL_OP_NO_TLSv1 },
#if defined SSL_OP_NO_TLSv1_2
	{ "OP_NO_TLSv1_2", SSL_OP_NO_TLSv1_2 },
#endif
	{ "OP_PKCS1_CHECK_1", SSL_OP_PKCS1_CHECK_1 },
	{ "OP_PKCS1_CHECK_2", SSL_OP_PKCS1_CHECK_2 },
	{ "OP_NETSCAPE_CA_DN_BUG", SSL_OP_NETSCAPE_CA_DN_BUG },
	{ "OP_NETSCAPE_DEMO_CIPHER_CHANGE_BUG", SSL_OP_NETSCAPE_DEMO_CIPHER_CHANGE_BUG },
#if defined SSL_OP_CRYPTOPRO_TLSEXT_BUG
	{ "OP_CRYPTOPRO_TLSEXT_BUG", SSL_OP_CRYPTOPRO_TLSEXT_BUG },
#endif
	{ NULL, 0 },
};

int luaopen_sec_ssl_context(lua_State *L) {
	initall(L);

	luaL_newlib(L, sx_globals);
	auxL_setintegers(L, sx_verify);
	auxL_setintegers(L, sx_option);

	return 1;
} /* luaopen_sec_ssl_context() */


/*
 * SSL - openssl.ssl
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static SSL *ssl_push(lua_State *L, SSL *ssl) {
	SSL **ud = prepsimple(L, SSL_CLASS);

	CRYPTO_add(&(ssl)->references, 1, CRYPTO_LOCK_SSL);
	*ud = ssl;

	return *ud;
} /* ssl_push() */

static int ssl_new(lua_State *L) {
	lua_pushnil(L);

	return 1;
} /* ssl_new() */


static int ssl_interpose(lua_State *L) {
	return interpose(L, SSL_CLASS);
} /* ssl_interpose() */


static int ssl_setOptions(lua_State *L) {
	SSL *ssl = checksimple(L, 1, SSL_CTX_CLASS);
	auxL_Integer options = auxL_checkinteger(L, 2);

	auxL_pushinteger(L, SSL_set_options(ssl, options));

	return 1;
} /* ssl_setOptions() */


static int ssl_getOptions(lua_State *L) {
	SSL *ssl = checksimple(L, 1, SSL_CTX_CLASS);

	auxL_pushinteger(L, SSL_get_options(ssl));

	return 1;
} /* ssl_getOptions() */


static int ssl_clearOptions(lua_State *L) {
	SSL *ssl = checksimple(L, 1, SSL_CTX_CLASS);
	auxL_Integer options = auxL_checkinteger(L, 2);

	auxL_pushinteger(L, SSL_clear_options(ssl, options));

	return 1;
} /* ssl_clearOptions() */


static int ssl_getPeerCertificate(lua_State *L) {
	SSL *ssl = checksimple(L, 1, SSL_CLASS);
	X509 **x509 = prepsimple(L, X509_CERT_CLASS);

	if (!(*x509 = SSL_get_peer_certificate(ssl)))
		return 0;

	return 1;
} /* ssl_getPeerCertificate() */


static int ssl_getPeerChain(lua_State *L) {
	SSL *ssl = checksimple(L, 1, SSL_CLASS);
	STACK_OF(X509) *chain;

	if (!(chain = SSL_get_peer_cert_chain(ssl)))
		return 0;

	xl_dup(L, chain, 0);

	return 1;
} /* ssl_getPeerChain() */


static int ssl_getCipherInfo(lua_State *L) {
	SSL *ssl = checksimple(L, 1, SSL_CLASS);
	const SSL_CIPHER *cipher;
	char descr[256];

	if (!(cipher = SSL_get_current_cipher(ssl)))
		return 0;

	lua_newtable(L);

	lua_pushstring(L, SSL_CIPHER_get_name(cipher));
	lua_setfield(L, -2, "name");

	lua_pushinteger(L, SSL_CIPHER_get_bits(cipher, 0));
	lua_setfield(L, -2, "bits");

	lua_pushstring(L, SSL_CIPHER_get_version(cipher));
	lua_setfield(L, -2, "version");

	lua_pushstring(L, SSL_CIPHER_description(cipher, descr, sizeof descr));
	lua_setfield(L, -2, "description");

	return 1;
} /* ssl_getCipherInfo() */


static int ssl_getHostName(lua_State *L) {
	SSL *ssl = checksimple(L, 1, SSL_CLASS);
	const char *host;

	if (!(host = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name)))
		return 0;

	lua_pushstring(L, host);

	return 1;
} /* ssl_getHostName() */


static int ssl_setHostName(lua_State *L) {
	SSL *ssl = checksimple(L, 1, SSL_CLASS);
	const char *host = luaL_checkstring(L, 2);

	if (!SSL_set_tlsext_host_name(ssl, host))
		return auxL_error(L, auxL_EOPENSSL, "ssl:setHostName");

	lua_pushboolean(L, 1);

	return 1;
} /* ssl_setHostName() */


static int ssl_getVersion(lua_State *L) {
	SSL *ssl = checksimple(L, 1, SSL_CLASS);
	int format = luaL_checkoption(L, 2, "d", (const char *[]){ "d", ".", "f", NULL });
	int version = SSL_version(ssl);
	int major, minor;

	switch (format) {
	case 1: case 2:
		major = 0xff & ((version >> 8));
		minor = (0xff & version);

		luaL_argcheck(L, minor < 10, 2, "unable to convert SSL version to float because minor version >= 10");
		lua_pushnumber(L, major + ((double)minor / 10));

		break;
	default:
		lua_pushinteger(L, version);

		break;
	}

	return 1;
} /* ssl_getVersion() */


static int ssl_getClientVersion(lua_State *L) {
	SSL *ssl = checksimple(L, 1, SSL_CLASS);
	int format = luaL_checkoption(L, 2, "d", (const char *[]){ "d", ".", "f", NULL });
	int version = ssl->client_version;
	int major, minor;

	switch (format) {
	case 1: case 2:
		major = 0xff & ((version >> 8));
		minor = (0xff & version);

		luaL_argcheck(L, minor < 10, 2, "unable to convert SSL client version to float because minor version >= 10");
		lua_pushnumber(L, major + ((double)minor / 10));

		break;
	default:
		lua_pushinteger(L, version);

		break;
	}

	return 1;
} /* ssl_getClientVersion() */


#if HAVE_SSL_GET0_ALPN_SELECTED
static int ssl_getAlpnSelected(lua_State *L) {
	SSL *ssl = checksimple(L, 1, SSL_CLASS);
	const unsigned char *data;
	unsigned len;
	SSL_get0_alpn_selected(ssl, &data, &len);
	if (0 == len) {
		lua_pushnil(L);
	} else {
		lua_pushlstring(L, (const char *)data, len);
	}
	return 1;
} /* ssl_getAlpnSelected() */
#endif


#if HAVE_SSL_SET_ALPN_PROTOS
static int ssl_setAlpnProtos(lua_State *L) {
	SSL *ssl = checksimple(L, 1, SSL_CLASS);
	luaL_Buffer B;
	size_t len;
	const char *tmp;

	luaL_buffinit(L, &B);
	checkprotos(&B, L, 2);
	luaL_pushresult(&B);
	tmp = lua_tolstring(L, -1, &len);

	/* OpenSSL 1.0.2 doesn't update the error stack on failure. */
	ERR_clear_error();
	if (0 != SSL_set_alpn_protos(ssl, (const unsigned char*)tmp, len)) {
		if (!ERR_peek_error()) {
			return luaL_error(L, "unable to set ALPN protocols: %s", aux_strerror(ENOMEM));
		} else {
			return auxL_error(L, auxL_EOPENSSL, "ssl:setAlpnProtos");
		}
	}

	lua_pushboolean(L, 1);

	return 1;
} /* ssl_setAlpnProtos() */
#endif


static int ssl__gc(lua_State *L) {
	SSL **ud = luaL_checkudata(L, 1, SSL_CLASS);

	if (*ud) {
		SSL_free(*ud);
		*ud = NULL;
	}

	return 0;
} /* ssl__gc() */


static const luaL_Reg ssl_methods[] = {
	{ "setOptions",       &ssl_setOptions },
	{ "getOptions",       &ssl_getOptions },
	{ "clearOptions",     &ssl_clearOptions },
	{ "getPeerCertificate", &ssl_getPeerCertificate },
	{ "getPeerChain",     &ssl_getPeerChain },
	{ "getCipherInfo",    &ssl_getCipherInfo },
	{ "getHostName",      &ssl_getHostName },
	{ "setHostName",      &ssl_setHostName },
	{ "getVersion",       &ssl_getVersion },
	{ "getClientVersion", &ssl_getClientVersion },
#if HAVE_SSL_GET0_ALPN_SELECTED
	{ "getAlpnSelected",  &ssl_getAlpnSelected },
#endif
#if HAVE_SSL_SET_ALPN_PROTOS
	{ "setAlpnProtos",    &ssl_setAlpnProtos },
#endif
	{ NULL,            NULL },
};

static const luaL_Reg ssl_metatable[] = {
	{ "__gc", &ssl__gc },
	{ NULL,   NULL },
};

static const luaL_Reg ssl_globals[] = {
	{ "new",       &ssl_new },
	{ "interpose", &ssl_interpose },
	{ NULL,        NULL },
};

static const auxL_IntegerReg ssl_version[] = {
	{ "SSL2_VERSION", SSL2_VERSION },
	{ "SSL3_VERSION", SSL3_VERSION },
	{ "TLS1_VERSION", TLS1_VERSION },
#if defined TLS1_1_VERSION
	{ "TLS1_1_VERSION", TLS1_1_VERSION },
#endif
#if defined TLS1_2_VERSION
	{ "TLS1_2_VERSION", TLS1_2_VERSION },
#endif
	{ NULL, 0 },
};


int luaopen_sec_ssl(lua_State *L) {
	initall(L);

	luaL_newlib(L, ssl_globals);
	auxL_setintegers(L, ssl_version);
	auxL_setintegers(L, sx_verify);
	auxL_setintegers(L, sx_option);

	return 1;
} /* luaopen_sec_ssl() */


/*
 * Digest - openssl.digest
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static const EVP_MD *md_optdigest(lua_State *L, int index) {
	const char *name = luaL_optstring(L, index, "sha1");
	const EVP_MD *type;

	if (!(type = EVP_get_digestbyname(name)))
		luaL_argerror(L, index, lua_pushfstring(L, "%s: invalid digest type", name));

	return type;
} /* md_optdigest() */


static int md_new(lua_State *L) {
	const EVP_MD *type = md_optdigest(L, 1);
	EVP_MD_CTX *ctx;

	ctx = prepudata(L, sizeof *ctx, DIGEST_CLASS, NULL);

	EVP_MD_CTX_init(ctx);

	if (!EVP_DigestInit_ex(ctx, type, NULL))
		return auxL_error(L, auxL_EOPENSSL, "digest.new");

	return 1;
} /* md_new() */


static int md_interpose(lua_State *L) {
	return interpose(L, DIGEST_CLASS);
} /* md_interpose() */


static void md_update_(lua_State *L, EVP_MD_CTX *ctx, int from, int to) {
	int i;

	for (i = from; i <= to; i++) {
		const void *p;
		size_t n;

		p = luaL_checklstring(L, i, &n);

		if (!EVP_DigestUpdate(ctx, p, n))
			auxL_error(L, auxL_EOPENSSL, "digest:update");
	}
} /* md_update_() */


static int md_update(lua_State *L) {
	EVP_MD_CTX *ctx = luaL_checkudata(L, 1, DIGEST_CLASS);

	md_update_(L, ctx, 2, lua_gettop(L));

	lua_pushvalue(L, 1);

	return 1;
} /* md_update() */


static int md_final(lua_State *L) {
	EVP_MD_CTX *ctx = luaL_checkudata(L, 1, DIGEST_CLASS);
	unsigned char md[EVP_MAX_MD_SIZE];
	unsigned len;

	md_update_(L, ctx, 2, lua_gettop(L));

	if (!EVP_DigestFinal_ex(ctx, md, &len))
		return auxL_error(L, auxL_EOPENSSL, "digest:final");

	lua_pushlstring(L, (char *)md, len);

	return 1;
} /* md_final() */


static int md__gc(lua_State *L) {
	EVP_MD_CTX *ctx = luaL_checkudata(L, 1, DIGEST_CLASS);

	EVP_MD_CTX_cleanup(ctx);

	return 0;
} /* md__gc() */


static const luaL_Reg md_methods[] = {
	{ "update", &md_update },
	{ "final",  &md_final },
	{ NULL,     NULL },
};

static const luaL_Reg md_metatable[] = {
	{ "__gc", &md__gc },
	{ NULL,   NULL },
};

static const luaL_Reg md_globals[] = {
	{ "new",       &md_new },
	{ "interpose", &md_interpose },
	{ NULL,        NULL },
};

int luaopen_sec_digest(lua_State *L) {
	initall(L);

	luaL_newlib(L, md_globals);

	return 1;
} /* luaopen_sec_digest() */


/*
 * HMAC - openssl.hmac
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static int hmac_new(lua_State *L) {
	const void *key;
	size_t len;
	const EVP_MD *type;
	HMAC_CTX *ctx;

	key = luaL_checklstring(L, 1, &len);
	type = md_optdigest(L, 2);

	ctx = prepudata(L, sizeof *ctx, HMAC_CLASS, NULL);

	HMAC_Init_ex(ctx, key, len, type, NULL);

	return 1;
} /* hmac_new() */


static int hmac_interpose(lua_State *L) {
	return interpose(L, HMAC_CLASS);
} /* hmac_interpose() */


static void hmac_update_(lua_State *L, HMAC_CTX *ctx, int from, int to) {
	int i;

	for (i = from; i <= to; i++) {
		const void *p;
		size_t n;

		p = luaL_checklstring(L, i, &n);

		HMAC_Update(ctx, p, n);
	}
} /* hmac_update_() */


static int hmac_update(lua_State *L) {
	HMAC_CTX *ctx = luaL_checkudata(L, 1, HMAC_CLASS);

	hmac_update_(L, ctx, 2, lua_gettop(L));

	lua_pushvalue(L, 1);

	return 1;
} /* hmac_update() */


static int hmac_final(lua_State *L) {
	HMAC_CTX *ctx = luaL_checkudata(L, 1, HMAC_CLASS);
	unsigned char hmac[EVP_MAX_MD_SIZE];
	unsigned len;

	hmac_update_(L, ctx, 2, lua_gettop(L));

	HMAC_Final(ctx, hmac, &len);

	lua_pushlstring(L, (char *)hmac, len);

	return 1;
} /* hmac_final() */


static int hmac__gc(lua_State *L) {
	HMAC_CTX *ctx = luaL_checkudata(L, 1, HMAC_CLASS);

	HMAC_CTX_cleanup(ctx);

	return 0;
} /* hmac__gc() */


static const luaL_Reg hmac_methods[] = {
	{ "update", &hmac_update },
	{ "final",  &hmac_final },
	{ NULL,     NULL },
};

static const luaL_Reg hmac_metatable[] = {
	{ "__gc", &hmac__gc },
	{ NULL,   NULL },
};

static const luaL_Reg hmac_globals[] = {
	{ "new",       &hmac_new },
	{ "interpose", &hmac_interpose },
	{ NULL,        NULL },
};

int luaopen_sec_hmac(lua_State *L) {
	initall(L);

	luaL_newlib(L, hmac_globals);

	return 1;
} /* luaopen_sec_hmac() */


/*
 * Cipher - openssl.cipher
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static const EVP_CIPHER *cipher_checktype(lua_State *L, int index) {
	const char *name = luaL_checkstring(L, index);
	const EVP_CIPHER *type;

	if (!(type = EVP_get_cipherbyname(name)))
		luaL_argerror(L, index, lua_pushfstring(L, "%s: invalid cipher type", name));

	return type;
} /* cipher_checktype() */


static int cipher_new(lua_State *L) {
	const EVP_CIPHER *type;
	EVP_CIPHER_CTX *ctx;

	type = cipher_checktype(L, 1);

	ctx = prepudata(L, sizeof *ctx, CIPHER_CLASS, NULL);
	EVP_CIPHER_CTX_init(ctx);

	if (!EVP_CipherInit_ex(ctx, type, NULL, NULL, NULL, -1))
		return auxL_error(L, auxL_EOPENSSL, "cipher.new");

	return 1;
} /* cipher_new() */


static int cipher_interpose(lua_State *L) {
	return interpose(L, CIPHER_CLASS);
} /* cipher_interpose() */


static int cipher_init(lua_State *L, _Bool encrypt) {
	EVP_CIPHER_CTX *ctx = luaL_checkudata(L, 1, CIPHER_CLASS);
	const void *key, *iv;
	size_t n, m;

	key = luaL_checklstring(L, 2, &n);
	m = (size_t)EVP_CIPHER_CTX_key_length(ctx);
	luaL_argcheck(L, n == m, 2, lua_pushfstring(L, "%d: invalid key length (should be %d)", (int)n, (int)m));

	iv = luaL_optlstring(L, 3, NULL, &n);
	m = (size_t)EVP_CIPHER_CTX_iv_length(ctx);
	luaL_argcheck(L, n == m, 3, lua_pushfstring(L, "%d: invalid IV length (should be %d)", (int)n, (int)m));

	if (!EVP_CipherInit_ex(ctx, NULL, NULL, key, iv, encrypt))
		goto sslerr;

	if (!lua_isnoneornil(L, 4)) {
		luaL_checktype(L, 4, LUA_TBOOLEAN);

		if (!EVP_CIPHER_CTX_set_padding(ctx, lua_toboolean(L, 4)))
			goto sslerr;
	}

	lua_settop(L, 1);

	return 1;
sslerr:
	return auxL_error(L, auxL_EOPENSSL, (encrypt)? "cipher:encrypt" : "cipher:decrypt");
} /* cipher_init() */


static int cipher_encrypt(lua_State *L) {
	return cipher_init(L, 1);
} /* cipher_encrypt() */


static int cipher_decrypt(lua_State *L) {
	return cipher_init(L, 0);
} /* cipher_decrypt() */


static _Bool cipher_update_(lua_State *L, EVP_CIPHER_CTX *ctx, luaL_Buffer *B, int from, int to) {
	const unsigned char *p, *pe;
	size_t block, step, n;
	int i;

	block = EVP_CIPHER_CTX_block_size(ctx);

	if (LUAL_BUFFERSIZE < block * 2)
		luaL_error(L, "cipher:update: LUAL_BUFFERSIZE(%d) < 2 * EVP_CIPHER_CTX_block_size(%d)", (int)LUAL_BUFFERSIZE, (int)block);

	step = LUAL_BUFFERSIZE - block;

	for (i = from; i <= to; i++) {
		p = (const unsigned char *)luaL_checklstring(L, i, &n);
		pe = p + n;

		while (p < pe) {
			int in = (int)MIN((size_t)(pe - p), step), out;

			if (!EVP_CipherUpdate(ctx, (void *)luaL_prepbuffer(B), &out, p, in))
				return 0;

			p += in;
			luaL_addsize(B, out);
		}
	}

	return 1;
} /* cipher_update_() */


static int cipher_update(lua_State *L) {
	EVP_CIPHER_CTX *ctx = luaL_checkudata(L, 1, CIPHER_CLASS);
	luaL_Buffer B;

	luaL_buffinit(L, &B);

	if (!cipher_update_(L, ctx, &B, 2, lua_gettop(L)))
		goto sslerr;

	luaL_pushresult(&B);

	return 1;
sslerr:
	lua_pushnil(L);
	auxL_pusherror(L, auxL_EOPENSSL, NULL);

	return 2;
} /* cipher_update() */


static int cipher_final(lua_State *L) {
	EVP_CIPHER_CTX *ctx = luaL_checkudata(L, 1, CIPHER_CLASS);
	luaL_Buffer B;
	size_t block;
	int out;

	luaL_buffinit(L, &B);

	if (!cipher_update_(L, ctx, &B, 2, lua_gettop(L)))
		goto sslerr;

	block = EVP_CIPHER_CTX_block_size(ctx);

	if (LUAL_BUFFERSIZE < block)
		return luaL_error(L, "cipher:update: LUAL_BUFFERSIZE(%d) < EVP_CIPHER_CTX_block_size(%d)", (int)LUAL_BUFFERSIZE, (int)block);

	if (!EVP_CipherFinal(ctx, (void *)luaL_prepbuffer(&B), &out))
		goto sslerr;

	luaL_addsize(&B, out);
	luaL_pushresult(&B);

	return 1;
sslerr:
	lua_pushnil(L);
	auxL_pusherror(L, auxL_EOPENSSL, NULL);

	return 2;
} /* cipher_final() */


static int cipher__gc(lua_State *L) {
	EVP_CIPHER_CTX *ctx = luaL_checkudata(L, 1, CIPHER_CLASS);

	EVP_CIPHER_CTX_cleanup(ctx);

	return 0;
} /* cipher__gc() */


static const luaL_Reg cipher_methods[] = {
	{ "encrypt", &cipher_encrypt },
	{ "decrypt", &cipher_decrypt },
	{ "update",  &cipher_update },
	{ "final",   &cipher_final },
	{ NULL,      NULL },
};

static const luaL_Reg cipher_metatable[] = {
	{ "__gc", &cipher__gc },
	{ NULL,   NULL },
};

static const luaL_Reg cipher_globals[] = {
	{ "new",       &cipher_new },
	{ "interpose", &cipher_interpose },
	{ NULL,        NULL },
};

int luaopen_sec_cipher(lua_State *L) {
	initall(L);

	luaL_newlib(L, cipher_globals);

	return 1;
} /* luaopen_sec_cipher() */


/*
 * Rand - openssl.rand
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

struct randL_state {
	pid_t pid;
}; /* struct randL_state */

static struct randL_state *randL_getstate(lua_State *L) {
	return lua_touserdata(L, lua_upvalueindex(1));
} /* randL_getstate() */

#ifndef HAVE_SYS_SYSCTL_H
//#define HAVE_SYS_SYSCTL_H (!defined __sun && !defined _AIX)
#endif

#if HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h> /* CTL_KERN KERN_RANDOM RANDOM_UUID KERN_URND KERN_ARND sysctl(2) */
#endif

#ifndef HAVE_RANDOM_UUID
//#define HAVE_RANDOM_UUID (defined __linux) /* RANDOM_UUID is an enum, not macro */
#endif

#ifndef HAVE_KERN_URND
//#define HAVE_KERN_URND (defined KERN_URND)
#endif

#ifndef HAVE_KERN_ARND
//#define HAVE_KERN_ARND (defined KERN_ARND)
#endif

static int randL_stir(struct randL_state *st, unsigned rqstd) {
	unsigned count = 0;
	int error;
	unsigned char data[256];
#if HAVE_RANDOM_UUID || HAVE_KERN_URND || HAVE_KERN_ARND
#if HAVE_RANDOM_UUID
	int mib[] = { CTL_KERN, KERN_RANDOM, RANDOM_UUID };
#elif HAVE_KERN_URND
	int mib[] = { CTL_KERN, KERN_URND };
#else
	int mib[] = { CTL_KERN, KERN_ARND };
#endif

	while (count < rqstd) {
		size_t n = MIN(rqstd - count, sizeof data);

		if (0 != sysctl(mib, countof(mib), data, &n, (void *)0, 0))
			break;

		RAND_add(data, n, n);

		count += n;
	}
#endif

	if (count < rqstd) {
#if defined O_CLOEXEC && (!defined _AIX /* O_CLOEXEC overflows int */)
		int fd = open("/dev/urandom", O_RDONLY|O_CLOEXEC);
#else
		int fd = open("/dev/urandom", O_RDONLY);
#endif

		if (fd == -1)
			goto syserr;

		while (count < rqstd) {
			ssize_t n = read(fd, data, MIN(rqstd - count, sizeof data));

			switch (n) {
			case 0:
				errno = EIO;

				/* FALL THROUGH */
			case -1:
				if (errno == EINTR)
					continue;

				error = errno;

				close(fd);

				goto error;
			default:
				RAND_add(data, n, n);

				count += n;
			}
		}

		close(fd);
	}

	st->pid = getpid();

	return 0;
syserr:
	error = errno;
error:;
	struct {
		struct timeval tv;
		pid_t pid;
		struct rusage ru;
		struct utsname un;
		uintptr_t aslr;
#if defined __APPLE__
		uint64_t mt;
#elif defined __sun
		struct timespec mt;
#endif
	} junk;

	gettimeofday(&junk.tv, NULL);
	junk.pid = getpid();
	getrusage(RUSAGE_SELF, &junk.ru);
	uname(&junk.un);
	junk.aslr = (uintptr_t)&strcpy ^ (uintptr_t)&randL_stir;
#if defined __APPLE__
	junk.mt = mach_absolute_time();
#elif defined __sun
	/*
	 * NOTE: Linux requires -lrt for clock_gettime, and in any event
	 * already has RANDOM_UUID. The BSDs have KERN_URND and KERN_ARND.
	 * Just do this for Solaris to keep things simple. We've already
	 * crossed the line of what can be reasonably accomplished on
	 * unreasonable platforms.
	 */
	clock_gettime(CLOCK_MONOTONIC, &junk.mt);
#endif

	RAND_add(&junk, sizeof junk, 0.1);

	st->pid = getpid();

	return error;
} /* randL_stir() */


static void randL_checkpid(struct randL_state *st) {
	if (st->pid != getpid())
		(void)randL_stir(st, 16);
} /* randL_checkpid() */


static int rand_stir(lua_State *L) {
	int error = randL_stir(randL_getstate(L), auxL_optunsigned(L, 1, 16, 0, UINT_MAX));

	if (error) {
		lua_pushboolean(L, 0);
		lua_pushstring(L, aux_strerror(error));
		lua_pushinteger(L, error);

		return 3;
	} else {
		lua_pushboolean(L, 1);

		return 1;
	}
} /* rand_stir() */


static int rand_add(lua_State *L) {
	const void *buf;
	size_t len;
	lua_Number entropy;

	buf = luaL_checklstring(L, 1, &len);
	entropy = luaL_optnumber(L, 2, len);

	RAND_add(buf, len, entropy);

	lua_pushboolean(L, 1);

	return 1;
} /* rand_add() */


static int rand_bytes(lua_State *L) {
	int size = luaL_checkint(L, 1);
	luaL_Buffer B;
	int count = 0, n;

	randL_checkpid(randL_getstate(L));

	luaL_buffinit(L, &B);

	while (count < size) {
		n = MIN((size - count), LUAL_BUFFERSIZE);

		if (!RAND_bytes((void *)luaL_prepbuffer(&B), n))
			return auxL_error(L, auxL_EOPENSSL, "rand.bytes");

		luaL_addsize(&B, n);
		count += n;
	}

	luaL_pushresult(&B);

	return 1;
} /* rand_bytes() */


static int rand_ready(lua_State *L) {
	lua_pushboolean(L, RAND_status() == 1);

	return 1;
} /* rand_ready() */


static unsigned long long rand_llu(lua_State *L) {
	unsigned long long llu;

	if (!RAND_bytes((void *)&llu, sizeof llu))
		auxL_error(L, auxL_EOPENSSL, "rand.uniform");

	return llu;
} /* rand_llu() */

/*
 * The following algorithm for rand_uniform() is taken from OpenBSD's
 * arc4random_uniform, written by Otto Moerbeek, with subsequent
 * simplification by Jorden Verwer. Otto's source code comment reads
 * 
 *   Uniformity is achieved by generating new random numbers until the one
 *   returned is outside the range [0, 2**32 % upper_bound). This guarantees
 *   the selected random number will be inside [2**32 % upper_bound, 2**32)
 *   which maps back to [0, upper_bound) after reduction modulo upper_bound.
 *
 * --
 * A more bit-efficient approach by the eminent statistician Herman Rubin
 * can be found in this sci.crypt Usenet post.
 *
 *   From: hrubin@odds.stat.purdue.edu (Herman Rubin)
 *   Newsgroups: sci.crypt
 *   Subject: Re: Generating a random number between 0 and N-1
 *   Date: 14 Nov 2002 11:20:37 -0500
 *   Organization: Purdue University Statistics Department
 *   Lines: 40
 *   Message-ID: <ar0igl$1ak2@odds.stat.purdue.edu>
 *   References: <yO%y9.19646$RO1.373975@weber.videotron.net> <3DCD8D75.40408@nospam.com>
 *   NNTP-Posting-Host: odds.stat.purdue.edu
 *   X-Trace: mozo.cc.purdue.edu 1037290837 9316 128.210.141.13 (14 Nov 2002 16:20:37 GMT)
 *   X-Complaints-To: ne...@news.purdue.edu
 *   NNTP-Posting-Date: Thu, 14 Nov 2002 16:20:37 +0000 (UTC)
 *   Xref: archiver1.google.com sci.crypt:78935
 *   
 *   In article <3DCD8D7...@nospam.com>,
 *   Michael Amling  <nos...@nospam.com> wrote:
 *   >Carlos Moreno wrote:
 *   
 *   I have already posted on this, but a repeat might be
 *   in order.
 *   
 *   If one can trust random bits, the most bitwise efficient
 *   manner to get a single random integer between 0 and N-1
 *   can be obtained as follows; the code can be made more
 *   computationally efficient.  I believe it is easier to
 *   understand with gotos.  I am assuming N>1.
 *   
 *   	i = 0;	j = 1;
 *   
 *   loop:	j=2*j; i=2*i+RANBIT;
 *   	if (j < N) goto loop;
 *   	if (i >= N) {
 *   		i = i - N;
 *   		j = j - N;
 *   		goto loop:}
 *   	else return (i);
 *   
 *   The algorithm works because at each stage i is uniform
 *   between 0 and j-1.
 *   
 *   Another possibility is to generate k bits, where 2^k >= N.
 *   If 2^k = c*N + remainder, generate the appropriate value
 *   if a k-bit random number is less than c*N.
 *   
 *   For N = 17 (numbers just larger than powers of 2 are "bad"),
 *   the amount of information is about 4.09 bits, the best
 *   algorithm to generate one random number takes about 5.765
 *   bits, taking k = 5 uses 9.412 bits, taking k = 6 or 7 uses
 *   7.529 bits.  These are averages, but the tails are not bad.
 *
 * (https://groups.google.com/forum/message/raw?msg=sci.crypt/DMslf6tSrD8/rv9rk6oP3r4J)
 */
static int rand_uniform(lua_State *L) {
	unsigned long long r;

	randL_checkpid(randL_getstate(L));

	if (lua_isnoneornil(L, 1)) {
		r = rand_llu(L);
	} else {
		unsigned long long N, m;

		if (sizeof (lua_Unsigned) >= sizeof r) {
			N = luaL_checkunsigned(L, 1);
		} else {
			N = luaL_checknumber(L, 1);
		}

		luaL_argcheck(L, N > 1, 1, lua_pushfstring(L, "[0, %d): interval is empty", (int)N));

		m = -N % N;

		do {
			r = rand_llu(L);
		} while (r < m);

		r = r % N;
	}

	if (sizeof (lua_Unsigned) >= sizeof r) {
		lua_pushunsigned(L, r);
	} else {
		lua_pushnumber(L, r);
	}

	return 1;
} /* rand_uniform() */


static const luaL_Reg rand_globals[] = {
	{ "stir",    &rand_stir },
	{ "add",     &rand_add },
	{ "bytes",   &rand_bytes },
	{ "ready",   &rand_ready },
	{ "uniform", &rand_uniform },
	{ NULL,      NULL },
};

int luaopen_sec_rand(lua_State *L) {
	struct randL_state *st;

	initall(L);

	luaL_newlibtable(L, rand_globals);
	st = lua_newuserdata(L, sizeof *st);
	memset(st, 0, sizeof *st);
	luaL_setfuncs(L, rand_globals, 1);

	return 1;
} /* luaopen_sec_rand() */


/*
 * DES - openssl.des
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static int de5_string_to_key(lua_State *L) {
	DES_cblock key;

	DES_string_to_key(luaL_checkstring(L, 1), &key);
	lua_pushlstring(L, (char *)key, sizeof key);

	return 1;
} /* de5_string_to_key() */

static int de5_set_odd_parity(lua_State *L) {
	const char *src;
	size_t len;
	DES_cblock key;

	src = luaL_checklstring(L, 1, &len);
	memset(&key, 0, sizeof key);
	memcpy(&key, src, MIN(len, sizeof key));

	DES_set_odd_parity(&key);
	lua_pushlstring(L, (char *)key, sizeof key);

	return 1;
} /* de5_set_odd_parity() */

static const luaL_Reg des_globals[] = {
	{ "string_to_key",  &de5_string_to_key },
	{ "set_odd_parity", &de5_set_odd_parity },
	{ NULL,            NULL },
};

int luaopen_sec_des(lua_State *L) {
	initall(L);

	luaL_newlib(L, des_globals);

	return 1;
} /* luaopen_sec_des() */


/*
 * Multithread Reentrancy Protection
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static struct {
	pthread_mutex_t *lock;
	int nlock;
} mt_state;

static void mt_lock(int mode, int type, const char *file NOTUSED, int line NOTUSED) {
	if (mode & CRYPTO_LOCK)
		pthread_mutex_lock(&mt_state.lock[type]);
	else
		pthread_mutex_unlock(&mt_state.lock[type]);
} /* mt_lock() */

/*
 * Sources include Google and especially the Wine Project. See get_unix_tid
 * at http://source.winehq.org/git/wine.git/?a=blob;f=dlls/ntdll/server.c.
 */
#if __FreeBSD__
#include <sys/thr.h> /* thr_self(2) */
#elif __NetBSD__
#include <lwp.h> /* _lwp_self(2) */
#endif

static unsigned long mt_gettid(void) {
#if __APPLE__
	return pthread_mach_thread_np(pthread_self());
#elif __DragonFly__
	return lwp_gettid();
#elif  __FreeBSD__
	long id;

	thr_self(&id);

	return id;
#elif __NetBSD__
	return _lwp_self();
#else
	/*
	 * pthread_t is an integer on Solaris and Linux, an unsigned integer
	 * on AIX, and a unique pointer on OpenBSD.
	 */
	return (unsigned long)pthread_self();
#endif
} /* mt_gettid() */

static int mt_init(void) {
	static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	static int done, bound;
	int error = 0;

	if ((error = pthread_mutex_lock(&mutex)))
		return error;

	if (done)
		goto epilog;

	if (!CRYPTO_get_locking_callback()) {
		if (!mt_state.lock) {
			int i;

			mt_state.nlock = CRYPTO_num_locks();
		
			if (!(mt_state.lock = malloc(mt_state.nlock * sizeof *mt_state.lock))) {
				error = errno;
				goto epilog;
			}

			for (i = 0; i < mt_state.nlock; i++) {
				if ((error = pthread_mutex_init(&mt_state.lock[i], NULL))) {
					while (i > 0) {
						pthread_mutex_destroy(&mt_state.lock[--i]);
					}

					free(mt_state.lock);
					mt_state.lock = NULL;

					goto epilog;
				}
			}
		}

		CRYPTO_set_locking_callback(&mt_lock);
		bound = 1;
	}

	if (!CRYPTO_get_id_callback()) {
		CRYPTO_set_id_callback(&mt_gettid);
		bound = 1;
	}

	if (bound && (error = dl_anchor()))
		goto epilog;

	done = 1;
epilog:
	pthread_mutex_unlock(&mutex);

	return error;
} /* mt_init() */


static void initall(lua_State *L) {
	static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	static int initssl;
	int error;

	if ((error = mt_init()))
		auxL_error(L, error, "openssl.init");

	pthread_mutex_lock(&mutex);

	if (!initssl) {
		initssl = 1;

		SSL_load_error_strings();
		SSL_library_init();
		OpenSSL_add_all_algorithms();

		/*
		 * TODO: Figure out a way to detect whether OpenSSL has
		 * already been configured.
		 */
		OPENSSL_config(NULL);
	}

	pthread_mutex_unlock(&mutex);

	if ((error = compat_init()))
		auxL_error(L, error, "openssl.init");

	if ((error = ex_init()))
		auxL_error(L, error, "openssl.init");

	ex_newstate(L);

	addclass(L, BIGNUM_CLASS, bn_methods, bn_metatable);
	addclass(L, PKEY_CLASS, pk_methods, pk_metatable);
	addclass(L, X509_NAME_CLASS, xn_methods, xn_metatable);
	addclass(L, X509_GENS_CLASS, gn_methods, gn_metatable);
	addclass(L, X509_EXT_CLASS, xe_methods, xe_metatable);
	addclass(L, X509_CERT_CLASS, xc_methods, xc_metatable);
	addclass(L, X509_CSR_CLASS, xr_methods, xr_metatable);
	addclass(L, X509_CRL_CLASS, xx_methods, xx_metatable);
	addclass(L, X509_CHAIN_CLASS, xl_methods, xl_metatable);
	addclass(L, X509_STORE_CLASS, xs_methods, xs_metatable);
	addclass(L, PKCS12_CLASS, p12_methods, p12_metatable);
	addclass(L, SSL_CTX_CLASS, sx_methods, sx_metatable);
	addclass(L, SSL_CLASS, ssl_methods, ssl_metatable);
	addclass(L, DIGEST_CLASS, md_methods, md_metatable);
	addclass(L, HMAC_CLASS, hmac_methods, hmac_metatable);
	addclass(L, CIPHER_CLASS, cipher_methods, cipher_metatable);
} /* initall() */


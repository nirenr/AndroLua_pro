/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2018 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lluv library.
******************************************************************************/

#include "lluv_error.h"
#include <assert.h>

#ifdef _MSC_VER
#  define str_n_len strnlen
#else
#  include <memory.h>

  static size_t str_n_len(const char *start, size_t maxlen){
    const char *end = (const char *)memchr(start, '\0', maxlen);
    return (end) ? (size_t)(end - start) : maxlen;
  }
#endif

#define LLUV_ERROR_NAME LLUV_PREFIX" Error"
static const char *LLUV_ERROR = LLUV_ERROR_NAME;
static const char *LLUV_ERR_UV_NAME = "LIBUV";
static const char *LLUV_ERR_LIB_NAME = "LLUV";

//{ Error object

LLUV_INTERNAL int lluv_error_create(lua_State *L, int error_category, uv_errno_t error_no, const char *ext){
  static size_t max_ext_len = 4096;
  lluv_error_t *err;
  size_t len;

  if(ext)len = str_n_len(ext, max_ext_len);else len = 0;

  if(0 == len){
    err = lutil_newudatap(L, lluv_error_t, LLUV_ERROR);
  }
  else{
    err = (lluv_error_t*)lutil_newudatap_impl(L, sizeof(lluv_error_t) + len, LLUV_ERROR);
#ifdef _MSC_VER
    strncpy_s(&err->ext[0], len + 1, ext, len);
#else
    strncpy(&err->ext[0], ext, len);
#endif
  }

  err->ext[len] = '\0';
  err->cat      = error_category;
  err->no       = error_no;

  return 1;
}

static lluv_error_t *lluv_check_error(lua_State *L, int i){
  lluv_error_t *err = (lluv_error_t *)lutil_checkudatap (L, i, LLUV_ERROR);
  luaL_argcheck (L, err != NULL, 1, LLUV_ERROR_NAME" expected");
  return err;
}

static int lluv_err_category(lua_State *L){
  lluv_error_t *err = lluv_check_error(L,1);

  if(err->cat == LLUV_ERR_UV) lua_pushstring(L, LLUV_ERR_UV_NAME);
  else if(err->cat == LLUV_ERR_LIB) lua_pushstring(L, LLUV_ERR_LIB);
  else lua_pushinteger(L, err->cat);

  return 1;
}

static int lluv_err_no(lua_State *L){
  lluv_error_t *err = lluv_check_error(L,1);
  lua_pushinteger(L, err->no);
  return 1;
}

static int lluv_err_msg(lua_State *L){
  lluv_error_t *err = lluv_check_error(L,1);
  lua_pushstring(L, uv_strerror(err->no));
  return 1;
}

static int lluv_err_name(lua_State *L){
  lluv_error_t *err = lluv_check_error(L,1);
  lua_pushstring(L, uv_err_name(err->no));
  return 1;
}

static int lluv_err_ext(lua_State *L){
  lluv_error_t *err = lluv_check_error(L,1);
  lua_pushstring(L, err->ext);
  return 1;
}

static int lluv_err_tostring(lua_State *L){
  lluv_error_t *err = lluv_check_error(L,1);
  const char *cat = 0;
  int n = 2;

  if(err->cat == LLUV_ERR_LIB)     cat = LLUV_ERR_LIB;
  else if(err->cat == LLUV_ERR_UV) cat = LLUV_ERR_UV_NAME;

  if(cat) lua_pushfstring(L, "[%s]", cat);
  else    lua_pushfstring(L, "[%d]", err->cat);

  lua_pushfstring(L, "[%s] %s (%d)",
    uv_err_name(err->no),
    uv_strerror(err->no),
    err->no
  );

  if(err->ext[0]){
    lua_pushfstring(L, " - %s", err->ext);
    n += 1;
  }

  lua_concat(L, n);

  return 1;
}

static int lluv_err_equal(lua_State *L){
  lluv_error_t *lhs = lluv_check_error(L, 1);
  lluv_error_t *rhs = lluv_check_error(L, 2);
  lua_pushboolean(L, ((lhs->no == rhs->no)&&(lhs->cat == rhs->cat))?1:0);
  return 1;
}

//}

LLUV_INTERNAL int lluv_fail(lua_State *L, lluv_flags_t flags, int error_category, uv_errno_t error_no, const char *ext){
  if(!(flags & LLUV_FLAG_RAISE_ERROR)){
    lua_pushnil(L);
    lluv_error_create(L, error_category, error_no, ext);
    return 2;
  }

  lluv_error_create(L, error_category, error_no, ext);
  return lua_error(L);
}

static int lluv_error_new(lua_State *L){
  int tp, no = luaL_checkint(L, 2);
  const char *ext = lua_tostring(L, 3);

  if(lua_isnumber(L, 1)){
    tp = luaL_checkint(L, 1);
  }
  else{
    const char* str = luaL_checkstring(L, 1);
    if(strcmp(str, LLUV_ERR_UV_NAME) == 0) tp = LLUV_ERR_UV;
    else if(strcmp(str, LLUV_ERR_LIB_NAME) == 0) tp = LLUV_ERR_LIB;
    else{
      lua_pushfstring(L, "Unknown error category: %s", str);
      return lua_error(L);
    }
  }

  //! @todo checks error type value

  lluv_error_create(L, tp, no, ext);
  return 1;
}

static lluv_uv_const_t lluv_error_constants[] = {

  /* error categories */
  { LLUV_ERR_LIB,        "ERROR_LIB"        },
  { LLUV_ERR_UV,         "ERROR_UV"         },

  /* error codes */
#define XX(N,M) {UV_##N, #N },
  UV_ERRNO_MAP(XX)
#undef  XX

  {0, NULL}
};

static const struct luaL_Reg lluv_err_methods[] = {
  { "no",              lluv_err_no               },
  { "msg",             lluv_err_msg              },
  { "name",            lluv_err_name             },
  { "ext",             lluv_err_ext              },
  { "cat",             lluv_err_category         },
  { "category",        lluv_err_category         },
  { "to_s",            lluv_err_tostring         },
  { "tostring",        lluv_err_tostring         },
  { "__tostring",      lluv_err_tostring         },
  { "__eq",            lluv_err_equal            },

  {NULL,NULL}
};

static const struct luaL_Reg lluv_error_functions[] = {
  { "error",     lluv_error_new     },

  {NULL,NULL}
};

LLUV_INTERNAL void lluv_error_initlib(lua_State *L, int nup, int safe){
  lutil_pushnvalues(L, nup);

  if(!lutil_createmetap(L, LLUV_ERROR, lluv_err_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);

  luaL_setfuncs(L, lluv_error_functions, nup);
  lluv_register_constants(L, lluv_error_constants);
}


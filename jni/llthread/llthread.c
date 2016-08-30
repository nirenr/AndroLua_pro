#if !defined(_WIN32) && !defined(USE_PTHREAD)
#  define USE_PTHREAD
#endif

#define LLTHREAD_VERSION_MAJOR 0
#define LLTHREAD_VERSION_MINOR 1
#define LLTHREAD_VERSION_PATCH 4
#define LLTHREAD_VERSION_COMMENT "dev"

#ifndef USE_PTHREAD
#  include <windows.h>
#  include <process.h>
#else
#  include <pthread.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <errno.h>
#include <lualib.h>
#include "l52util.h"
#include "traceback.inc"
#include "copy.inc"
#include<jni.h>
#include "luajava.h"

JavaVM *g_jvm = NULL;

/*export*/
#ifdef _WIN32
#  define LLTHREADS_EXPORT_API __declspec(dllexport)
#else
#  define LLTHREADS_EXPORT_API LUALIB_API
#endif

/* wrap strerror_s(). */
#ifdef _WIN32
#  ifdef __GNUC__
#    ifndef strerror_r
#      define strerror_r(errno, buf, buflen) do { \
         strncpy((buf), strerror(errno), (buflen)-1); \
         (buf)[(buflen)-1] = '\0'; \
       } while(0)
#    endif
#  else
#    ifndef strerror_r
#      define strerror_r(errno, buf, buflen) strerror_s((buf), (buflen), (errno))
#    endif
#  endif
#endif

#ifndef USE_PTHREAD
#  define OS_THREAD_RETURN      unsigned int __stdcall
#  define INVALID_THREAD        INVALID_HANDLE_VALUE
#  define INFINITE_JOIN_TIMEOUT INFINITE
#  define JOIN_OK               0
#  define JOIN_ETIMEDOUT        1
#  define JOIN_FAIL             2
typedef DWORD  join_timeout_t;
typedef HANDLE os_thread_t;
#else
#  define OS_THREAD_RETURN      void *
#  define INFINITE_JOIN_TIMEOUT -1
#  define JOIN_OK               0
#  define JOIN_ETIMEDOUT        ETIMEDOUT
typedef int       join_timeout_t;
typedef pthread_t os_thread_t;
#endif

#define ERROR_LEN 1024

#define flags_t unsigned char

#define FLAG_NONE      (flags_t)0
#define FLAG_STARTED   (flags_t)1<<0
#define FLAG_DETACHED  (flags_t)1<<1
#define FLAG_JOINED    (flags_t)1<<2
#define FLAG_JOINABLE  (flags_t)1<<3

/*At least one flag*/
#define FLAG_IS_SET(O, F) (O->flags & (flags_t)(F))
#define FLAG_SET(O, F) O->flags |= (flags_t)(F)
#define FLAG_UNSET(O, F) O->flags &= ~((flags_t)(F))
#define IS(O, F) FLAG_IS_SET(O, FLAG_##F)
#define SET(O, F) FLAG_SET(O, FLAG_##F)

#define ALLOC_STRUCT(S) (S*)calloc(1, sizeof(S))
#define FREE_STRUCT(O) free(O)

#ifndef LLTHREAD_MODULE_NAME
#  define LLTHREAD_MODULE_NAME llthreads
#endif

#define CAT(S1,S2) S1##S2

#define LLTHREAD_OPEN_NAME_IMPL(NAME) CAT(luaopen_, NAME)

#define LLTHREAD_OPEN_NAME luaopen_threads

LLTHREADS_EXPORT_API int LLTHREAD_OPEN_NAME(lua_State *L);

#define LLTHREAD_NAME "LLThread"
static const char *LLTHREAD_TAG = LLTHREAD_NAME;
static const char *LLTHREAD_LOGGER_HOLDER = LLTHREAD_NAME " logger holder";

typedef struct llthread_child_t {
  lua_State  *L;
  int        status;
  flags_t    flags;
} llthread_child_t;

typedef struct llthread_t {
  llthread_child_t *child;
  os_thread_t       thread;
  flags_t           flags;
} llthread_t;

static int fail(lua_State *L, const char *msg){
  lua_pushnil(L);
  lua_pushstring(L, msg);
  return 2;
}

//{ logger interface
void llthread_log(lua_State *L, const char *hdr, const char *msg){
  int top = lua_gettop(L);
  lua_rawgetp(L, LUA_REGISTRYINDEX, LLTHREAD_LOGGER_HOLDER);
  if(lua_isnil(L, -1)){
    lua_pop(L, 1);
    fputs(hdr,  stderr);
    fputs(msg,  stderr);
    fputc('\n', stderr);
    fflush(stderr);
    return;
  }
  lua_pushstring(L, hdr);
  lua_pushstring(L, msg);
  lua_concat(L, 2);
  lua_pcall(L, 1, 0, 0);
  lua_settop(L, top);
}
//}

//{ llthread_child

static void open_thread_libs(lua_State *L){
#define L_REGLIB(L, name) lua_pushcfunction(L, luaopen_##name); lua_setfield(L, -2)

  int top = lua_gettop(L);

#ifndef LLTHREAD_REGISTER_STD_LIBRARY

  luaL_openlibs(L);
  lua_getglobal(L, "package"); lua_getfield(L, -1, "preload"); lua_remove(L, -2);

#else

  lutil_require(L, "_G",      luaopen_base,    1);
  lutil_require(L, "package", luaopen_package, 1);
  lua_settop(L, top);

  /* get package.preload */
  lua_getglobal(L, "package"); lua_getfield(L, -1, "preload"); lua_remove(L, -2);
  L_REGLIB(L, io,        1);
  L_REGLIB(L, os,        1);
  L_REGLIB(L, math,      1);
  L_REGLIB(L, table,     1);
  L_REGLIB(L, string,    1);

#ifdef LUA_DBLIBNAME
  L_REGLIB(L, debug,     1);
#endif

  /* @fixme find out luaopen_XXX at runtime */
#ifdef LUA_JITLIBNAME
  L_REGLIB(L, bit,        1);
  L_REGLIB(L, jit,        1);
  L_REGLIB(L, ffi,        1);
#elif defined LUA_BITLIBNAME
  L_REGLIB(L, bit32,      1);
#endif

#endif

#ifdef LLTHREAD_REGISTER_THREAD_LIBRARY
  L_REGLIB(L, llthreads, 0);
#endif

  lua_settop(L, top);

#undef L_REGLIB
}

static llthread_child_t *llthread_child_new() {
  llthread_child_t *this = ALLOC_STRUCT(llthread_child_t);
  if(!this) return NULL;

  memset(this, 0, sizeof(llthread_child_t));

  /* create new lua_State for the thread.             */
  /* open standard libraries.                         */
  this->L = luaL_newstate();
  open_thread_libs(this->L);


  return this;
}

static void llthread_child_destroy(llthread_child_t *this) {
  lua_close(this->L);
  FREE_STRUCT(this);
}

static OS_THREAD_RETURN llthread_child_thread_run(void *arg) {
  llthread_child_t *this = (llthread_child_t *)arg;
  lua_State *L = this->L;
  int nargs = lua_gettop(L) - 1;

  /* push traceback function as first value on stack. */
  lua_pushcfunction(this->L, traceback); 
  lua_insert(L, 1);

  this->status = lua_pcall(L, nargs, LUA_MULTRET, 1);

  /* alwasy print errors here, helps with debugging bad code. */
  if(this->status != 0) {
    llthread_log(L, "Error from thread: ", lua_tostring(L, -1));
  }

  if(IS(this, DETACHED) || !IS(this, JOINABLE)) {
    /* thread is detached, so it must clean-up the child state. */
    llthread_child_destroy(this);
    this = NULL;
  }

#ifndef USE_PTHREAD
  if(this) {
    /* attached thread, don't close thread handle. */
    _endthreadex(0);
  } else {
    /* detached thread, close thread handle. */
    _endthread();
  }
  return 0;
#else
  return this;
#endif
}

//}

//{ llthread

static void llthread_validate(llthread_t *this){
  /* describe valid state of llthread_t object 
   * from after create and before destroy
   */
  if(!IS(this, STARTED)){
    assert(!IS(this, DETACHED));
    assert(!IS(this, JOINED));
    assert(!IS(this, JOINABLE));
    return;
  }

  if(IS(this, DETACHED)){
    if(!IS(this, JOINABLE)) assert(this->child == NULL);
    else assert(this->child != NULL);
  }
}

static int llthread_detach(llthread_t *this);

static int llthread_join(llthread_t *this, join_timeout_t timeout);

static llthread_t *llthread_new() {
  llthread_t *this = ALLOC_STRUCT(llthread_t);
  if(!this) return NULL;

  this->flags  = FLAG_NONE;
#ifndef USE_PTHREAD
  this->thread = INVALID_THREAD;
#endif
  this->child  = llthread_child_new();
  if(!this->child){
    FREE_STRUCT(this);
    return NULL;
  }

  return this;
}

static void llthread_cleanup_child(llthread_t *this) {
  if(this->child) {
    llthread_child_destroy(this->child);
    this->child = NULL;
  }
}

static void llthread_destroy(llthread_t *this) {
  do{
    /* thread not started */
    if(!IS(this, STARTED)){
      llthread_cleanup_child(this);
      break;
    }

    /* DETACHED */
    if(IS(this, DETACHED)){
      if(IS(this, JOINABLE)){
        llthread_detach(this);
      }
      break;
    }

    /* ATTACHED */
    if(!IS(this, JOINED)){
      llthread_join(this, INFINITE_JOIN_TIMEOUT);
      if(!IS(this, JOINED)){
        /* @todo use current lua state to logging */
        /*
         * char buf[ERROR_LEN];
         * strerror_r(errno, buf, ERROR_LEN);
         * llthread_log(L, "Error can not join thread on gc: ", buf);
         */
      }
    }
    if(IS(this, JOINABLE)){
      llthread_cleanup_child(this);
    }

  }while(0);

  FREE_STRUCT(this);
}

static int llthread_push_args(lua_State *L, llthread_child_t *child, int idx, int top) {
  return llthread_copy_values(L, child->L, idx, top, 1 /* is_arg */);
}

static int llthread_push_results(lua_State *L, llthread_child_t *child, int idx, int top) {
  return llthread_copy_values(child->L, L, idx, top, 0 /* is_arg */);
}

static int llthread_detach(llthread_t *this){
  int rc = 0;

  assert(IS(this, STARTED));
  assert(this->child != NULL);

  this->child = NULL;

  /*we can not detach joined thread*/
  if(IS(this, JOINED))
    return 0;

#ifdef USE_PTHREAD
  rc = pthread_detach(this->thread);
#else
  assert(this->thread != INVALID_THREAD);
  CloseHandle(this->thread);
  this->thread = INVALID_THREAD;
#endif
  return rc;
}

/*   | detached | joinable ||    join    | which thread  |  gc    | detach  |
 *   |          |          ||   return   | destroy child | calls  |   on    |
 *   ------------------------------------------------------------------------
 *   |   false  |   falas  ||  <NONE>    |    child      |  join  | <NEVER> |
 *  *|   false  |   true   || Lua values |    parent     |  join  | <NEVER> |
 *  *|   true   |   false  ||  <ERROR>   |    child      | <NONE> |  start  |
 *   |   true   |   true   ||  <NONE>    |    child      | detach |   gc    |
 *   ------------------------------------------------------------------------
 *   * llthread behavior.
 */
static int llthread_start(llthread_t *this, int start_detached, int joinable) {
  llthread_child_t *child = this->child;
  int rc = 0;

  llthread_validate(this);

  if(joinable) SET(child, JOINABLE);
  if(start_detached) SET(child, DETACHED);

#ifndef USE_PTHREAD
  this->thread = (HANDLE)_beginthreadex(NULL, 0, llthread_child_thread_run, child, 0, NULL);
  if(INVALID_THREAD == this->thread){
    rc = -1;
  }
#else
  rc = pthread_create(&(this->thread), NULL, llthread_child_thread_run, child);
#endif

  if(rc == 0) {
    SET(this, STARTED);
    if(joinable) SET(this, JOINABLE);
    if(start_detached) SET(this, DETACHED);
    if((start_detached)&&(!joinable)){
      rc = llthread_detach(this);
    }
  }

  llthread_validate(this);

  return rc;
}

static int llthread_join(llthread_t *this, join_timeout_t timeout) {
  llthread_validate(this);

  if(IS(this, JOINED)){
    return JOIN_OK;
  } else{
#ifndef USE_PTHREAD
  DWORD ret = 0;
  if(INVALID_THREAD == this->thread) return JOIN_OK;
  ret = WaitForSingleObject( this->thread, timeout );
  if( ret == WAIT_OBJECT_0){ /* Destroy the thread object. */
    CloseHandle( this->thread );
    this->thread = INVALID_THREAD;
    SET(this, JOINED);

    llthread_validate(this);

    return JOIN_OK;
  }
  else if( ret == WAIT_TIMEOUT ){
    return JOIN_ETIMEDOUT;
  }
  return JOIN_FAIL;
#else
  int rc;
  if(timeout == 0){
    rc = pthread_kill(this->thread, 0);
    if(rc == 0){ /* still alive */
      return JOIN_ETIMEDOUT;
    }

    if(rc != ESRCH){ 
      /*@fixme what else it can be ?*/
      return rc;
    }

    /*thread dead so we call join to free pthread_t struct */
  }

  /* @todo use pthread_tryjoin_np/pthread_timedjoin_np to support timeout */

  /* then join the thread. */
  rc = pthread_join(this->thread, NULL);
  if((rc == 0) || (rc == ESRCH)) {
    SET(this, JOINED);
    rc = JOIN_OK;
  }

  llthread_validate(this);

  return rc;
#endif
  }
}

static int llthread_alive(llthread_t *this) {
  llthread_validate(this);

  if(IS(this, JOINED)){
    return JOIN_OK;
  } else{
#ifndef USE_PTHREAD
  DWORD ret = 0;
  if(INVALID_THREAD == this->thread) return JOIN_OK;
  ret = WaitForSingleObject( this->thread, 0 );
  if( ret == WAIT_OBJECT_0) return JOIN_OK;
  if( ret == WAIT_TIMEOUT ) return JOIN_ETIMEDOUT;
  return JOIN_FAIL;
#else
  int rc = pthread_kill(this->thread, 0);
  if(rc == 0){ /* still alive */
    return JOIN_ETIMEDOUT;
  }

  if(rc != ESRCH){ 
    /*@fixme what else it can be ?*/
    return rc;
  }

  return JOIN_OK;
#endif
  }
}

static llthread_t *llthread_create(lua_State *L, const char *code, size_t code_len) {
  llthread_t *this        = llthread_new();
  llthread_child_t *child = this->child;
  if(g_jvm != NULL){
    JNIEnv *env=NULL;
    if((*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL) == JNI_OK){
      pushJNIEnv(env,child->L);
	}
  }
  /* load Lua code into child state. */
  int rc = luaL_loadbuffer(child->L, code, code_len, code);
  if(rc != 0) {
    /* copy error message to parent state. */
    size_t len; const char *str = lua_tolstring(child->L, -1, &len);
    if(str != NULL) {
      lua_pushlstring(L, str, len);
    } else {
      /* non-string error message. */
      lua_pushfstring(L, "luaL_loadbuffer() failed to load Lua code: rc=%d", rc);
    }
    llthread_destroy(this);
    lua_error(L);
    return NULL;
  }

  /* copy extra args from main state to child state. */
  /* Push all args after the Lua code. */
  llthread_push_args(L, child, 3, lua_gettop(L));

  llthread_validate(this);

  return this;
}

//}

//{ Lua interface to llthread

static llthread_t *l_llthread_at (lua_State *L, int i) {
  llthread_t **this = (llthread_t **)lutil_checkudatap (L, i, LLTHREAD_TAG);
  luaL_argcheck (L,  this != NULL, i, "thread expected");
  luaL_argcheck (L, *this != NULL, i, "thread expected");
  // luaL_argcheck (L, !(counter->flags & FLAG_DESTROYED), 1, "PDH Counter is destroyed");
  return *this;
}

static int l_llthread_delete(lua_State *L) {
  llthread_t **pthis = (llthread_t **)lutil_checkudatap (L, 1, LLTHREAD_TAG);
  luaL_argcheck (L, pthis != NULL, 1, "thread expected");
  if(*pthis == NULL) return 0;
  //(*g_jvm)->DetachCurrentThread(g_jvm);
  llthread_destroy(*pthis);
  *pthis = NULL;
   return 0;
}

static int l_llthread_start(lua_State *L) {
  llthread_t *this = l_llthread_at(L, 1);
  int start_detached = lua_toboolean(L, 2);
  int joinable, rc;

  if(!lua_isnone(L, 3)) joinable = lua_toboolean(L, 3);
  else joinable = start_detached ? 0 : 1;

  if(IS(this, STARTED)) {
    return fail(L, "Thread already started.");
  }

  rc = llthread_start(this, start_detached, joinable);
  if(rc != 0) {
    char buf[ERROR_LEN];
    strerror_r(errno, buf, ERROR_LEN);
    return fail(L, buf);
  }

  lua_settop(L, 1); // return this
  return 1;
}

static int l_llthread_join(lua_State *L) {
  llthread_t *this = l_llthread_at(L, 1);
  llthread_child_t *child = this->child;
  int rc;

  if(!IS(this, STARTED )) {
    return fail(L, "Can't join a thread that hasn't be started.");
  }
  if( IS(this, DETACHED) && !IS(this, JOINABLE)) {
    return fail(L, "Can't join a thread that has been detached.");
  }
  if( IS(this, JOINED  )) {
    return fail(L, "Can't join a thread that has already been joined.");
  }

  /* join the thread. */
  rc = llthread_join(this, luaL_optint(L, 2, INFINITE_JOIN_TIMEOUT));

  if(child && IS(this, JOINED)) {
    int top;

    if(IS(this, DETACHED) || !IS(this, JOINABLE)){
      /*child lua state has been destroyed by child thread*/
      /*@todo return thread exit code*/
      lua_pushboolean(L, 1);
      lua_pushnumber(L, 0);
      return 2;
    }
    
    /* copy values from child lua state */
    if(child->status != 0) {
      const char *err_msg = lua_tostring(child->L, -1);
      lua_pushboolean(L, 0);
      lua_pushfstring(L, "Error from child thread: %s", err_msg);
      top = 2;
    } else {
      lua_pushboolean(L, 1);
      top = lua_gettop(child->L);
      /* return results to parent thread. */
      llthread_push_results(L, child, 2, top);
    }

    llthread_cleanup_child(this);
    return top;
  }

  if( rc == JOIN_ETIMEDOUT ){
    return fail(L, "timeout");
  } 

  {
    char buf[ERROR_LEN];
    strerror_r(errno, buf, ERROR_LEN);

    /* llthread_cleanup_child(this); */

    return fail(L, buf);
  }

}

static int l_llthread_alive(lua_State *L) {
  llthread_t *this = l_llthread_at(L, 1);
  llthread_child_t *child = this->child;
  int rc;

  if(!IS(this, STARTED )) {
    return fail(L, "Can't join a thread that hasn't be started.");
  }
  if( IS(this, DETACHED) && !IS(this, JOINABLE)) {
    return fail(L, "Can't join a thread that has been detached.");
  }
  if( IS(this, JOINED  )) {
    return fail(L, "Can't join a thread that has already been joined.");
  }

  /* join the thread. */
  rc = llthread_alive(this);

  if( rc == JOIN_ETIMEDOUT ){
    lua_pushboolean(L, 1);
    return 1;
  }

  if(rc == JOIN_OK){
    lua_pushboolean(L, 0);
    return 1;
  }

  {
    char buf[ERROR_LEN];
    strerror_r(errno, buf, ERROR_LEN);

    /* llthread_cleanup_child(this); */

    return fail(L, buf);
  }

}

static int l_llthread_started(lua_State *L) {
  llthread_t *this = l_llthread_at(L, 1);
  lua_pushboolean(L, IS(this, STARTED)?1:0);
  return 1;
}

static int l_llthread_detached(lua_State *L) {
  llthread_t *this = l_llthread_at(L, 1);
  lua_pushboolean(L, IS(this, DETACHED)?1:0);
  return 1;
}

static int l_llthread_joinable(lua_State *L) {
  llthread_t *this = l_llthread_at(L, 1);
  lua_pushboolean(L, IS(this, JOINABLE)?1:0);
  return 1;
}

static int l_llthread_new(lua_State *L) {
  if(g_jvm == NULL){
	  JNIEnv *env=checkEnv(L);
	  (*env)->GetJavaVM(env,&g_jvm);
  }
  
  size_t lua_code_len; const char *lua_code = luaL_checklstring(L, 1, &lua_code_len);
  llthread_t **this = lutil_newudatap(L, llthread_t*, LLTHREAD_TAG);
  lua_insert(L, 2); /*move self prior args*/
  *this = llthread_create(L, lua_code, lua_code_len);

  lua_settop(L, 2);
  return 1;
}

static const struct luaL_Reg l_llthread_meth[] = {
  {"start",         l_llthread_start         },
  {"join",          l_llthread_join          },
  {"alive",         l_llthread_alive         },
  {"started",       l_llthread_started       },
  {"detached",      l_llthread_detached      },
  {"joinable",      l_llthread_joinable      },
  {"__gc",          l_llthread_delete        },

  {NULL, NULL}
};

//}

//{ version

static int l_llthread_version(lua_State *L){
  lua_pushinteger(L, LLTHREAD_VERSION_MAJOR);
  lua_pushinteger(L, LLTHREAD_VERSION_MINOR);
  lua_pushinteger(L, LLTHREAD_VERSION_PATCH);
#ifdef LLTHREAD_VERSION_COMMENT
  if(LLTHREAD_VERSION_COMMENT[0]){
    lua_pushliteral(L, LLTHREAD_VERSION_COMMENT);
    return 4;
  }
#endif
  return 3;
}

static int l_llthread_push_version(lua_State *L){
  lua_pushinteger(L, LLTHREAD_VERSION_MAJOR);
  lua_pushliteral(L, ".");
  lua_pushinteger(L, LLTHREAD_VERSION_MINOR);
  lua_pushliteral(L, ".");
  lua_pushinteger(L, LLTHREAD_VERSION_PATCH);
#ifdef LLTHREAD_VERSION_COMMENT
  if(LLTHREAD_VERSION_COMMENT[0]){
    lua_pushliteral(L, "-"LLTHREAD_VERSION_COMMENT);
    lua_concat(L, 6);
  }
  else
#endif
  lua_concat(L, 5);
  return 1;
}

//}

static int l_llthread_set_logger(lua_State *L){
  lua_settop(L, 1);
  luaL_argcheck(L, lua_isfunction(L, 1), 1, "function expected");
  lua_rawsetp(L, LUA_REGISTRYINDEX, LLTHREAD_LOGGER_HOLDER);
  return 0;
}

static const struct luaL_Reg l_llthreads_lib[] = {
  {"new",           l_llthread_new           },
  {"set_logger",    l_llthread_set_logger    },
  {"version",       l_llthread_version       },

  {NULL, NULL}
};

LLTHREADS_EXPORT_API int LLTHREAD_OPEN_NAME(lua_State *L) {
  int top = lua_gettop(L);
  lutil_createmetap(L, LLTHREAD_TAG,   l_llthread_meth,   0);
  lua_settop(L, top);

  lua_newtable(L);
  luaL_setfuncs(L, l_llthreads_lib, 0);

  lua_pushliteral(L, "_VERSION");
  l_llthread_push_version(L);
  lua_rawset(L, -3);

  return 1;
}

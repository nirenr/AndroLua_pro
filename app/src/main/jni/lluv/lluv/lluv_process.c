/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2016 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lluv library.
******************************************************************************/

#include "lluv.h"
#include "lluv_handle.h"
#include "lluv_stream.h"
#include "lluv_process.h"
#include "lluv_req.h"
#include "lluv_loop.h"
#include "lluv_error.h"
#include <memory.h>
#include <assert.h>

#ifdef _WIN32
#include <io.h> 
#else
#include <unistd.h>
#endif

#define LLUV_OS_HANDLE_NAME LLUV_PREFIX" OS Handle"
static const char *LLUV_OS_HANDLE = LLUV_OS_HANDLE_NAME;

typedef struct lluv_os_handle_tag{
  lluv_flags_t flags;
  int fd;
} lluv_os_handle_t;

static lluv_os_handle_t* lluv_check_os_handle(lua_State *L, int idx){
  lluv_os_handle_t *handle = (lluv_os_handle_t *)lutil_checkudatap (L, idx, LLUV_OS_HANDLE);
  luaL_argcheck (L, handle != NULL, idx, LLUV_OS_HANDLE_NAME" expected");

  luaL_argcheck (L, IS(handle, LLUV_FLAG_OPEN), idx, LLUV_OS_HANDLE_NAME" closed");

  return handle;
}

#ifdef _WIN32

#define IS_VALID_HANDLE(v) (((v) != (HANDLE)0) && ((v) != (HANDLE)-2)&& ((v) != INVALID_HANDLE_VALUE))

static DWORD lluv_duplicate_handle(HANDLE handle, HANDLE* dup) {
  if(!IS_VALID_HANDLE(handle)){
    *dup = INVALID_HANDLE_VALUE;
    return ERROR_INVALID_HANDLE;
  }
  else{
    HANDLE current_process = GetCurrentProcess();
    BOOL ret = DuplicateHandle(current_process, handle, current_process, dup, 0, TRUE, DUPLICATE_SAME_ACCESS);

    if(!ret){
      *dup = INVALID_HANDLE_VALUE;
      return GetLastError();
    }
  }

  return ERROR_SUCCESS;
}

#else

#define IS_VALID_HANDLE(v) (((v) != -1))

static int lluv_duplicate_handle(int oldfd, int* newfd) {
  if(!IS_VALID_HANDLE(oldfd)){
    *newfd = -1;
    return EBADF;
  }

  *newfd = dup(oldfd);

  if(!IS_VALID_HANDLE(*newfd)){
    *newfd = -1;
    return EBADF;
  }

  return 0;
}

#endif

LLUV_IMPL_SAFE(lluv_os_handle_new){
  lluv_os_handle_t *h;
  int dublicate = lua_toboolean(L, 2);

#ifdef _WIN32
  HANDLE newfd, handle = (HANDLE)lutil_checkint64(L, 1);
  if(dublicate){
    DWORD err = lluv_duplicate_handle(handle, &newfd);
    if(err){
      //! @todo return error.
      return 0;
    }
    if(newfd == INVALID_HANDLE_VALUE){
      //! @todo return error.
      return 0;
    }
    handle = newfd;
  }
#else
  int newfd, handle = luaL_checkinteger(L, 1);
  if(dublicate){
    int err = lluv_duplicate_handle(handle, &newfd);
    if(err){
      //! @todo return error.
      return 0;
    }
    handle = newfd;
  }
#endif

  h = lutil_newudatap(L, lluv_os_handle_t, LLUV_OS_HANDLE);
  h->flags = LLUV_FLAG_OPEN;

#ifdef _WIN32
  h->fd = _open_osfhandle((intptr_t)handle, 0);
#else
  h->fd = handle;
#endif

  return 1;
}

static int lluv_os_handle_fd(lua_State *L){
  lluv_os_handle_t *handle = lluv_check_os_handle(L, 1);

  lutil_pushint64(L, handle->fd);
  return 1;
}

static int lluv_os_handle_to_s(lua_State *L){
  lluv_os_handle_t *handle = (lluv_os_handle_t *)lutil_checkudatap (L, 1, LLUV_OS_HANDLE);
  luaL_argcheck (L, handle != NULL, 1, LLUV_OS_HANDLE_NAME" expected");

  lua_pushfstring(L, LLUV_OS_HANDLE_NAME" - %p (%p)", (void*)((intptr_t)handle->fd), handle);

  if(!IS(handle, LLUV_FLAG_OPEN)){
    lua_pushliteral(L, " (closed)");
    lua_concat(L, 2);
  }

  return 1;
}

static int lluv_os_handle_close(lua_State *L){
  lluv_os_handle_t *handle = (lluv_os_handle_t *)lutil_checkudatap (L, 1, LLUV_OS_HANDLE);
  luaL_argcheck (L, handle != NULL, 1, LLUV_OS_HANDLE_NAME" expected");

  if(IS(handle, LLUV_FLAG_OPEN)){
#ifdef _WIN32
    _close(handle->fd);
#else
    close(handle->fd);
#endif
    UNSET(handle, LLUV_FLAG_OPEN);
    handle->fd = 0;
  }

  lua_pushboolean(L, 1);
  return 1;
}

static const struct luaL_Reg lluv_os_handle_methods[] = {
  { "fd",                                 lluv_os_handle_fd                        },
  { "getfd",                              lluv_os_handle_fd                        },
  { "close",                              lluv_os_handle_close                     },
  { "__gc",                               lluv_os_handle_close                     },
  { "__tostring",                         lluv_os_handle_to_s                      },

  {NULL,NULL}
};

#define LLUV_PROCESS_NAME LLUV_PREFIX" Process"
static const char *LLUV_PROCESS = LLUV_PROCESS_NAME;

static void lluv_on_process_exit(uv_process_t* arg, int64_t exit_status, int term_signal){
  lluv_handle_t *handle = lluv_handle_byptr((uv_handle_t*)arg);
  lua_State *L = LLUV_HCALLBACK_L(handle);

  LLUV_CHECK_LOOP_CB_INVARIANT(L);

  if(!IS_(handle, OPEN)){
    lluv_handle_unlock(L, handle, LLUV_LOCK_EXIT);

    LLUV_CHECK_LOOP_CB_INVARIANT(L);
    return;
  }

  lua_rawgeti(L, LLUV_LUA_REGISTRY, LLUV_EXIT_CB(handle));
  lluv_handle_pushself(L, handle);
  lluv_handle_unlock(L, handle, LLUV_LOCK_EXIT);

  if(lua_isnil(L, -2)){
    lua_pop(L, 2);

    LLUV_CHECK_LOOP_CB_INVARIANT(L);
    return;
  }

  lua_pushnil(L);
  lutil_pushint64(L, exit_status);
  lutil_pushint64(L, term_signal);

  LLUV_HANDLE_CALL_CB(L, handle, 4);

  LLUV_CHECK_LOOP_CB_INVARIANT(L);
}

static void rawgets(lua_State *L, int idx, const char *name){
  idx = lua_absindex(L, idx);
  lua_pushstring(L, name);
  lua_rawget(L, idx);
}

static char* opt_get_string(lua_State *L, int idx, const char *name, int req, const char *err){
  const char *value;
  idx = lua_absindex(L, idx);
  rawgets(L, idx, name);
  value = lua_tostring(L, -1);
  lua_pop(L, 1);
  if(value) return (char*)value;
  if(req){
    lua_pushstring(L, err);
    lua_error(L);
  }
  return 0;
}

static char** opt_get_sarray(lua_State *L, int idx, const char *name, int req, char *first_value, const char *err){
  char **value; size_t n, j, i = 0;
  idx = lua_absindex(L, idx);
  rawgets(L, idx, name);

  if(lua_isnil(L, -1)){
    lua_pop(L, 1);
    if(req){
      lua_pushstring(L, err);
      lua_error(L);
    }
    return 0;
  }

  if(!lua_istable(L, -1)){
    lua_pop(L, 1);
    lua_pushstring(L, err);
    lua_error(L);
    return 0;
  }

  n = lua_objlen(L, -1);

  if(!n && !first_value){
    lua_pop(L, 1);
    return 0;
  }

  value = lluv_alloc(L, sizeof(char*) * (n + (first_value ? 2 : 1)));

  if(first_value) value[i++] = first_value;

  for(j=0; j<n; ++j){
    lua_rawgeti(L, -1, j + 1);
    value[i++] = (char*)luaL_checkstring(L, -1);
    lua_pop(L, 1);
  }
  lua_pop(L, 1);

  value[i] = NULL;
  return value;
}

static int64_t opt_get_int64(lua_State *L, int idx, const char *name, int req, const char *err){
  int64_t value;
  idx = lua_absindex(L, idx);
  rawgets(L, idx, name);

  if(lua_isnil(L, -1)){
    lua_pop(L, 1);
    if(req){
      lua_pushstring(L, err);
      return lua_error(L);
    }
    return 0;
  }

  if(!lua_isnumber(L, -1)){
    lua_pop(L, 1);
    lua_pushstring(L, err);
    return lua_error(L);
  }

  value = lutil_checkint64(L, -1);
  lua_pop(L, 1);
  return value;
}

static int opt_exists(lua_State *L, int idx, const char *name){
  int ret;
  rawgets(L, idx, name);
  ret = lua_isnil(L, -1) ? 0 : 1;
  lua_pop(L, 1);
  return ret;
}

static void opt_get_stdio(lua_State *L, int idx, uv_process_options_t *opt){
  static const lluv_uv_const_t FLAGS[] = {
    { UV_IGNORE,         "ignore"         },
    { UV_CREATE_PIPE,    "create_pipe"    },
    { UV_INHERIT_FD,     "inherit_fd"     },
    { UV_INHERIT_STREAM, "inherit_stream" },
    { UV_READABLE_PIPE,  "readable_pipe"  },
    { UV_WRITABLE_PIPE,  "writable_pipe"  },

    { 0, NULL }
  };

  size_t i, n;

  rawgets(L, idx, "stdio");
  if(lua_isnil(L, -1)){
    lua_settop(L, 1);
    return;
  }

  if(!lua_istable(L, -1)){
    lua_pop(L, 1);
    lua_pushstring(L, "stdio option must be an array");
    lua_error(L);
    return;
  }

  n = lua_objlen(L, -1);

  if(n == 0){
    lua_pop(L, 1);
    return;
  }

  opt->stdio = lluv_alloc(L, n * sizeof(*opt->stdio));
  opt->stdio_count = n;

  for(i = 0; i < n; ++i){
    lua_rawgeti(L, -1, i + 1);

    if(lua_istable(L, -1)){
      uv_stdio_flags flags = 0;

      if(opt_exists(L, -1, "fd")){
        if(lutil_isudatap(L, -1, LLUV_OS_HANDLE)){
          lluv_os_handle_t *h = lluv_check_os_handle(L, -1);
          opt->stdio[i].data.fd = h->fd;
        }
        else{
          opt->stdio[i].data.fd = (int)opt_get_int64 (L, -1, "fd",  0, "stdio.fd option must be a number or OS handle object" );
        }
        flags = UV_INHERIT_FD;
      }
      else if(opt_exists(L, -1, "stream")){
        lluv_handle_t *handle;
        rawgets(L, -1, "stream");
        handle = lluv_check_stream(L, -1, LLUV_FLAG_OPEN);
        lua_pop(L, 1);
        opt->stdio[i].data.stream = LLUV_H(handle, uv_stream_t);
        flags = UV_INHERIT_STREAM;
      }
      else{
        opt->stdio[i].data.fd = 0;
        flags = UV_IGNORE;
      }

      if(opt_exists(L, -1, "flags")){
        rawgets(L, -1, "flags");
        flags = lluv_opt_flags_ui(L, -1, 0, FLAGS);
        lua_pop(L, 1);
        // flags = opt_get_int64 (L, -1, "flags",  0, "stdio.flags option must be a number" );
      }

      opt->stdio[i].flags = flags;
    }
    else if(lua_isnumber(L, -1)){
      opt->stdio[i].data.fd = (int)lutil_checkint64(L, -1);
      opt->stdio[i].flags = UV_INHERIT_FD;
    }
    else if(lua_isuserdata(L, -1)){
      if(lutil_isudatap(L, -1, LLUV_OS_HANDLE)){
        lluv_os_handle_t *h = lluv_check_os_handle(L, -1);
        opt->stdio[i].data.fd = h->fd;
        opt->stdio[i].flags = UV_INHERIT_FD;
      }
      else{
        lluv_handle_t *handle = lluv_check_stream(L, -1, LLUV_FLAG_OPEN);
        opt->stdio[i].data.stream = LLUV_H(handle, uv_stream_t);
        opt->stdio[i].flags = UV_INHERIT_STREAM;
      }
    }
    else{
      lua_pushstring(L, "stdio element must be table, stream or number");
      lua_error(L);
      return;
    }

    lua_pop(L, 1);
  }
  lua_pop(L, 1);
}

static int lluv_fill_process_options_(lua_State *L){
  static const lluv_uv_const_t FLAGS[] = {
    { UV_PROCESS_SETUID,                     "setuid"   },
    { UV_PROCESS_SETGID,                     "setgid"   },
    { UV_PROCESS_WINDOWS_VERBATIM_ARGUMENTS, "verbatim" },
    { UV_PROCESS_DETACHED,                   "detached" },
    { UV_PROCESS_WINDOWS_HIDE,               "hide"     },

    { 0, NULL }
  };

  uv_process_options_t *opt = (uv_process_options_t *)lua_touserdata(L, 2);
  unsigned int flags = opt->flags;

  lua_settop(L, 1);

  opt->exit_cb = lluv_on_process_exit;

  if(lua_isstring(L, 1)){
    opt->file = (char*) lua_tostring(L, 1);
    return 1;
  }

  luaL_checktype(L, 1, LUA_TTABLE);

  opt->file  =           opt_get_string(L, 1, "file",   1, "file option required and must be a string");
  opt->cwd   =           opt_get_string(L, 1, "cwd",    0, "cwd option must be a string");
  opt->args  =           opt_get_sarray(L, 1, "args",   0, (char*)opt->file, "args option must be an array");
  opt->env   =           opt_get_sarray(L, 1, "env",    0, NULL, "env option must be an array");
  opt->uid   = (uv_uid_t)opt_get_int64 (L, 1, "uid",    0, "uid option must be a number"   );
  opt->gid   = (uv_gid_t)opt_get_int64 (L, 1, "gid",    0, "gid option must be a number"   );

  if(opt_exists(L, 1, "uid")) flags |= UV_PROCESS_SETUID;
  if(opt_exists(L, 1, "gid")) flags |= UV_PROCESS_SETGID;

  rawgets(L, 1, "flags");
  opt->flags = flags | lluv_opt_flags_ui(L, -1, 0, FLAGS);
  lua_pop(L, 1);

  opt_get_stdio(L, 1, opt);

  lua_settop(L, 1);
  return 1;
}

LLUV_INTERNAL int lluv_process_index(lua_State *L){
  return lluv__index(L, LLUV_PROCESS, lluv_handle_index);
}

static lluv_handle_t* lluv_check_process(lua_State *L, int idx, lluv_flags_t flags){
  lluv_handle_t *handle = lluv_check_handle(L, idx, flags);
  luaL_argcheck (L, LLUV_H(handle, uv_handle_t)->type == UV_PROCESS, idx, LLUV_PROCESS_NAME" expected");

  return handle;
}

LLUV_IMPL_SAFE(lluv_process_spawn){
  lluv_loop_t *loop  = lluv_opt_loop(L, 1, LLUV_FLAG_OPEN);
  int first_arg = loop ? 2 : 1;
  int cb = LUA_NOREF;
  uv_process_options_t opt;

  memset(&opt, 0, sizeof(opt));

  if(!lua_isnone(L, first_arg + 1)){
    lluv_check_callable(L, first_arg + 1);
    lua_settop(L, first_arg + 1);
    cb = luaL_ref(L, LLUV_LUA_REGISTRY);
  }
  lua_settop(L, 1);

  lua_pushlightuserdata(L, &opt);
  lua_pushvalue(L, LLUV_LUA_REGISTRY);
  lua_pushvalue(L, LLUV_LUA_HANDLES);
  lua_pushcclosure(L, lluv_fill_process_options_, 2);
  lua_insert(L, 1);

  if(lua_pcall(L, 2, 1, 0)){
    if(opt.args)  lluv_free(L, opt.args);
    if(opt.env)   lluv_free(L, opt.env);
    if(opt.stdio) lluv_free(L, opt.stdio);
    luaL_unref(L, LLUV_LUA_REGISTRY, cb);
    return lua_error(L);
  }

  if(!loop) loop = lluv_default_loop(L);

  {
    lluv_handle_t *handle = lluv_handle_create(L, UV_PROCESS, safe_flag | INHERITE_FLAGS(loop));
    int err = uv_spawn(loop->handle, LLUV_H(handle, uv_process_t), &opt);

    if(opt.args)  lluv_free(L, opt.args);
    if(opt.env)   lluv_free(L, opt.env);
    if(opt.stdio) lluv_free(L, opt.stdio);

    if(cb != LUA_NOREF) lluv_handle_lock(L, handle, LLUV_LOCK_EXIT);

    if(err < 0){
      if(cb == LUA_NOREF){
        lua_getfield(L, -1, "close");
        lua_pushvalue(L, -2);
        lua_pcall(L, 1, 0, 0);
        return lluv_fail(L, safe_flag | loop->flags, LLUV_ERR_UV, (uv_errno_t)err, opt.file);
      }
      lua_rawgeti(L, LLUV_LUA_REGISTRY, cb);
      lua_pushvalue(L, -2);
      lluv_error_create(L, LLUV_ERR_UV, (uv_errno_t)err, opt.file);
      lluv_loop_defer_call(L, loop, 2);
    }
    LLUV_EXIT_CB(handle) = cb;

    lutil_pushint64(L, LLUV_H(handle, uv_process_t)->pid);
    return 2;
  }
}

static int lluv_process_pid(lua_State *L){
  lluv_handle_t *handle = lluv_check_process(L, 1, LLUV_FLAG_OPEN);
  lutil_pushint64(L, LLUV_H(handle, uv_process_t)->pid);
  return 1;
}

static int lluv_process_kill(lua_State *L){
  lluv_handle_t *handle = lluv_check_process(L, 1, LLUV_FLAG_OPEN);
  int sig = luaL_optint(L, 2, SIGTERM);
  int err = uv_process_kill(LLUV_H(handle, uv_process_t), sig);
  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, (uv_errno_t)err, NULL);
  }
  lua_settop(L, 1);
  return 1;
}

LLUV_IMPL_SAFE(lluv_pid_kill){
  int64_t pid = lutil_checkint64(L, 1);
  int sig = luaL_optint(L, 2, SIGTERM);
  int err = uv_kill(pid, sig);
  if(err < 0){
    return lluv_fail(L, safe_flag, LLUV_ERR_UV, (uv_errno_t)err, NULL);
  }
  lua_pushboolean(L, 1);
  return 1;
}

static int lluv_disable_stdio_inheritance(lua_State* L) {
  uv_disable_stdio_inheritance();
  return 0;
}

static const struct luaL_Reg lluv_process_methods[] = {
  { "pid",                                lluv_process_pid                      },
  { "kill",                               lluv_process_kill                     },

  {NULL,NULL}
};

static const lluv_uv_const_t lluv_process_constants[] = {
  /* uv_process_flags  */
  { UV_PROCESS_SETUID,                     "PROCESS_SETUID"                     },
  { UV_PROCESS_SETGID,                     "PROCESS_SETGID"                     },
  { UV_PROCESS_WINDOWS_VERBATIM_ARGUMENTS, "PROCESS_WINDOWS_VERBATIM_ARGUMENTS" },
  { UV_PROCESS_DETACHED,                   "PROCESS_DETACHED"                   },
  { UV_PROCESS_WINDOWS_HIDE,               "PROCESS_WINDOWS_HIDE"               },
  
  /* uv_stdio_flags */
  { UV_IGNORE,                             "IGNORE"                             },
  { UV_CREATE_PIPE,                        "CREATE_PIPE"                        },
  { UV_INHERIT_FD,                         "INHERIT_FD"                         },
  { UV_INHERIT_STREAM,                     "INHERIT_STREAM"                     },
  { UV_READABLE_PIPE,                      "READABLE_PIPE"                      },
  { UV_WRITABLE_PIPE,                      "WRITABLE_PIPE"                      },

  { 0, NULL }
};

#define LLUV_FUNCTIONS(F)                                        \
  {"os_handle", lluv_os_handle_new_##F},                         \
  {"spawn", lluv_process_spawn_##F},                             \
  {"kill", lluv_pid_kill_##F},                                   \
  {"disable_stdio_inheritance", lluv_disable_stdio_inheritance}, \

static const struct luaL_Reg lluv_functions[][5] = {
  {
    LLUV_FUNCTIONS(unsafe)

    {NULL,NULL}
  },
  {
    LLUV_FUNCTIONS(safe)

    {NULL,NULL}
  },
};

LLUV_INTERNAL void lluv_process_initlib(lua_State *L, int nup, int safe){
  lutil_pushnvalues(L, nup);
  if(!lutil_createmetap(L, LLUV_PROCESS, lluv_process_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);

  lutil_pushnvalues(L, nup);
  if(!lutil_createmetap(L, LLUV_OS_HANDLE, lluv_os_handle_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);

  luaL_setfuncs(L, lluv_functions[safe], nup);
  lluv_register_constants(L, lluv_process_constants);
}

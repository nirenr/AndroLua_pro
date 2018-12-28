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
#include "lluv_pipe.h"
#include "lluv_loop.h"
#include "lluv_error.h"
#include "lluv_req.h"
#include <assert.h>

#define LLUV_PIPE_NAME LLUV_PREFIX" Pipe"
static const char *LLUV_PIPE = LLUV_PIPE_NAME;

LLUV_INTERNAL int lluv_pipe_index(lua_State *L){
  return lluv__index(L, LLUV_PIPE, lluv_stream_index);
}

LLUV_IMPL_SAFE_(lluv_pipe_create){
  lluv_loop_t *loop  = lluv_opt_loop(L, 1, LLUV_FLAG_OPEN);
  int ipc = lua_toboolean(L, loop ? 2 : 1);
  lluv_handle_t *handle; int err;

  if(!loop) loop = lluv_default_loop(L);

  handle = lluv_stream_create(L, UV_NAMED_PIPE, safe_flag | INHERITE_FLAGS(loop));
  err = uv_pipe_init(loop->handle, LLUV_H(handle, uv_pipe_t), ipc);
  if(err < 0){
    lluv_handle_cleanup(L, handle, -1);
    return lluv_fail(L, safe_flag | loop->flags, LLUV_ERR_UV, (uv_errno_t)err, NULL);
  }
  return 1;
}

static lluv_handle_t* lluv_check_pipe(lua_State *L, int idx, lluv_flags_t flags){
  lluv_handle_t *handle = lluv_check_stream(L, idx, flags);
  luaL_argcheck (L, LLUV_H(handle, uv_handle_t)->type == UV_NAMED_PIPE, idx, LLUV_PIPE_NAME" expected");

  return handle;
}

static int lluv_pipe_open(lua_State *L){
  lluv_handle_t  *handle = lluv_check_pipe(L, 1, LLUV_FLAG_OPEN);
  uv_file fd = (uv_file)lutil_checkint64(L, 2);
  int err = uv_pipe_open(LLUV_H(handle, uv_pipe_t), fd);
  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }

  lua_settop(L, 1);
  return 1;
}

#if LLUV_UV_VER_GE(1, 16, 0)

static int lluv_pipe_chmod(lua_State *L){
  static const lluv_uv_const_t FLAGS[] = {
    { UV_READABLE, "read"     },
    { UV_READABLE, "readable" },
    { UV_WRITABLE, "write"    },
    { UV_WRITABLE, "writable" },

    { 0, NULL }
  };

  lluv_handle_t *handle = lluv_check_pipe(L, 1, LLUV_FLAG_OPEN);
  int flags = UV_READABLE, err;

  if(!lua_isnoneornil(L, 2)){
    flags = lluv_opt_flags_ui_2(L, 2, flags, FLAGS);
  }

  lua_settop(L, 1);

  err = uv_pipe_chmod(LLUV_H(handle, uv_pipe_t), flags);
  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }

  return 1;
}

#endif

static int lluv_pipe_bind(lua_State *L){
  lluv_handle_t *handle = lluv_check_pipe(L, 1, LLUV_FLAG_OPEN);
  const char      *addr = luaL_checkstring(L, 2);
  int               err = uv_pipe_bind(LLUV_H(handle, uv_pipe_t), addr);
  int top = lua_gettop(L);
  if(top > 3) lua_settop(L, top = 3);

  if(err < 0){
    if(!lua_isfunction(L, top)){
      return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, addr);
    }
    lua_pushvalue(L, 1);
    lluv_error_create(L, LLUV_ERR_UV, err, addr);
    lluv_loop_defer_call(L, lluv_loop_by_handle(&handle->handle), 2);
  }
  else if(lua_isfunction(L, top)){
    lua_pushvalue(L, 1);
    lua_pushnil(L);
    lua_pushvalue(L, 2);
    lluv_loop_defer_call(L, lluv_loop_by_handle(&handle->handle), 3);
  }

  lua_settop(L, 1);
  return 1;
}

static int lluv_pipe_ipc(lua_State *L){
  lluv_handle_t *handle = lluv_check_pipe(L, 1, LLUV_FLAG_OPEN);
  lua_settop(L, 1);
  lua_pushboolean(L, LLUV_H(handle, uv_pipe_t)->ipc);
  return 1;
}

static int lluv_pipe_connect(lua_State *L){
  lluv_handle_t  *handle = lluv_check_pipe(L, 1, LLUV_FLAG_OPEN);
  const char       *addr = luaL_checkstring(L, 2);
  lluv_req_t       *req;

  lluv_check_args_with_cb(L, 3);
  req = lluv_req_new(L, UV_CONNECT, handle);

  uv_pipe_connect( LLUV_R(req, connect), LLUV_H(handle, uv_pipe_t), addr, lluv_on_stream_connect_cb);

  lua_settop(L, 1);
  return 1;
}

static int lluv_pipe_getsockname(lua_State *L){
  lluv_handle_t  *handle = lluv_check_pipe(L, 1, LLUV_FLAG_OPEN);
  char buf[255]; size_t len = sizeof(buf);
  int err = uv_pipe_getsockname(LLUV_H(handle, uv_pipe_t), buf, &len);
  if(err >= 0){
    lua_pushlstring(L, buf, len);
    return 1;
  }
  if(err != UV_ENOBUFS){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }
  {
    char *buf = lluv_alloc(L, len);
    if(!buf){
      return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
    }
    err = uv_pipe_getsockname(LLUV_H(handle, uv_pipe_t), buf, &len);
    if(err < 0){
      lluv_free(L, buf);
      return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
    }
    lua_pushlstring(L, buf, len);
    lluv_free(L, buf);
    return 1;
  }
}

#if LLUV_UV_VER_GE(1,3,0)

static int lluv_pipe_getpeername(lua_State *L){
  lluv_handle_t  *handle = lluv_check_pipe(L, 1, LLUV_FLAG_OPEN);
  char buf[255]; size_t len = sizeof(buf);
  int err = uv_pipe_getpeername(LLUV_H(handle, uv_pipe_t), buf, &len);
  if(err >= 0){
    lua_pushlstring(L, buf, len);
    return 1;
  }
  if(err != UV_ENOBUFS){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }
  {
    char *buf = lluv_alloc(L, len);
    if(!buf){
      return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
    }
    err = uv_pipe_getpeername(LLUV_H(handle, uv_pipe_t), buf, &len);
    if(err < 0){
      lluv_free(L, buf);
      return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
    }
    lua_pushlstring(L, buf, len);
    lluv_free(L, buf);
    return 1;
  }
}

#endif

static int lluv_pipe_pending_instances(lua_State *L){
  lluv_handle_t  *handle = lluv_check_pipe(L, 1, LLUV_FLAG_OPEN);
  int count              = luaL_checkint(L, 2);

  uv_pipe_pending_instances(LLUV_H(handle, uv_pipe_t), count);

  lua_settop(L, 1);
  return 1;
}

static int lluv_pipe_pending_count(lua_State *L){
  lluv_handle_t  *handle = lluv_check_pipe(L, 1, LLUV_FLAG_OPEN);
  int err = uv_pipe_pending_count(LLUV_H(handle, uv_pipe_t));
  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }
  lua_pushnumber(L, err);
  return 1;
}

static int lluv_pipe_pending_type(lua_State *L){
  lluv_handle_t  *handle = lluv_check_pipe(L, 1, LLUV_FLAG_OPEN);
  uv_handle_type    type = uv_pipe_pending_type(LLUV_H(handle, uv_pipe_t));
  lua_pushnumber(L, type);
  return 1;
}

static const struct luaL_Reg lluv_pipe_methods[] = {
  { "open",              lluv_pipe_open              },
  { "bind",              lluv_pipe_bind              },
  { "connect",           lluv_pipe_connect           },
  { "getsockname",       lluv_pipe_getsockname       },
  { "ipc",               lluv_pipe_ipc               },
#if LLUV_UV_VER_GE(1,3,0)
  { "getpeername",       lluv_pipe_getpeername       },
#endif
  { "pending_instances", lluv_pipe_pending_instances },
  { "pending_count",     lluv_pipe_pending_count     },
  { "pending_type",      lluv_pipe_pending_type      },
#if LLUV_UV_VER_GE(1, 16, 0)
  { "chmod",             lluv_pipe_chmod             },
#endif

  {NULL,NULL}
};

#define LLUV_FUNCTIONS(F)         \
  {"pipe", lluv_pipe_create_##F}, \

static const struct luaL_Reg lluv_functions[][2] = {
  {
    LLUV_FUNCTIONS(unsafe)

    {NULL,NULL}
  },
  {
    LLUV_FUNCTIONS(safe)

    {NULL,NULL}
  },
};

LLUV_INTERNAL void lluv_pipe_initlib(lua_State *L, int nup, int safe){
  lutil_pushnvalues(L, nup);
  if(!lutil_createmetap(L, LLUV_PIPE, lluv_pipe_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);

  luaL_setfuncs(L, lluv_functions[safe], nup);
}

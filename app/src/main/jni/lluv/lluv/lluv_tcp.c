/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2018 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lluv library.
******************************************************************************/

#include "lluv.h"
#include "lluv_handle.h"
#include "lluv_stream.h"
#include "lluv_tcp.h"
#include "lluv_loop.h"
#include "lluv_error.h"
#include "lluv_req.h"
#include <assert.h>

#define LLUV_TCP_NAME LLUV_PREFIX" tcp"
static const char *LLUV_TCP = LLUV_TCP_NAME;

LLUV_INTERNAL int lluv_tcp_index(lua_State *L){
  return lluv__index(L, LLUV_TCP, lluv_stream_index);
}

LLUV_IMPL_SAFE_(lluv_tcp_create){
  lluv_loop_t   *loop   = lluv_opt_loop(L, 1, LLUV_FLAG_OPEN);
  lluv_handle_t *handle;
  int err;

#if LLUV_UV_VER_GE(1,7,0)
  unsigned int flags = lluv_opt_af_flags(L, loop ? 2 : 1, AF_UNSPEC);
#endif

  if(!loop) loop = lluv_default_loop(L);

  handle = lluv_stream_create(L, UV_TCP, safe_flag | INHERITE_FLAGS(loop));

#if LLUV_UV_VER_GE(1,7,0)
  err = uv_tcp_init_ex(loop->handle, LLUV_H(handle, uv_tcp_t), flags);
#else
  err = uv_tcp_init(loop->handle, LLUV_H(handle, uv_tcp_t));
#endif

  if(err < 0){
    lluv_handle_cleanup(L, handle, -1);
    return lluv_fail(L, safe_flag | loop->flags, LLUV_ERR_UV, (uv_errno_t)err, NULL);
  }
  return 1;
}

static lluv_handle_t* lluv_check_tcp(lua_State *L, int idx, lluv_flags_t flags){
  lluv_handle_t *handle = lluv_check_stream(L, idx, LLUV_FLAG_OPEN);
  luaL_argcheck (L, LLUV_H(handle, uv_handle_t)->type == UV_TCP, idx, LLUV_TCP_NAME" expected");

  luaL_argcheck (L, FLAGS_IS_SET(handle->flags, flags), idx, LLUV_TCP_NAME" closed");
  return handle;
}

static int lluv_tcp_connect(lua_State *L){
  lluv_handle_t  *handle = lluv_check_tcp(L, 1, LLUV_FLAG_OPEN);
  struct sockaddr_storage sa; lluv_req_t *req;
  int err = lluv_check_addr(L, 2, &sa);

  if(err < 0){
    lua_settop(L, 3);
    lua_pushliteral(L, ":");lua_insert(L, -2);lua_concat(L, 3);
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, lua_tostring(L, -1));
  }

  lluv_check_args_with_cb(L, 4);

  req = lluv_req_new(L, UV_CONNECT, handle);

  err = uv_tcp_connect(LLUV_R(req, connect), LLUV_H(handle, uv_tcp_t), (struct sockaddr *)&sa, lluv_on_stream_connect_cb);

  return lluv_return_req(L, handle, req, err);
}

static int lluv_tcp_bind(lua_State *L){
  static const lluv_uv_const_t FLAGS[] = {
    { UV_TCP_IPV6ONLY ,   "ipv6only"   },

    { 0, NULL }
  };

  lluv_handle_t  *handle = lluv_check_tcp(L, 1, LLUV_FLAG_OPEN);
  struct sockaddr_storage sa; int err = lluv_check_addr(L, 2, &sa);
  unsigned int flags = 0;
  int top = lua_gettop(L);
  if(top > 5)lua_settop(L, top = 5);

  if((top > 4) || (!lua_isfunction(L, 4))){
    flags = lluv_opt_flags_ui(L, 4, flags, FLAGS);
  }

  if(err < 0){
    lua_checkstack(L, 3);

    lua_pushvalue(L, 2); lua_pushliteral(L, ":"); lua_pushvalue(L, 3); lua_concat(L, 3);

    if(!lua_isfunction(L, top)){
      return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, lua_tostring(L, -1));
    }

    lluv_error_create(L, LLUV_ERR_UV, err, lua_tostring(L, -1));
    lua_remove(L, -2);
    lua_pushvalue(L, 1);
    lua_insert(L, -2);
    lluv_loop_defer_call(L, lluv_loop_by_handle(&handle->handle), 2);
    lua_settop(L, 1);
    return 1;
  }

  err = uv_tcp_bind(LLUV_H(handle, uv_tcp_t), (struct sockaddr *)&sa, flags);
  if(err < 0){
    lua_checkstack(L, 3);

    lua_pushvalue(L, 2); lua_pushliteral(L, ":"); lua_pushvalue(L, 3); lua_concat(L, 3);

    if(!lua_isfunction(L, top)){
      return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, lua_tostring(L, -1));
    }

    lluv_error_create(L, LLUV_ERR_UV, err, lua_tostring(L, -1));
    lua_remove(L, -2);
    lua_pushvalue(L, 1);
    lua_insert(L, -2);
    lluv_loop_defer_call(L, lluv_loop_by_handle(&handle->handle), 2);
    lua_settop(L, 1);
    return 1;
  }

  if(lua_isfunction(L, top)){
    lua_pushvalue(L, 1);
    lua_pushnil(L);
    lluv_loop_defer_call(L,
      lluv_loop_by_handle(&handle->handle),
      lluv_push_addr(L, &sa) + 2
    );
  }

  lua_settop(L, 1);
  return 1;
}

static int lluv_tcp_open(lua_State *L){
  lluv_handle_t  *handle = lluv_check_tcp(L, 1, LLUV_FLAG_OPEN);
  uv_os_sock_t sock = lluv_check_os_sock(L, 2);
  int err = uv_tcp_open(LLUV_H(handle, uv_tcp_t), sock);
  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }

  lua_settop(L, 1);
  return 1;
}

static int lluv_tcp_nodelay(lua_State *L){
  lluv_handle_t *handle = lluv_check_tcp(L, 1, LLUV_FLAG_OPEN);
  int enable = lua_toboolean(L, 2);
  int err = uv_tcp_nodelay(LLUV_H(handle, uv_tcp_t), enable);

  lua_settop(L, 1);

  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }
  return 1;
}

static int lluv_tcp_keepalive(lua_State *L){
  lluv_handle_t *handle = lluv_check_tcp(L, 1, LLUV_FLAG_OPEN);
  int enable = lua_toboolean(L, 2);
  unsigned int delay = 0; int err;

  if(enable) delay = (unsigned int)luaL_checkint(L, 3);
  err = uv_tcp_keepalive(LLUV_H(handle, uv_tcp_t), enable, delay);

  lua_settop(L, 1);

  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }
  return 1;
}

static int lluv_tcp_simultaneous_accepts(lua_State *L){
  lluv_handle_t *handle = lluv_check_tcp(L, 1, LLUV_FLAG_OPEN);
  int enable = lua_toboolean(L, 2);
  int err = uv_tcp_simultaneous_accepts(LLUV_H(handle, uv_tcp_t), enable);

  lua_settop(L, 1);

  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }
  return 1;
}

static int lluv_tcp_getsockname(lua_State *L){
  lluv_handle_t *handle = lluv_check_tcp(L, 1, LLUV_FLAG_OPEN);
  struct sockaddr_storage sa; int sa_len = sizeof(sa);
  int err = uv_tcp_getsockname(LLUV_H(handle, uv_tcp_t), (struct sockaddr*)&sa, &sa_len);

  lua_settop(L, 1);
  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }
  return lluv_push_addr(L, &sa);
}

static int lluv_tcp_getpeername(lua_State *L){
  lluv_handle_t *handle = lluv_check_tcp(L, 1, LLUV_FLAG_OPEN);
  struct sockaddr_storage sa; int sa_len = sizeof(sa);
  int err = uv_tcp_getpeername(LLUV_H(handle, uv_tcp_t), (struct sockaddr*)&sa, &sa_len);
  lua_settop(L, 1);
  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }
  return lluv_push_addr(L, &sa);
}

static const struct luaL_Reg lluv_tcp_methods[] = {
  { "open",                 lluv_tcp_open                 },
  { "bind",                 lluv_tcp_bind                 },
  { "connect",              lluv_tcp_connect              },
  { "nodelay",              lluv_tcp_nodelay              },
  { "keepalive",            lluv_tcp_keepalive            },
  { "simultaneous_accepts", lluv_tcp_simultaneous_accepts },
  { "getsockname",          lluv_tcp_getsockname          },
  { "getpeername",          lluv_tcp_getpeername          },

  {NULL,NULL}
};

static const lluv_uv_const_t lluv_tcp_constants[] = {
  { UV_TCP_IPV6ONLY,   "TCP_IPV6ONLY"   },

#if LLUV_UV_VER_GE(1,7,0)
  {AF_UNSPEC,          "AF_UNSPEC"      },
  {AF_INET,            "AF_INET"        },
  {AF_INET6,           "AF_INET6"       },
#endif

  { 0, NULL }
};

#define LLUV_FUNCTIONS(F)         \
  {"tcp", lluv_tcp_create_##F},   \

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

LLUV_INTERNAL void lluv_tcp_initlib(lua_State *L, int nup, int safe){
  lutil_pushnvalues(L, nup);
  if(!lutil_createmetap(L, LLUV_TCP, lluv_tcp_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);

  luaL_setfuncs(L, lluv_functions[safe], nup);
  lluv_register_constants(L, lluv_tcp_constants);
}

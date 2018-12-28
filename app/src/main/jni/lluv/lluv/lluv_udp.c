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
#include "lluv_udp.h"
#include "lluv_loop.h"
#include "lluv_error.h"
#include "lluv_req.h"
#include "lluv_stream.h"
#include <assert.h>

#define LLUV_UDP_NAME LLUV_PREFIX" udp"
static const char *LLUV_UDP = LLUV_UDP_NAME;

LLUV_INTERNAL int lluv_udp_index(lua_State *L){
  return lluv__index(L, LLUV_UDP, lluv_handle_index);
}

LLUV_IMPL_SAFE(lluv_udp_create){
  lluv_loop_t   *loop   = lluv_opt_loop(L, 1, LLUV_FLAG_OPEN);
  lluv_handle_t *handle;
  int err;

#if LLUV_UV_VER_GE(1,7,0)
  unsigned int flags = lluv_opt_af_flags(L, loop ? 2 : 1, AF_UNSPEC);
#endif

  if(!loop) loop = lluv_default_loop(L);

  handle = lluv_handle_create(L, UV_UDP, safe_flag | INHERITE_FLAGS(loop));

#if LLUV_UV_VER_GE(1,7,0)
  err = uv_udp_init_ex(loop->handle, LLUV_H(handle, uv_udp_t), flags);
#else
  err = uv_udp_init(loop->handle, LLUV_H(handle, uv_udp_t));
#endif

  if(err < 0){
    lluv_handle_cleanup(L, handle, -1);
    return lluv_fail(L, safe_flag | loop->flags, LLUV_ERR_UV, (uv_errno_t)err, NULL);
  }
  return 1;
}

static lluv_handle_t* lluv_check_udp(lua_State *L, int idx, lluv_flags_t flags){
  lluv_handle_t *handle = lluv_check_handle(L, idx, flags);
  luaL_argcheck (L, LLUV_H(handle, uv_handle_t)->type == UV_UDP, idx, LLUV_UDP_NAME" expected");

  return handle;
}

static int lluv_udp_open(lua_State *L){
  lluv_handle_t  *handle = lluv_check_udp(L, 1, LLUV_FLAG_OPEN);
  uv_os_sock_t sock = lluv_check_os_sock(L, 2);
  int err = uv_udp_open(LLUV_H(handle, uv_udp_t), sock);
  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }

  lua_settop(L, 1);
  return 1;
}

static int lluv_udp_bind(lua_State *L){
  static const lluv_uv_const_t FLAGS[] = {
    { UV_UDP_IPV6ONLY ,   "ipv6only"   },
    { UV_UDP_REUSEADDR,   "reuseaddr"  },

    { 0, NULL }
  };

  lluv_handle_t  *handle = lluv_check_udp(L, 1, LLUV_FLAG_OPEN);
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

  err = uv_udp_bind(LLUV_H(handle, uv_udp_t), (struct sockaddr *)&sa, flags);
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

//{ Send

static int lluv_udp_try_send(lua_State *L){
  lluv_handle_t *handle = lluv_check_udp(L, 1, LLUV_FLAG_OPEN);
  struct sockaddr_storage sa; int err = lluv_check_addr(L, 2, &sa);
  size_t len; const char *str = luaL_checklstring(L, 4, &len);
  uv_buf_t buf = lluv_buf_init((char*)str, len);

  if(err < 0){
    lua_settop(L, 3);
    lua_pushliteral(L, ":");lua_insert(L, -2);lua_concat(L, 3);
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, lua_tostring(L, -1));
  }

  lluv_check_none(L, 3);

  err = uv_udp_try_send(LLUV_H(handle, uv_udp_t), &buf, 1, (struct sockaddr*)&sa);
  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }

  lua_pushinteger(L, err);
  return 1;
}

static void lluv_on_udp_send_cb(uv_udp_send_t* arg, int status){
  lluv_on_stream_req_cb((uv_req_t*)arg, status);
}

static int lluv_udp_send_(lua_State *L, lluv_handle_t *handle, struct sockaddr *sa, uv_buf_t *buf, size_t n){
  int err; lluv_req_t *req;

  if(lua_gettop(L) == 6){
    int ctx;
    lluv_check_callable(L, -2);
    ctx = luaL_ref(L, LLUV_LUA_REGISTRY);
    req = lluv_req_new(L, UV_UDP_SEND, handle);
    lluv_req_ref(L, req); /* string/table */
    req->ctx = ctx;
  }
  else{
    if(lua_gettop(L) == 4)
      lua_settop(L, 5);
    else
      lluv_check_args_with_cb(L, 5);

    req = lluv_req_new(L, UV_UDP_SEND, handle);
    lluv_req_ref(L, req); /* string/table */
  }

  err = uv_udp_send(LLUV_R(req, udp_send), LLUV_H(handle, uv_udp_t), buf, n, sa, lluv_on_udp_send_cb);

  return lluv_return_req(L, handle, req, err);
}

static int lluv_udp_send_t(lua_State *L, lluv_handle_t  *handle, struct sockaddr *sa){
  int i, n = lua_rawlen(L, 4);
  uv_buf_t *buf;

  assert(lua_type(L, 4) == LUA_TTABLE);

  luaL_argcheck(L, n > 0, 4, "Empty array not supported");

  buf = (uv_buf_t*)lluv_alloca(sizeof(uv_buf_t) * n);
  if(!buf){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, ENOMEM, NULL); 
  }

  for(i = 0; i < n; ++i){
    size_t len; const char *str;
    lua_rawgeti(L, 4, i + 1);
    str = luaL_checklstring(L, -1, &len);
    buf[i] = lluv_buf_init((char*)str, len);
    lua_pop(L, 1);
  }

  return lluv_udp_send_(L, handle, sa, buf, n);
}

static int lluv_udp_send(lua_State *L){
  lluv_handle_t  *handle = lluv_check_udp(L, 1, LLUV_FLAG_OPEN);
  struct sockaddr_storage sa; int err = lluv_check_addr(L, 2, &sa);

  if(err < 0){
    int top = lua_gettop(L);
    if(top > 4) lua_settop(L, top = 5);

    if(lua_isfunction(L, top)){
      lua_pushvalue(L, 1); /*self*/
      /*host:port*/
      lua_pushvalue(L, 2); lua_pushliteral(L, ":"); lua_pushvalue(L, 3); lua_concat(L, 3);
      lluv_error_create(L, LLUV_ERR_UV, err, lua_tostring(L, -1));
      lua_remove(L, -2);
      lluv_loop_defer_call(L, lluv_loop_by_handle(&handle->handle), 2);
      lua_settop(L, 1);
      return 1;
    }
  
    lua_settop(L, 3);
    lua_pushliteral(L, ":");lua_insert(L, -2);lua_concat(L, 3);
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, lua_tostring(L, -1));
  }

  if(lua_type(L, 4) == LUA_TTABLE){
    return lluv_udp_send_t(L, handle, (struct sockaddr*)&sa);
  }
  else{
    size_t len; const char *str = luaL_checklstring(L, 4, &len);
    uv_buf_t buf = lluv_buf_init((char*)str, len);
    return lluv_udp_send_(L, handle, (struct sockaddr*)&sa, &buf, 1);
  }
}

//}

//{ Recv

static void lluv_on_udp_recv_cb(uv_udp_t *arg, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags){
  lluv_handle_t *handle = lluv_handle_byptr((uv_handle_t*)arg);
  lua_State *L = LLUV_HCALLBACK_L(handle);

  LLUV_CHECK_LOOP_CB_INVARIANT(L);

  if((nread == 0) && (addr == NULL)){
    /*
    ** The receive callback will be called with 
    ** nread == 0 and addr == NULL when there is 
    ** nothing to read
    */
    lluv_free_buffer((uv_handle_t*)arg, buf);
    return;
  }

  lua_rawgeti(L, LLUV_LUA_REGISTRY, LLUV_READ_CB(handle));
  assert(!lua_isnil(L, -1));

  lluv_handle_pushself(L, handle);

  if(nread >= 0){
    assert(addr);
    lua_pushnil(L);
    lua_pushlstring(L, buf->base, nread);
    lluv_free_buffer((uv_handle_t*)arg, buf);
  }
  else{
    lluv_free_buffer((uv_handle_t*)arg, buf);

    /* The callee is responsible for stopping closing the stream 
     *  when an error happens by calling uv_read_stop() or uv_close().
     *  Trying to read from the stream again is undefined.
     */
    uv_udp_recv_stop(arg);

    luaL_unref(L, LLUV_LUA_REGISTRY, LLUV_READ_CB(handle));
    LLUV_READ_CB(handle) = LUA_NOREF;

    lluv_error_create(L, LLUV_ERR_UV, (uv_errno_t)nread, NULL);
    lua_pushnil(L);

    lluv_handle_unlock(L, handle, LLUV_LOCK_READ);
  }
  lua_pushinteger(L, flags);

  LLUV_HANDLE_CALL_CB(L, handle, 4 + lluv_push_addr(L, (const struct sockaddr_storage*)addr));

  LLUV_CHECK_LOOP_CB_INVARIANT(L);
}

static int lluv_udp_start_recv(lua_State *L){
  lluv_handle_t *handle = lluv_check_udp(L, 1, LLUV_FLAG_OPEN);
  int err;

  lluv_check_args_with_cb(L, 2);
  LLUV_READ_CB(handle) = luaL_ref(L, LLUV_LUA_REGISTRY);

  err = uv_udp_recv_start(LLUV_H(handle, uv_udp_t), lluv_alloc_buffer_cb, lluv_on_udp_recv_cb);

  if(err >= 0) lluv_handle_lock(L, handle, LLUV_LOCK_READ);

  return lluv_return(L, handle, LLUV_READ_CB(handle), err);
}

static int lluv_udp_stop_recv(lua_State *L){
  lluv_handle_t *handle = lluv_check_udp(L, 1, LLUV_FLAG_OPEN);
  int err;

  lluv_check_none(L, 2);

  err = uv_udp_recv_stop(LLUV_H(handle, uv_udp_t));
  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }

  if(LLUV_READ_CB(handle) != LUA_NOREF){
    luaL_unref(L, LLUV_LUA_REGISTRY, LLUV_READ_CB(handle));
    LLUV_READ_CB(handle) = LUA_NOREF;
    lluv_handle_unlock(L, handle, LLUV_LOCK_READ);
  }

  lua_settop(L, 1);
  return 1;
}

//}

static int lluv_udp_getsockname(lua_State *L){
  lluv_handle_t *handle = lluv_check_udp(L, 1, LLUV_FLAG_OPEN);
  struct sockaddr_storage sa; int sa_len = sizeof(sa);
  int err = uv_udp_getsockname(LLUV_H(handle, uv_udp_t), (struct sockaddr*)&sa, &sa_len);

  lua_settop(L, 1);
  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }
  return lluv_push_addr(L, &sa);
}

static int lluv_udp_set_membership(lua_State *L){
  static const lluv_uv_const_t FLAGS[] = {
    { UV_LEAVE_GROUP,   "leave" },
    { UV_JOIN_GROUP,    "join"  },

    { 0, NULL }
  };

  lluv_handle_t  *handle = lluv_check_udp(L, 1, LLUV_FLAG_OPEN);
  const char *multicast_addr = luaL_checkstring(L, 2);
  const char *interface_addr = lua_isnoneornil(L,3)?NULL:luaL_checkstring(L, 3);
  uv_membership membership   = (uv_membership)lluv_opt_named_const(L, 4, UV_JOIN_GROUP, FLAGS);

  int err = uv_udp_set_membership(LLUV_H(handle, uv_udp_t), multicast_addr, interface_addr, membership);
  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }

  lua_settop(L, 1);
  return 1;
}

static int lluv_udp_set_multicast_loop(lua_State *L){
  lluv_handle_t  *handle = lluv_check_udp(L, 1, LLUV_FLAG_OPEN);
  int enable = lua_toboolean(L, 2);

  int err = uv_udp_set_multicast_loop(LLUV_H(handle, uv_udp_t), enable);
  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }

  lua_settop(L, 1);
  return 1;
}

static int lluv_udp_set_multicast_ttl(lua_State *L){
  lluv_handle_t  *handle = lluv_check_udp(L, 1, LLUV_FLAG_OPEN);
  int ttl = luaL_checkint(L, 2);

  int err = uv_udp_set_multicast_ttl(LLUV_H(handle, uv_udp_t), ttl);
  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }

  lua_settop(L, 1);
  return 1;
}

static int lluv_udp_set_multicast_interface(lua_State *L){
  lluv_handle_t  *handle = lluv_check_udp(L, 1, LLUV_FLAG_OPEN);
  const char *interface_addr = luaL_checkstring(L, 2);

  int err = uv_udp_set_multicast_interface(LLUV_H(handle, uv_udp_t), interface_addr);
  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }

  lua_settop(L, 1);
  return 1;
}

static int lluv_udp_set_broadcast(lua_State *L){
  lluv_handle_t  *handle = lluv_check_udp(L, 1, LLUV_FLAG_OPEN);
  int enable = lua_toboolean(L, 2);

  int err = uv_udp_set_broadcast(LLUV_H(handle, uv_udp_t), enable);
  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }

  lua_settop(L, 1);
  return 1;
}

static int lluv_udp_set_ttl(lua_State *L){
  lluv_handle_t  *handle = lluv_check_udp(L, 1, LLUV_FLAG_OPEN);
  int ttl = luaL_checkint(L, 2);

  int err = uv_udp_set_ttl(LLUV_H(handle, uv_udp_t), ttl);
  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }

  lua_settop(L, 1);
  return 1;
}

static int lluv_udp_get_send_queue_size(lua_State *L){
  lluv_handle_t *handle = lluv_check_udp(L, 1, LLUV_FLAG_OPEN);
  size_t queue_size;

#if LLUV_UV_VER_GE(1,19,0)
  queue_size = uv_udp_get_send_queue_size(LLUV_H(handle, uv_udp_t));
#else
  queue_size = LLUV_H(handle, uv_udp_t)->send_queue_size;
#endif

  lutil_pushint64(L, queue_size);
  return 1;
}

static int lluv_udp_get_send_queue_count(lua_State *L){
  lluv_handle_t *handle = lluv_check_udp(L, 1, LLUV_FLAG_OPEN);
  size_t queue_count;

#if LLUV_UV_VER_GE(1,19,0)
  queue_count = uv_udp_get_send_queue_count(LLUV_H(handle, uv_udp_t));
#else
  queue_count = LLUV_H(handle, uv_udp_t)->send_queue_count;
#endif

  lutil_pushint64(L, queue_count);
  return 1;
}

static const struct luaL_Reg lluv_udp_methods[] = {
  { "open",                     lluv_udp_open                    },
  { "bind",                     lluv_udp_bind                    },
  { "try_send",                 lluv_udp_try_send                },
  { "send",                     lluv_udp_send                    },
  { "getsockname",              lluv_udp_getsockname             },
  { "start_recv",               lluv_udp_start_recv              },
  { "stop_recv",                lluv_udp_stop_recv               },
  { "set_membership",           lluv_udp_set_membership          },
  { "set_multicast_loop",       lluv_udp_set_multicast_loop      },
  { "set_multicast_ttl",        lluv_udp_set_multicast_ttl       },
  { "set_multicast_interface",  lluv_udp_set_multicast_interface },
  { "set_broadcast",            lluv_udp_set_broadcast           },
  { "set_ttl",                  lluv_udp_set_ttl                 },
  { "get_send_queue_size",      lluv_udp_get_send_queue_size     },
  { "get_send_queue_count",     lluv_udp_get_send_queue_count    },

  {NULL,NULL}
};

static const lluv_uv_const_t lluv_udp_constants[] = {
  { UV_UDP_IPV6ONLY,   "UDP_IPV6ONLY"   },
  { UV_UDP_PARTIAL,    "UDP_PARTIAL"    },
  { UV_UDP_REUSEADDR,  "UDP_REUSEADDR"  },
  { UV_LEAVE_GROUP ,   "LEAVE_GROUP "   },
  { UV_JOIN_GROUP,     "JOIN_GROUP"     },

  { 0, NULL }
};

#define LLUV_FUNCTIONS(F)       \
  {"udp", lluv_udp_create_##F}, \

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

LLUV_INTERNAL void lluv_udp_initlib(lua_State *L, int nup, int safe){
  lutil_pushnvalues(L, nup);
  if(!lutil_createmetap(L, LLUV_UDP, lluv_udp_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);

  luaL_setfuncs(L, lluv_functions[safe], nup);
  lluv_register_constants(L, lluv_udp_constants);
}

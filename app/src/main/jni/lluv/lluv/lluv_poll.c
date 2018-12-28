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
#include "lluv_poll.h"
#include "lluv_loop.h"
#include "lluv_error.h"
#include <assert.h>

#define LLUV_POLL_NAME LLUV_PREFIX" Poll"
static const char *LLUV_POLL = LLUV_POLL_NAME;

LLUV_INTERNAL int lluv_poll_index(lua_State *L){
  return lluv__index(L, LLUV_POLL, lluv_handle_index);
}

LLUV_IMPL_SAFE(lluv_poll_create){
  lluv_loop_t *loop  = lluv_opt_loop(L, 1, LLUV_FLAG_OPEN);
  int fd = luaL_checkint(L, loop ? 2 : 1);
  lluv_handle_t *handle; int err;

  if(!loop) loop = lluv_default_loop(L);
  handle = lluv_handle_create(L, UV_POLL, safe_flag | INHERITE_FLAGS(loop));

  err = uv_poll_init(loop->handle, LLUV_H(handle, uv_poll_t), fd);
  if(err < 0){
    lluv_handle_cleanup(L, handle, -1);
    return lluv_fail(L, safe_flag | loop->flags, LLUV_ERR_UV, (uv_errno_t)err, NULL);
  }
  return 1;
}

LLUV_IMPL_SAFE(lluv_poll_create_socket){
  lluv_loop_t *loop  = lluv_opt_loop(L, 1, LLUV_FLAG_OPEN);
  uv_os_sock_t socket = lluv_check_os_sock(L, loop ? 2 : 1);
  lluv_handle_t *handle; int err;

  if(!loop) loop = lluv_default_loop(L);
  handle = lluv_handle_create(L, UV_POLL, safe_flag | INHERITE_FLAGS(loop));

  err = uv_poll_init_socket(loop->handle, LLUV_H(handle, uv_poll_t), socket);
  if(err < 0){
    lluv_handle_cleanup(L, handle, -1);
    return lluv_fail(L, safe_flag | loop->flags, LLUV_ERR_UV, (uv_errno_t)err, NULL);
  }
  return 1;
}

static lluv_handle_t* lluv_check_poll(lua_State *L, int idx, lluv_flags_t flags){
  lluv_handle_t *handle = lluv_check_handle(L, idx, flags);
  luaL_argcheck (L, LLUV_H(handle,uv_handle_t)->type == UV_POLL, idx, LLUV_POLL_NAME" expected");

  return handle;
}

static void lluv_on_poll_start(uv_poll_t *arg, int status, int events){
  lluv_handle_t *handle = lluv_handle_byptr((uv_handle_t*)arg);
  lua_State *L = LLUV_HCALLBACK_L(handle);

  LLUV_CHECK_LOOP_CB_INVARIANT(L);

  lua_rawgeti(L, LLUV_LUA_REGISTRY, LLUV_START_CB(handle));
  assert(!lua_isnil(L, -1)); /* is callble */

  lluv_handle_pushself(L, handle);
  lluv_push_status(L, status);
  lua_pushinteger(L, events);

  LLUV_HANDLE_CALL_CB(L, handle, 3);

  LLUV_CHECK_LOOP_CB_INVARIANT(L);
}

static int lluv_poll_start(lua_State *L){
  static const lluv_uv_const_t FLAGS[] = {
    { UV_READABLE,   "readable"   },
    { UV_WRITABLE,   "writable"   },
#if LLUV_UV_VER_GE(1,9,0)
    { UV_DISCONNECT, "disconnect" },
#endif

    { 0, NULL }
  };

  lluv_handle_t *handle = lluv_check_poll(L, 1, LLUV_FLAG_OPEN);
  int events = UV_READABLE;
  int err;

  if(!lua_isfunction(L, 2))
    events = lluv_opt_flags_ui(L, 2, UV_READABLE, FLAGS);

  lluv_check_args_with_cb(L, 3);
  LLUV_START_CB(handle) = luaL_ref(L, LLUV_LUA_REGISTRY);

  err = uv_poll_start(LLUV_H(handle, uv_poll_t), events, lluv_on_poll_start);

  if(err >= 0) lluv_handle_lock(L, handle, LLUV_LOCK_START);

  return lluv_return(L, handle, LLUV_START_CB(handle), err);
}

static int lluv_poll_stop(lua_State *L){
  lluv_handle_t *handle = lluv_check_poll(L, 1, LLUV_FLAG_OPEN);
  int err = uv_poll_stop(LLUV_H(handle, uv_poll_t));
  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }

  lluv_handle_unlock(L, handle, LLUV_LOCK_START);

  lua_settop(L, 1);
  return 1;
}

static const struct luaL_Reg lluv_poll_methods[] = {
  { "start",      lluv_poll_start      },
  { "stop",       lluv_poll_stop       },

  {NULL,NULL}
};

static const lluv_uv_const_t lluv_poll_constants[] = {
  { UV_READABLE,   "READABLE"   },
  { UV_WRITABLE,   "WRITABLE"   },
#if LLUV_UV_VER_GE(1,9,0)
  { UV_DISCONNECT, "DISCONNECT" },
#endif

  { 0, NULL }
};

#define LLUV_POLL_FUNCTIONS(F)                  \
  {"poll", lluv_poll_create_##F},               \
  {"poll_socket", lluv_poll_create_socket_##F}, \

static const struct luaL_Reg lluv_poll_functions[][3] = {
  {
    LLUV_POLL_FUNCTIONS(unsafe)

    {NULL,NULL}
  },
  {
    LLUV_POLL_FUNCTIONS(safe)

    {NULL,NULL}
  },
};

LLUV_INTERNAL void lluv_poll_initlib(lua_State *L, int nup, int safe){
  assert((safe == 0) || (safe == 1));

  lutil_pushnvalues(L, nup);
  if(!lutil_createmetap(L, LLUV_POLL, lluv_poll_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);

  luaL_setfuncs(L, lluv_poll_functions[safe], nup);
  lluv_register_constants(L, lluv_poll_constants);
}

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
#include "lluv_fs_poll.h"
#include "lluv_loop.h"
#include "lluv_error.h"
#include <assert.h>

#define LLUV_FS_POLL_NAME LLUV_PREFIX" FS Poll"
static const char *LLUV_FS_POLL = LLUV_FS_POLL_NAME;

LLUV_INTERNAL int lluv_fs_poll_index(lua_State *L){
  return lluv__index(L, LLUV_FS_POLL, lluv_handle_index);
}

LLUV_IMPL_SAFE(lluv_fs_poll_create){
  lluv_loop_t   *loop   = lluv_opt_loop_ex(L, 1, LLUV_FLAG_OPEN);
  lluv_handle_t *handle = lluv_handle_create(L, UV_FS_POLL, safe_flag | INHERITE_FLAGS(loop));
  int err = uv_fs_poll_init(loop->handle, LLUV_H(handle, uv_fs_poll_t));
  if(err < 0){
    lluv_handle_cleanup(L, handle, -1);
    return lluv_fail(L, safe_flag | loop->flags, LLUV_ERR_UV, (uv_errno_t)err, NULL);
  }
  return 1;
}

static lluv_handle_t* lluv_check_fs_poll(lua_State *L, int idx, lluv_flags_t flags){
  lluv_handle_t *handle = lluv_check_handle(L, idx, flags);
  luaL_argcheck (L, LLUV_H(handle, uv_handle_t)->type == UV_FS_POLL, idx, LLUV_FS_POLL_NAME" expected");

  return handle;
}

static void lluv_on_fs_poll_start(uv_fs_poll_t *arg, int status, const uv_stat_t* prev, const uv_stat_t* curr){
  lluv_handle_t *handle = lluv_handle_byptr((uv_handle_t*)arg);
  lua_State *L = LLUV_HCALLBACK_L(handle);

  LLUV_CHECK_LOOP_CB_INVARIANT(L);

  lua_rawgeti(L, LLUV_LUA_REGISTRY, LLUV_START_CB(handle));
  assert(!lua_isnil(L, -1)); /* is callble */

  lluv_handle_pushself(L, handle);
  lluv_push_status(L, status);

  if(prev)lluv_push_stat(L, prev); else lua_pushnil(L);
  if(curr)lluv_push_stat(L, curr); else lua_pushnil(L);

  LLUV_HANDLE_CALL_CB(L, handle, 4);

  LLUV_CHECK_LOOP_CB_INVARIANT(L);
}

static int lluv_fs_poll_start(lua_State *L){
  lluv_handle_t *handle = lluv_check_fs_poll(L, 1, LLUV_FLAG_OPEN);
  const char *path   = luaL_checkstring(L, 2);
  /* For maximum portability, use multi-second intervals.                   */
  /* Sub-second intervals will not detect all changes on many file systems. */
  unsigned int interval = 5000;
  int err;

  if(lua_gettop(L) > 3) interval = luaL_optint(L, 3, interval);

  lluv_check_args_with_cb(L, 4);
  LLUV_START_CB(handle) = luaL_ref(L, LLUV_LUA_REGISTRY);

  err = uv_fs_poll_start(LLUV_H(handle, uv_fs_poll_t), lluv_on_fs_poll_start, path, interval);

  if(err >= 0) lluv_handle_lock(L, handle, LLUV_LOCK_START);

  return lluv_return(L, handle, LLUV_START_CB(handle), err);
}

static int lluv_fs_poll_stop(lua_State *L){
  lluv_handle_t *handle = lluv_check_fs_poll(L, 1, LLUV_FLAG_OPEN);
  int err = uv_fs_poll_stop(LLUV_H(handle, uv_fs_poll_t));
  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }

  lluv_handle_unlock(L, handle, LLUV_LOCK_START);

  lua_settop(L, 1);
  return 1;
}

static int lluv_fs_getpath(lua_State *L){
  lluv_handle_t  *handle = lluv_check_fs_poll(L, 1, LLUV_FLAG_OPEN);
  char buf[255]; size_t len = sizeof(buf);
  int err = uv_fs_poll_getpath(LLUV_H(handle, uv_fs_poll_t), buf, &len);
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
    err = uv_fs_poll_getpath(LLUV_H(handle, uv_fs_poll_t), buf, &len);
    if(err < 0){
      lluv_free(L, buf);
      return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
    }
    lua_pushlstring(L, buf, len);
    lluv_free(L, buf);
    return 1;
  }
}

static const struct luaL_Reg lluv_fs_poll_methods[] = {
  { "start",      lluv_fs_poll_start      },
  { "stop",       lluv_fs_poll_stop       },
  { "getpath",    lluv_fs_getpath          },

  {NULL,NULL}
};

#define LLUV_FS_POLL_FUNCTIONS(F)       \
  {"fs_poll", lluv_fs_poll_create_##F}, \

static const struct luaL_Reg lluv_fs_poll_functions[][2] = {
  {
    LLUV_FS_POLL_FUNCTIONS(unsafe)

    {NULL,NULL}
  },
  {
    LLUV_FS_POLL_FUNCTIONS(safe)

    {NULL,NULL}
  },
};

static const lluv_uv_const_t lluv_fs_poll_constants[] = {

  { 0, NULL }
};

LLUV_INTERNAL void lluv_fs_poll_initlib(lua_State *L, int nup, int safe){
  lutil_pushnvalues(L, nup);
  if(!lutil_createmetap(L, LLUV_FS_POLL, lluv_fs_poll_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);

  luaL_setfuncs(L, lluv_fs_poll_functions[safe], nup);
  lluv_register_constants(L, lluv_fs_poll_constants);
}

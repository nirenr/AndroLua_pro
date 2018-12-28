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
#include "lluv_timer.h"
#include "lluv_loop.h"
#include "lluv_error.h"
#include <assert.h>

#define LLUV_TIMER_NAME LLUV_PREFIX" Timer"
static const char *LLUV_TIMER = LLUV_TIMER_NAME;

LLUV_INTERNAL int lluv_timer_index(lua_State *L){
  return lluv__index(L, LLUV_TIMER, lluv_handle_index);
}

LLUV_IMPL_SAFE(lluv_timer_create){
  lluv_loop_t   *loop   = lluv_opt_loop_ex(L, 1, LLUV_FLAG_OPEN);
  lluv_handle_t *handle = lluv_handle_create(L, UV_TIMER, safe_flag | INHERITE_FLAGS(loop));
  int err = uv_timer_init(loop->handle, LLUV_H(handle, uv_timer_t));
  if(err < 0){
    lluv_handle_cleanup(L, handle, -1);
    return lluv_fail(L, safe_flag | loop->flags, LLUV_ERR_UV, (uv_errno_t)err, NULL);
  }
  return 1;
}

static lluv_handle_t* lluv_check_timer(lua_State *L, int idx, lluv_flags_t flags){
  lluv_handle_t *handle = lluv_check_handle(L, idx, flags);
  luaL_argcheck (L, LLUV_H(handle, uv_handle_t)->type == UV_TIMER, idx, LLUV_TIMER_NAME" expected");

  return handle;
}

static void lluv_on_timer_start(uv_timer_t *arg){
  uv_handle_t *h = (uv_handle_t*)arg;
  if(!uv_is_active(h)){
    lluv_handle_t *handle = lluv_handle_byptr(h);
    lua_State *L = LLUV_HCALLBACK_L(handle);
    lluv_handle_unlock(L, handle, LLUV_LOCK_START);
  }
  lluv_on_handle_start(h);
}

static int lluv_timer_start(lua_State *L){
  lluv_handle_t *handle = lluv_check_timer(L, 1, LLUV_FLAG_OPEN);
  uint64_t timeout, repeat;
  int err;

  lluv_check_args_with_cb(L, 4);
  LLUV_START_CB(handle) = luaL_ref(L, LLUV_LUA_REGISTRY);

  if(lua_gettop(L) > 1){
    timeout = lutil_checkint64(L, 2);
    if(lua_gettop(L) > 2)
      repeat = lutil_checkint64(L, 3);
    else
      repeat = 0;
  }
  else{
    timeout = 0;
    repeat  = 0;
  }

  err = uv_timer_start(LLUV_H(handle, uv_timer_t), lluv_on_timer_start, timeout, repeat);

  if(err >= 0) lluv_handle_lock(L, handle, LLUV_LOCK_START);

  return lluv_return(L, handle, LLUV_START_CB(handle), err);
}

static int lluv_timer_stop(lua_State *L){
  lluv_handle_t *handle = lluv_check_timer(L, 1, LLUV_FLAG_OPEN);
  int err = uv_timer_stop(LLUV_H(handle, uv_timer_t));
  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }

  lluv_handle_lock(L, handle, LLUV_LOCK_START);

  lua_settop(L, 1);
  return 1;
}

static int lluv_timer_again(lua_State *L){
  lluv_handle_t *handle = lluv_check_timer(L, 1, LLUV_FLAG_OPEN);
  int err;
  if(lua_isnumber(L, 2)){
    uint64_t repeat = lutil_optint64(L, 2, 0);
    uv_timer_set_repeat(LLUV_H(handle, uv_timer_t), repeat);
  }
  err = uv_timer_again(LLUV_H(handle, uv_timer_t));
  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }

  lluv_handle_lock(L, handle, LLUV_LOCK_START);

  lua_settop(L, 1);
  return 1;
}

static int lluv_timer_set_repeat(lua_State *L){
  lluv_handle_t *handle = lluv_check_timer(L, 1, LLUV_FLAG_OPEN);
  uint64_t repeat = lutil_optint64(L, 2, 0);
  uv_timer_set_repeat(LLUV_H(handle, uv_timer_t), repeat);
  lua_settop(L, 1);
  return 1;
}

static int lluv_timer_get_repeat(lua_State *L){
  lluv_handle_t *handle = lluv_check_timer(L, 1, LLUV_FLAG_OPEN);
  uint64_t repeat = uv_timer_get_repeat(LLUV_H(handle, uv_timer_t));
  lutil_pushint64(L, repeat);
  return 1;
}

static const struct luaL_Reg lluv_timer_methods[] = {
  { "start",      lluv_timer_start      },
  { "stop",       lluv_timer_stop       },
  { "again",      lluv_timer_again      },
  { "set_repeat", lluv_timer_set_repeat },
  { "get_repeat", lluv_timer_get_repeat },

  {NULL,NULL}
};

#define LLUV_FUNCTIONS(F)           \
  {"timer", lluv_timer_create_##F}, \

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


LLUV_INTERNAL void lluv_timer_initlib(lua_State *L, int nup, int safe){
  lutil_pushnvalues(L, nup);
  if(!lutil_createmetap(L, LLUV_TIMER, lluv_timer_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);

  luaL_setfuncs(L, lluv_functions[safe], nup);
}

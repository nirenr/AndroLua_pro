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
#include "lluv_check.h"
#include "lluv_loop.h"
#include "lluv_error.h"
#include <assert.h>

#define LLUV_CHECK_NAME LLUV_PREFIX" Check"
static const char *LLUV_CHECK = LLUV_CHECK_NAME;

LLUV_INTERNAL int lluv_check_index(lua_State *L){
  return lluv__index(L, LLUV_CHECK, lluv_handle_index);
}

LLUV_IMPL_SAFE(lluv_check_create){
  lluv_loop_t   *loop   = lluv_opt_loop_ex(L, 1, LLUV_FLAG_OPEN);
  lluv_handle_t *handle = lluv_handle_create(L, UV_CHECK, safe_flag | INHERITE_FLAGS(loop));
  int err = uv_check_init(loop->handle, LLUV_H(handle, uv_check_t));
  if(err < 0){
    lluv_handle_cleanup(L, handle, -1);
    return lluv_fail(L, safe_flag | loop->flags, LLUV_ERR_UV, (uv_errno_t)err, NULL);
  }
  return 1;
}

static lluv_handle_t* lluv_check_check(lua_State *L, int idx, lluv_flags_t flags){
  lluv_handle_t *handle = lluv_check_handle(L, idx, flags);
  luaL_argcheck (L, LLUV_H(handle, uv_handle_t)->type == UV_CHECK, idx, LLUV_CHECK_NAME" expected");

  return handle;
}

static void lluv_on_check_start(uv_check_t *arg){
  lluv_on_handle_start((uv_handle_t*)arg);
}

static int lluv_check_start(lua_State *L){
  lluv_handle_t *handle = lluv_check_check(L, 1, LLUV_FLAG_OPEN);
  int err;

  lluv_check_args_with_cb(L, 2);
  LLUV_START_CB(handle) = luaL_ref(L, LLUV_LUA_REGISTRY);

  err = uv_check_start(LLUV_H(handle, uv_check_t), lluv_on_check_start);

  if(err >= 0) lluv_handle_lock(L, handle, LLUV_LOCK_START);

  return lluv_return(L, handle, LLUV_START_CB(handle), err);
}

static int lluv_check_stop(lua_State *L){
  lluv_handle_t *handle = lluv_check_check(L, 1, LLUV_FLAG_OPEN);
  int err = uv_check_stop(LLUV_H(handle, uv_check_t));
  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }

  lluv_handle_unlock(L, handle, LLUV_LOCK_START);

  lua_settop(L, 1);
  return 1;
}

static const struct luaL_Reg lluv_check_methods[] = {
  { "start",      lluv_check_start      },
  { "stop",       lluv_check_stop       },

  {NULL,NULL}
};

#define LLUV_FUNCTIONS(F)           \
  {"check", lluv_check_create_##F}, \

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
LLUV_INTERNAL void lluv_check_initlib(lua_State *L, int nup, int safe){
  lutil_pushnvalues(L, nup);
  if(!lutil_createmetap(L, LLUV_CHECK, lluv_check_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);

  luaL_setfuncs(L, lluv_functions[safe], nup);
}

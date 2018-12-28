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
#include "lluv_tty.h"
#include "lluv_loop.h"
#include "lluv_error.h"
#include <assert.h>

#define LLUV_TTY_NAME LLUV_PREFIX" tty"
static const char *LLUV_TTY = LLUV_TTY_NAME;

LLUV_INTERNAL int lluv_tty_index(lua_State *L){
  return lluv__index(L, LLUV_TTY, lluv_stream_index);
}

LLUV_IMPL_SAFE(lluv_tty_create){
  lluv_loop_t *loop = lluv_opt_loop(L, 1, LLUV_FLAG_OPEN);
  uv_file fd        = (uv_file)lutil_checkint64(L, loop ? 2 : 1);
  int readable      = lua_toboolean(L, loop ? 3 : 2);
  lluv_handle_t *handle;
  int err;

  if(!loop) loop = lluv_default_loop(L);

  handle = lluv_stream_create(L, UV_TTY, safe_flag | INHERITE_FLAGS(loop));
  err = uv_tty_init(loop->handle, LLUV_H(handle, uv_tty_t), fd, readable);
  if(err < 0){
    lluv_handle_cleanup(L, handle, -1);
    return lluv_fail(L, safe_flag | loop->flags, LLUV_ERR_UV, (uv_errno_t)err, NULL);
  }
  return 1;
}

static lluv_handle_t* lluv_check_tty(lua_State *L, int idx, lluv_flags_t flags){
  lluv_handle_t *handle = lluv_check_stream(L, idx, flags);
  luaL_argcheck (L, LLUV_H(handle, uv_handle_t)->type == UV_TTY, idx, LLUV_TTY_NAME" expected");

  return handle;
}

static int lluv_tty_set_mode(lua_State *L){
  lluv_handle_t *handle = lluv_check_tty(L, 1, LLUV_FLAG_OPEN);
  int mode = luaL_checkint(L, 2);
  int err  = uv_tty_set_mode(LLUV_H(handle, uv_tty_t), mode);
  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, (uv_errno_t)err, NULL);
  }
  lua_settop(L, 1);
  return 1;
}

LLUV_IMPL_SAFE(lluv_tty_reset_mode){
  lluv_loop_t *loop = lluv_opt_loop_ex(L, 1, 0);
  int err = uv_tty_reset_mode();

  if(err < 0){
    return lluv_fail(L, safe_flag | loop->flags, LLUV_ERR_UV, (uv_errno_t)err, NULL);
  }
  
  lua_pushboolean(L, 1);
  return 1;
}

static int lluv_tty_get_winsize(lua_State *L){
  lluv_handle_t *handle = lluv_check_tty(L, 1, LLUV_FLAG_OPEN);
  int width, height;
  int err  = uv_tty_get_winsize(LLUV_H(handle, uv_tty_t), &width, &height);
  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, (uv_errno_t)err, NULL);
  }

  lua_settop(L, 1);
  lua_pushinteger(L, width);
  lua_pushinteger(L, height);
  return 2;
}

static const struct luaL_Reg lluv_tty_methods[] = {
  { "set_mode",       lluv_tty_set_mode     },
  { "get_winsize",    lluv_tty_get_winsize  },

  {NULL,NULL}
};

#define LLUV_FUNCTIONS(F)                      \
  {"tty",            lluv_tty_create_##F},     \
  {"tty_reset_mode", lluv_tty_reset_mode_##F}, \

static const struct luaL_Reg lluv_functions[][3] = {
  {
    LLUV_FUNCTIONS(unsafe)

    {NULL,NULL}
  },
  {
    LLUV_FUNCTIONS(safe)

    {NULL,NULL}
  },
};

LLUV_INTERNAL void lluv_tty_initlib(lua_State *L, int nup, int safe){
  lutil_pushnvalues(L, nup);
  if(!lutil_createmetap(L, LLUV_TTY, lluv_tty_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);

  luaL_setfuncs(L, lluv_functions[safe], nup);
}

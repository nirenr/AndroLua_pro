/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2016 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lluv library.
******************************************************************************/

#ifndef _LLUV_LOOP_H_
#define _LLUV_LOOP_H_

#include "lluv.h"
#include "lluv_utils.h"
#include "lluv_list.h"

// number of values that push loop.run
#define LLUV_CALLBACK_TOP_SIZE 0

#define LLUV_BUFFER_SIZE 65536

typedef struct lluv_loop_tag{
  uv_loop_t   *handle;/* read only */
  lluv_flags_t flags; /* read only */
  lua_State   *L;
  lluv_list_t  defer;
  int8_t       level;
  size_t       buffer_size;
  char         buffer[LLUV_BUFFER_SIZE];
}lluv_loop_t;

LLUV_INTERNAL void lluv_loop_initlib(lua_State *L, int nup);

LLUV_INTERNAL int lluv_loop_create(lua_State *L, uv_loop_t *loop, lluv_flags_t flags);

LLUV_INTERNAL lluv_loop_t* lluv_check_loop(lua_State *L, int idx, lluv_flags_t flags);

LLUV_INTERNAL lluv_loop_t* lluv_opt_loop(lua_State *L, int idx, lluv_flags_t flags);

LLUV_INTERNAL lluv_loop_t* lluv_opt_loop_ex(lua_State *L, int idx, lluv_flags_t flags);

LLUV_INTERNAL lluv_loop_t* lluv_push_default_loop(lua_State *L);

LLUV_INTERNAL lluv_loop_t* lluv_default_loop(lua_State *L);

LLUV_INTERNAL lluv_loop_t* lluv_ensure_loop_at(lua_State *L, int idx);

LLUV_INTERNAL lluv_loop_t* lluv_loop_byptr(uv_loop_t *h);

LLUV_INTERNAL lluv_loop_t* lluv_loop_by_handle(uv_handle_t* h);

LLUV_INTERNAL void lluv_loop_pushself(lua_State *L, lluv_loop_t *loop);

LLUV_INTERNAL void lluv_loop_defer_call(lua_State *L, lluv_loop_t *loop, int nargs);

LLUV_INTERNAL int lluv_loop_defer_proceed(lua_State *L, lluv_loop_t *loop);

#define LLUV_CHECK_LOOP_CB_INVARIANT(L) \
  assert("Some one use invalid callback handler" && (lua_gettop(L) == LLUV_CALLBACK_TOP_SIZE)); \
  assert("Invalid number of upvalues" && (lua_isnone(L, LLUV_NONE_MARK_INDEX)));                \
  assert("Invalid LLUV registry" && (lua_type(L, LLUV_LUA_REGISTRY) == LUA_TTABLE));            \
  assert("Invalid loop" && lluv_check_loop(L, LLUV_LOOP_INDEX, 0));

#define LLUV_HANDLE_CALL_CB(L, H, A)                                            \
  {                                                                             \
    int err = lluv_lua_call((L), (A), 0);                                       \
    if(!err)lluv_loop_defer_proceed((L), lluv_loop_by_handle(&(H)->handle));    \
  }                                                                             \

#define LLUV_LOOP_CALL_CB(L, LOOP, A)                                           \
  {                                                                             \
    int err = lluv_lua_call((L), (A), 0);                                       \
    if(!err)lluv_loop_defer_proceed((L), LOOP);                                 \
  }                                                                             \

#endif

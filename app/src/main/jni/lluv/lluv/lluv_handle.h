/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2016 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lluv library.
******************************************************************************/

#ifndef _LLUV_HANDLE_H_
#define _LLUV_HANDLE_H_

#define LLUV_CLOSE_CB(H)      H->callbacks[0]
#define LLUV_START_CB(H)      H->callbacks[1]
#define LLUV_READ_CB(H)       H->callbacks[1]
#define LLUV_EXIT_CB(H)       H->callbacks[1]
#define LLUV_CONNECTION_CB(H) H->callbacks[2]
#define LLUV_MAX_HANDLE_CB    3


#include "lluv.h"
#include "lluv_utils.h"

typedef struct lluv_handle_tag{
  int          self;
  lluv_flags_t lock;
  int          lock_counter;
  lua_State   *L;
  lluv_flags_t flags;
  int          callbacks[LLUV_MAX_HANDLE_CB];
  uv_handle_t  handle;
} lluv_handle_t;

//! @todo make debug verions with check cast with checking uv_handle_type
#define LLUV_H(H, T) ((T*)&H->handle)

#define LLUV_HCALLBACK_L(H) (lluv_loop_by_handle(&H->handle)->L)

LLUV_INTERNAL void lluv_handle_initlib(lua_State *L, int nup, int safe);

LLUV_INTERNAL int lluv_handle_index(lua_State *L);

LLUV_INTERNAL lluv_handle_t* lluv_handle_create(lua_State *L, uv_handle_type type, lluv_flags_t flags);

LLUV_INTERNAL lluv_handle_t* lluv_check_handle(lua_State *L, int idx, lluv_flags_t flags);

LLUV_INTERNAL void lluv_handle_cleanup(lua_State *L, lluv_handle_t *handle, int idx);

/* Convert uv_handle_t* to lluv_handle_t*
 * There no checks so it function has UB if this handle was created not by
 * lluv library.
 */
LLUV_INTERNAL lluv_handle_t* lluv_handle_byptr(uv_handle_t *h);

/* find lluv_handle Lua object associatad with this handle.
 * function returns nil if this handle has no Lua object.
 */
LLUV_INTERNAL int lluv_handle_find(lua_State *L, uv_handle_t *h);

LLUV_INTERNAL int lluv_handle_pushself(lua_State *L, lluv_handle_t *handle);

LLUV_INTERNAL void lluv_on_handle_start(uv_handle_t *arg);

LLUV_INTERNAL void lluv_handle_lock(lua_State *L, lluv_handle_t *handle, lluv_flags_t lock);

LLUV_INTERNAL void lluv_handle_unlock(lua_State *L, lluv_handle_t *handle, lluv_flags_t lock);

#define LLUV_LOCK_CLOSE       LLUV_FLAG_0
#define LLUV_LOCK_START       LLUV_FLAG_1
#define LLUV_LOCK_READ        LLUV_FLAG_1
#define LLUV_LOCK_EXIT        LLUV_FLAG_1
#define LLUV_LOCK_CONNECTION  LLUV_FLAG_2
#define LLUV_LOCK_MANUAL      LLUV_FLAG_3
#define LLUV_LOCK_REQ         LLUV_FLAG_7 /* counter lock */

#endif

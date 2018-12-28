/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2016 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lluv library.
******************************************************************************/

#ifndef _LLUV_TCP_H_
#define _LLUV_TCP_H_

LLUV_INTERNAL void lluv_tcp_initlib(lua_State *L, int nup, int safe);

LLUV_INTERNAL int lluv_tcp_index(lua_State *L);

LLUV_INTERNAL int lluv_tcp_create_safe(lua_State *L);

LLUV_INTERNAL int lluv_tcp_create_unsafe(lua_State *L);

#endif

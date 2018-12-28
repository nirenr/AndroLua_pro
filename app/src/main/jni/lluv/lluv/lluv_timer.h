/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2016 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lluv library.
******************************************************************************/

#ifndef _LLUV_TIMER_H_
#define _LLUV_TIMER_H_

LLUV_INTERNAL void lluv_timer_initlib(lua_State *L, int nup, int safe);

LLUV_INTERNAL int lluv_timer_index(lua_State *L);

#endif

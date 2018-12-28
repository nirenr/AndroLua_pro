/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2016 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lluv library.
******************************************************************************/

#ifndef _LLUV_FS_EVENT_H_
#define _LLUV_FS_EVENT_H_

LLUV_INTERNAL void lluv_fs_event_initlib(lua_State *L, int nup, int safe);

LLUV_INTERNAL int lluv_fs_event_index(lua_State *L);

#endif

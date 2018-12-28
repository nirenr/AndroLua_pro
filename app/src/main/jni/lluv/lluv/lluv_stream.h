/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2016 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lluv library.
******************************************************************************/

#ifndef _LLUV_STREAM_H_
#define _LLUV_STREAM_H_

LLUV_INTERNAL void lluv_stream_initlib(lua_State *L, int nup, int safe);

LLUV_INTERNAL int lluv_stream_index(lua_State *L);

LLUV_INTERNAL lluv_handle_t* lluv_stream_create(lua_State *L, uv_handle_type type, lluv_flags_t flags);

LLUV_INTERNAL lluv_handle_t* lluv_check_stream(lua_State *L, int idx, lluv_flags_t flags);

LLUV_INTERNAL void lluv_on_stream_connect_cb(uv_connect_t* arg, int status);

LLUV_INTERNAL void lluv_on_stream_req_cb(uv_req_t* arg, int status);

#endif

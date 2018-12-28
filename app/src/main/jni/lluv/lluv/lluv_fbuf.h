/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2016 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lluv library.
******************************************************************************/

#ifndef _LLUV_FBUF_H_
#define _LLUV_FBUF_H_

#include "lluv.h"

typedef struct lluv_fixed_buffer_tag{
  size_t  capacity;
  char    data[1];
}lluv_fixed_buffer_t;

LLUV_INTERNAL void lluv_fbuf_initlib(lua_State *L, int nup, int safe);

LLUV_INTERNAL lluv_fixed_buffer_t *lluv_fbuf_alloc(lua_State *L, size_t n);

LLUV_INTERNAL lluv_fixed_buffer_t *lluv_check_fbuf(lua_State *L, int i);

#endif

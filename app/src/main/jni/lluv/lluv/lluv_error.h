/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2016 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lluv library.
******************************************************************************/

#ifndef _LLUV_ERROR_H_
#define _LLUV_ERROR_H_

#include "lluv.h"
#include "lluv_utils.h"

/* error category */
#define LLUV_ERR_LIB 0
#define LLUV_ERR_UV  1

typedef struct lluv_error_tag{
  int        cat;
  uv_errno_t no;
  char       ext[1];
}lluv_error_t;

LLUV_INTERNAL void lluv_error_initlib(lua_State *L, int nup, int safe);

LLUV_INTERNAL int lluv_error_create(lua_State *L, int error_category, uv_errno_t error_no, const char *ext);

LLUV_INTERNAL int lluv_fail(lua_State *L, lluv_flags_t flags, int error_category, uv_errno_t error_no, const char *ext);

#endif



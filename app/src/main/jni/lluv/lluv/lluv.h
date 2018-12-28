/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2016 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lluv library.
******************************************************************************/

#ifndef _LLUV_H_
#define _LLUV_H_

#include <uv.h>
#include <lua.h>
#include "l52util.h"

#define LLUV_PREFIX "Lua-UV"

/*export*/
#ifdef _WIN32
#  define LLUV_EXPORT_API __declspec(dllexport)
#else
#  define LLUV_EXPORT_API LUALIB_API
#endif

#define LLUV_INTERNAL

#endif

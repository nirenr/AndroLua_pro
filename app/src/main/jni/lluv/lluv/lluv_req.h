/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2016 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lluv library.
******************************************************************************/

#ifndef _LLUV_REQ_H_
#define _LLUV_REQ_H_

#include "lluv.h"
#include "lluv_handle.h"

typedef struct lluv_req_tag{
  lluv_handle_t *handle;
  int           cb;
  int           arg;
  int           ctx;
  uv_req_t      req;
} lluv_req_t;

#define LLUV_R(H, T) ((uv_##T##_t*)&H->req)

LLUV_INTERNAL lluv_req_t* lluv_req_new(lua_State *L, uv_req_type type, lluv_handle_t *h);

LLUV_INTERNAL void lluv_req_free(lua_State *L, lluv_req_t *req);

LLUV_INTERNAL void lluv_req_ref(lua_State *L, lluv_req_t *req);

LLUV_INTERNAL void lluv_req_ref_ctx(lua_State *L, lluv_req_t *req);

LLUV_INTERNAL lluv_req_t* lluv_req_byptr(uv_req_t *r);

LLUV_INTERNAL int lluv_req_has_cb(lua_State *L, lluv_req_t *req);

#endif

/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2016 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lluv library.
******************************************************************************/

#include "lluv.h"
#include "lluv_req.h"
#include <assert.h>


LLUV_INTERNAL lluv_req_t* lluv_req_new(lua_State *L, uv_req_type type, lluv_handle_t *h){
  size_t extra_size = uv_req_size(type) - sizeof(uv_req_t);
  lluv_req_t *req = (lluv_req_t*)lluv_alloc(L, sizeof(lluv_req_t) + extra_size);

  req->req.data = req;
  req->handle   = h;
  req->cb       = luaL_ref(L, LLUV_LUA_REGISTRY);
  req->arg      = LUA_NOREF;
  req->ctx      = LUA_NOREF;

  if(h) lluv_handle_lock(L, h, LLUV_LOCK_REQ);

  return req;
}

LLUV_INTERNAL void lluv_req_free(lua_State *L, lluv_req_t *req){
  luaL_unref(L, LLUV_LUA_REGISTRY, req->cb);
  luaL_unref(L, LLUV_LUA_REGISTRY, req->arg);
  luaL_unref(L, LLUV_LUA_REGISTRY, req->ctx);
  if(req->handle){
    lluv_handle_unlock(L, req->handle, LLUV_LOCK_REQ);
  }
  lluv_free(L, req);
}

LLUV_INTERNAL lluv_req_t* lluv_req_byptr(uv_req_t *r){
  size_t off = offsetof(lluv_req_t, req);
  lluv_req_t *req = (lluv_req_t *)(((char*)r) - off);
  assert(req == r->data);
  return req;
}

LLUV_INTERNAL void lluv_req_ref(lua_State *L, lluv_req_t *req){
  luaL_unref(L, LLUV_LUA_REGISTRY, req->arg);
  req->arg = luaL_ref(L, LLUV_LUA_REGISTRY);
}

LLUV_INTERNAL void lluv_req_ref_ctx(lua_State *L, lluv_req_t *req){
  luaL_unref(L, LLUV_LUA_REGISTRY, req->ctx);
  req->ctx = luaL_ref(L, LLUV_LUA_REGISTRY);
}

LLUV_INTERNAL int lluv_req_has_cb(lua_State *L, lluv_req_t *req){
  int res;
  lua_rawgeti(L, LLUV_LUA_REGISTRY, req->cb);
  res = lua_isnil(L, -1);
  lua_pop(L, 1);

  return res;
}
/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2016 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lluv library.
******************************************************************************/

#include "lluv_fbuf.h"
#include "lluv_utils.h"

//! @todo implement pack/unpack functions

//{ Fixed buffer

#define LLUV_FIXEDBUFFER_NAME LLUV_PREFIX" Fixed buffer"
static const char *LLUV_FIXEDBUFFER = LLUV_FIXEDBUFFER_NAME;

LLUV_INTERNAL lluv_fixed_buffer_t *lluv_fbuf_alloc(lua_State *L, size_t n){
  lluv_fixed_buffer_t *buffer = (lluv_fixed_buffer_t*)lutil_newudatap_impl(L, sizeof(lluv_fixed_buffer_t) + n - 1, LLUV_FIXEDBUFFER);
  buffer->capacity = n;
  
  // this prevent GC so user shoul do this explicitly
  // but we remove ref in close method
  //
  // lua_pushvalue(L, -1);
  // lua_rawsetp(L, LLUV_LUA_REGISTRY, &buffer->data[0]);
  return buffer;
}

LLUV_INTERNAL lluv_fixed_buffer_t *lluv_check_fbuf(lua_State *L, int i){
  lluv_fixed_buffer_t *buffer = (lluv_fixed_buffer_t *)lutil_checkudatap (L, i, LLUV_FIXEDBUFFER);
  luaL_argcheck (L, buffer != NULL, i, LLUV_FIXEDBUFFER_NAME" expected");
  return buffer;
}

static int lluv_fbuf_new(lua_State *L){
  int64_t len = lutil_checkint64(L, 1);
  /*lluv_fixed_buffer_t *buffer = */lluv_fbuf_alloc(L, (size_t)len);
  return 1;
}

static int lluv_fbuf_close(lua_State *L){
  lluv_fixed_buffer_t *buffer = lluv_check_fbuf(L, 1);
  lua_pushnil(L);
  lua_rawsetp(L, LLUV_LUA_REGISTRY, &buffer->data[0]);
  return 0;
}

static int lluv_fbuf_to_s(lua_State *L){
  lluv_fixed_buffer_t *buffer = lluv_check_fbuf(L, 1);
  int64_t len = buffer->capacity;
  int64_t off = 0;

  if(lua_gettop(L) > 2){
    off = lutil_checkint64(L, 2);
    len = lutil_checkint64(L, 3);
  }
  else if(lua_gettop(L) > 1){
    off = 0;
    len = lutil_checkint64(L, 2);
  }

  luaL_argcheck (L, buffer->capacity >= ((size_t)off + len), 2, LLUV_PREFIX" out of index");

  lua_pushlstring(L, buffer->data + off, (size_t)len);
  return 1;
}

static int lluv_fbuf_topointer(lua_State *L){
  lluv_fixed_buffer_t *buffer = lluv_check_fbuf(L, 1);
  int64_t off = 0;

  if(lua_gettop(L) > 1){
    off = lutil_checkint64(L, 2);
  }

  luaL_argcheck(L, buffer->capacity > (size_t)off, 2, LLUV_PREFIX" capacity out of index");

  lua_pushlightuserdata(L, buffer->data + off);
  lutil_pushint64(L, buffer->capacity - off);
  return 2;
}

static int lluv_fbuf_size(lua_State *L){
  lluv_fixed_buffer_t *buffer = lluv_check_fbuf(L, 1);
  lutil_pushint64(L, buffer->capacity);
  return 1;
}

static const struct luaL_Reg lluv_fbuf_methods[] = {
  { "__gc",        lluv_fbuf_close          },
  { "__tostring",  lluv_fbuf_to_s           },
  { "free",        lluv_fbuf_close          },
  { "to_s",        lluv_fbuf_to_s           },
  { "to_p",        lluv_fbuf_topointer      },
  { "size",        lluv_fbuf_size           },

  {NULL,NULL}
};

//}

static const struct luaL_Reg lluv_fbuf_functions[] = {
  { "buffer",      lluv_fbuf_new    },

  {NULL,NULL}
};

void lluv_fbuf_initlib(lua_State *L, int nup, int safe){
  lutil_pushnvalues(L, nup);
  if(!lutil_createmetap(L, LLUV_FIXEDBUFFER, lluv_fbuf_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);

  luaL_setfuncs(L, lluv_fbuf_functions, nup);
}

/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2016 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lluv library.
******************************************************************************/

#include "l52util.h"

#include <memory.h>
#include <assert.h>

#if LUA_VERSION_NUM >= 502 

int luaL_typerror (lua_State *L, int narg, const char *tname) {
  const char *msg = lua_pushfstring(L, "%s expected, got %s", tname,
      luaL_typename(L, narg));
  return luaL_argerror(L, narg, msg);
}

#ifndef luaL_register

void luaL_register (lua_State *L, const char *libname, const luaL_Reg *l){
  if(libname) lua_newtable(L);
  luaL_setfuncs(L, l, 0);
}

#endif

#else 

void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup){
  luaL_checkstack(L, nup, "too many upvalues");
  for (; l->name != NULL; l++) {  /* fill the table with given functions */
    int i;
    for (i = 0; i < nup; i++)  /* copy upvalues to the top */
      lua_pushvalue(L, -nup);
    lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
    lua_setfield(L, -(nup + 2), l->name);
  }
  lua_pop(L, nup);  /* remove upvalues */
}

void lua_rawgetp(lua_State *L, int index, const void *p){
  index = lua_absindex(L, index);
  lua_pushlightuserdata(L, (void *)p);
  lua_rawget(L, index);
}

void lua_rawsetp (lua_State *L, int index, const void *p){
  index = lua_absindex(L, index);
  lua_pushlightuserdata(L, (void *)p);
  lua_insert(L, -2);
  lua_rawset(L, index);
}

int luaL_getmetafield (lua_State *L, int obj, const char *event) {
  if (!lua_getmetatable(L, obj))  /* no metatable? */
    return 0;
  lua_pushstring(L, event);
  lua_rawget(L, -2);
  if (lua_isnil(L, -1)) {
    lua_pop(L, 2);  /* remove metatable and metafield */
    return 0;
  }
  else {
    lua_remove(L, -2);  /* remove only metatable */
    return 1;
  }
}

int luaL_callmeta (lua_State *L, int obj, const char *event) {
  obj = lua_absindex(L, obj);
  if (!luaL_getmetafield(L, obj, event))  /* no metafield? */
    return 0;
  lua_pushvalue(L, obj);
  lua_call(L, 1, 1);
  return 1;
}

#endif

int lutil_newmetatablep (lua_State *L, const void *p) {
  lua_rawgetp(L, LUA_REGISTRYINDEX, p);
  if (!lua_isnil(L, -1))  /* name already in use? */
    return 0;  /* leave previous value on top, but return 0 */
  lua_pop(L, 1);

  lua_newtable(L);  /* create metatable */
  lua_pushvalue(L, -1); /* duplicate metatable to set*/
  lua_rawsetp(L, LUA_REGISTRYINDEX, p);

  return 1;
}

void lutil_getmetatablep (lua_State *L, const void *p) {
  lua_rawgetp(L, LUA_REGISTRYINDEX, p);
}

void lutil_setmetatablep (lua_State *L, const void *p) {
  lutil_getmetatablep(L, p);
  assert(lua_istable(L,-1));
  lua_setmetatable (L, -2);
}

int lutil_isudatap (lua_State *L, int ud, const void *p) {
  if (lua_isuserdata(L, ud)){
    if (lua_getmetatable(L, ud)) {           /* does it have a metatable? */
      int res;
      lutil_getmetatablep(L,p);              /* get correct metatable */
      res = lua_rawequal(L, -1, -2);         /* does it have the correct mt? */
      lua_pop(L, 2);                         /* remove both metatables */
      return res;
    }
  }
  return 0;
}

void *lutil_checkudatap (lua_State *L, int ud, const void *p) {
  void *up = lua_touserdata(L, ud);
  if (up != NULL) {                   /* value is a userdata? */
    if (lua_getmetatable(L, ud)) {    /* does it have a metatable? */
      lutil_getmetatablep(L,p);       /* get correct metatable */
      if (lua_rawequal(L, -1, -2)) {  /* does it have the correct mt? */
        lua_pop(L, 2);                /* remove both metatables */
        return up;
      }
    }
  }
  luaL_typerror(L, ud, p);  /* else error */
  return NULL;              /* to avoid warnings */
}

int lutil_createmetap (lua_State *L, const void *p, const luaL_Reg *methods, int nup) {
  if (!lutil_newmetatablep(L, p)){
    lua_insert(L, -1 - nup);       /* move mt prior upvalues */
    return 0;
  }

  lua_insert(L, -1 - nup);         /* move mt prior upvalues */
  luaL_setfuncs (L, methods, nup); /* define methods */
  lua_pushliteral (L, "__index");  /* define metamethods */
  lua_pushvalue (L, -2);
  lua_settable (L, -3);

  return 1;
}

void *lutil_newudatap_impl(lua_State *L, size_t size, const void *p){
  void *obj = lua_newuserdata (L, size);
  memset(obj, 0, size);
  lutil_setmetatablep(L, p);
  return obj;
}

void lutil_pushint64(lua_State *L, int64_t v){
  if(sizeof(lua_Integer) >= sizeof(int64_t)){
    lua_pushinteger(L, (lua_Integer)v);
    return;
  }
  lua_pushnumber(L, (lua_Number)v);
}

int64_t lutil_checkint64(lua_State *L, int idx){
  if(sizeof(lua_Integer) >= sizeof(int64_t))
    return luaL_checkinteger(L, idx);
  return (int64_t)luaL_checknumber(L, idx);
}

int64_t lutil_optint64(lua_State *L, int idx, int64_t v){
  if(sizeof(lua_Integer) >= sizeof(int64_t))
    return luaL_optinteger(L, idx, v);
  return (int64_t)luaL_optnumber(L, idx, (lua_Number)v);
}

void lutil_pushnvalues(lua_State *L, int n){
  int i = -n;
  for(;n;--n) lua_pushvalue(L, i);
}

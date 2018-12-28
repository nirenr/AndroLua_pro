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
#include "lluv_utils.h"
#include "lluv_list.h"
#include <assert.h>

LLUV_INTERNAL void lluv_list_init(lua_State *L, lluv_list_t *lst){
  lst->first = 0;
  lst->last = -1;
  lua_newtable(L);
  lst->t = luaL_ref(L, LLUV_LUA_REGISTRY);
}

LLUV_INTERNAL void lluv_list_close(lua_State *L, lluv_list_t *lst){
  lst->first = 0;
  lst->last = -1;
  luaL_unref(L, LLUV_LUA_REGISTRY, lst->t);
  lst->t = LUA_NOREF;
}

static void lluv_list_push_(lua_State *L, lluv_list_t *lst, lua_Integer i){
  int top = lua_gettop(L);

  assert(!lua_isnoneornil(L, -1));

  luaL_checkstack(L, 1, "too many arguments");

  lua_rawgeti(L, LLUV_LUA_REGISTRY, lst->t);
  lua_insert(L, -2);

  lua_rawseti(L, -2, i);
  lua_pop(L, 1);

  assert(top == (1 + lua_gettop(L)));
}

static int lluv_list_pop_(lua_State *L, lluv_list_t *lst, lua_Integer i){
  if(lst->first > lst->last) return 0;

  assert((lst->first == i)||(lst->last == i));

  luaL_checkstack(L, 3, "too many arguments");

  lua_rawgeti(L, LLUV_LUA_REGISTRY, lst->t);
  lua_rawgeti(L, -1, i);

  assert(!lua_isnoneornil(L, -1));

  lua_pushnil(L);
  lua_rawseti(L, -3, i);

  lua_remove(L, -2);
  return 1;
}

LLUV_INTERNAL void lluv_list_push_front(lua_State *L, lluv_list_t *lst){
  lua_Integer i = lst->first - 1;
  lluv_list_push_(L, lst, i);
  lst->first = i;
}

LLUV_INTERNAL void lluv_list_push_back(lua_State *L, lluv_list_t *lst){
  lua_Integer i = lst->last + 1;
  lluv_list_push_(L, lst, i);
  lst->last = i;
}

LLUV_INTERNAL int lluv_list_pop_front(lua_State *L, lluv_list_t *lst){
  int i = lluv_list_pop_(L, lst, lst->first);
  lst->first += i;
  return i;
}

LLUV_INTERNAL int lluv_list_pop_back(lua_State *L, lluv_list_t *lst){
  int i = lluv_list_pop_(L, lst, lst->last);
  lst->last -= i;
  return i;
}

LLUV_INTERNAL size_t lluv_list_size(lua_State *L, lluv_list_t *lst){
  return (size_t)(lst->last - lst->first + 1);
}

LLUV_INTERNAL int lluv_list_empty(lua_State *L, lluv_list_t *lst){
  return (lst->first > lst->last)?1:0;
}


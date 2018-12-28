/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2016 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lluv library.
******************************************************************************/

#ifndef _LLUV_LIST_H_
#define _LLUV_LIST_H_

typedef struct lluv_list_tag{
  lua_Integer first;
  lua_Integer last;
  int            t;
} lluv_list_t;

LLUV_INTERNAL void lluv_list_init(lua_State *L, lluv_list_t *lst);

LLUV_INTERNAL void lluv_list_close(lua_State *L, lluv_list_t *lst);

LLUV_INTERNAL void lluv_list_push_back(lua_State *L, lluv_list_t *lst);

LLUV_INTERNAL void lluv_list_push_front(lua_State *L, lluv_list_t *lst);

LLUV_INTERNAL int lluv_list_pop_back(lua_State *L, lluv_list_t *lst);

LLUV_INTERNAL int lluv_list_pop_front(lua_State *L, lluv_list_t *lst);

LLUV_INTERNAL size_t lluv_list_size(lua_State *L, lluv_list_t *lst);

LLUV_INTERNAL int lluv_list_empty(lua_State *L, lluv_list_t *lst);

#endif

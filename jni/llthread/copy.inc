/******************************************************************************
* Copyright (c) 2011 by Robert G. Jakabosky <bobby@sharedrealm.com>
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
******************************************************************************/

/* maximum recursive depth of table copies. */
#define MAX_COPY_DEPTH 30

typedef struct {
  lua_State *from_L;
  lua_State *to_L;
  int has_cache;
  int cache_idx;
  int is_arg;
} llthread_copy_state;

static int llthread_copy_table_from_cache(llthread_copy_state *state, int idx) {
  void *ptr;

  /* convert table to pointer for lookup in cache. */
  ptr = (void *)lua_topointer(state->from_L, idx);
  if(ptr == NULL) return 0; /* can't convert to pointer. */

  /* check if we need to create the cache. */
  if(!state->has_cache) {
    lua_newtable(state->to_L);
    lua_replace(state->to_L, state->cache_idx);
    state->has_cache = 1;
  }

  lua_pushlightuserdata(state->to_L, ptr);
  lua_rawget(state->to_L, state->cache_idx);
  if(lua_isnil(state->to_L, -1)) {
    /* not in cache. */
    lua_pop(state->to_L, 1);
    /* create new table and add to cache. */
    lua_newtable(state->to_L);
    lua_pushlightuserdata(state->to_L, ptr);
    lua_pushvalue(state->to_L, -2);
    lua_rawset(state->to_L, state->cache_idx);
    return 0;
  }
  /* found table in cache. */
  return 1;
}

static int llthread_copy_value(llthread_copy_state *state, int depth, int idx) {
  const char *str;
  size_t str_len;
  int kv_pos;

  /* Maximum recursive depth */
  if(++depth > MAX_COPY_DEPTH) {
    return luaL_error(state->from_L, "Hit maximum copy depth (%d > %d).", depth, MAX_COPY_DEPTH);
  }

  /* only support string/number/boolean/nil/table/lightuserdata. */
  switch(lua_type(state->from_L, idx)) {
  case LUA_TNIL:
    lua_pushnil(state->to_L);
    break;
  case LUA_TNUMBER:
    lua_pushnumber(state->to_L, lua_tonumber(state->from_L, idx));
    break;
  case LUA_TBOOLEAN:
    lua_pushboolean(state->to_L, lua_toboolean(state->from_L, idx));
    break;
  case LUA_TSTRING:
    str = lua_tolstring(state->from_L, idx, &(str_len));
    lua_pushlstring(state->to_L, str, str_len);
    break;
  case LUA_TLIGHTUSERDATA:
    lua_pushlightuserdata(state->to_L, lua_touserdata(state->from_L, idx));
    break;
  case LUA_TTABLE:
    /* make sure there is room on the new state for 3 values (table,key,value) */
    if(!lua_checkstack(state->to_L, 3)) {
      return luaL_error(state->from_L, "To stack overflow!");
    }
    /* make room on from stack for key/value pairs. */
    luaL_checkstack(state->from_L, 2, "From stack overflow!");

    /* check cache for table. */
    if(llthread_copy_table_from_cache(state, idx)) {
      /* found in cache don't need to copy table. */
      break;
    }
    lua_pushnil(state->from_L);
    while (lua_next(state->from_L, idx) != 0) {
      /* key is at (top - 1), value at (top), but we need to normalize these
       * to positive indices */
      kv_pos = lua_gettop(state->from_L);
      /* copy key */
      llthread_copy_value(state, depth, kv_pos - 1);
      /* copy value */
      llthread_copy_value(state, depth, kv_pos);
      /* Copied key and value are now at -2 and -1 in state->to_L. */
      lua_settable(state->to_L, -3);
      /* Pop value for next iteration */
      lua_pop(state->from_L, 1);
    }
    break;
  case LUA_TFUNCTION:
    if(lua_iscfunction(state->from_L, idx)){
      lua_CFunction fn = lua_tocfunction(state->from_L, idx);
      lua_pushcfunction(state->to_L, fn);
      break;
    }
  case LUA_TUSERDATA:
  case LUA_TTHREAD:
  default:
    if (state->is_arg) {
      return luaL_argerror(state->from_L, idx, "function/userdata/thread types un-supported.");
    } else {
      /* convert un-supported types to an error string. */
      lua_pushfstring(state->to_L, "Un-supported value: %s: %p",
        lua_typename(state->from_L, lua_type(state->from_L, idx)), lua_topointer(state->from_L, idx));
    }
  }

  return 1;
}

static int llthread_copy_values(lua_State *from_L, lua_State *to_L, int idx, int top, int is_arg) {
  llthread_copy_state state;
  int nvalues = 0;
  int n;

  nvalues = (top - idx) + 1;
  /* make sure there is room on the new state for the values. */
  if(!lua_checkstack(to_L, nvalues + 1)) {
    return luaL_error(from_L, "To stack overflow!");
  }

  /* setup copy state. */
  state.from_L = from_L;
  state.to_L = to_L;
  state.is_arg = is_arg;
  state.has_cache = 0; /* don't create cache table unless it is needed. */
  lua_pushnil(to_L);
  state.cache_idx = lua_gettop(to_L);

  nvalues = 0;
  for(n = idx; n <= top; n++) {
    llthread_copy_value(&state, 0, n);
    ++nvalues;
  }

  /* remove cache table. */
  lua_remove(to_L, state.cache_idx);

  return nvalues;
}

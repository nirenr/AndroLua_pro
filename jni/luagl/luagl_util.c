#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>

#include "luagl_util.h"


#if LUA_VERSION_NUM > 501
lua_Number luagl_tonumber(lua_State *L, int idx)
{
  return lua_tonumberx(L, idx, NULL);
}
lua_Integer luagl_tointeger(lua_State *L, int idx)
{
  return lua_tointegerx(L, idx, NULL);
}
lua_Unsigned luagl_tounsigned(lua_State *L, int idx)
{
  return lua_tounsignedx(L, idx, NULL);
}
#else
#define lua_Unsigned lua_Integer
#define lua_pushunsigned lua_pushinteger
#define luaL_checkunsigned luaL_checkinteger

lua_Number luagl_tonumber(lua_State *L, int idx)
{
  return lua_tonumber(L, idx);
}
lua_Integer luagl_tointeger(lua_State *L, int idx)
{
  return lua_tointeger(L, idx);
}
lua_Unsigned luagl_tounsigned(lua_State *L, int idx)
{
  return lua_tointeger(L, idx);
}
#endif

int luagl_checkboolean (lua_State *L, int narg) 
{
  int d = lua_toboolean(L, narg);
  if (d == 0)  /* avoid extra test when d is not 0 */
    luaL_checktype(L, narg, LUA_TBOOLEAN);
  return d;
}

void* luagl_checkuserdata (lua_State *L, int narg) 
{
  luaL_checktype(L, narg, LUA_TLIGHTUSERDATA);
  return lua_touserdata(L, narg);
}


/* returns a bi-dimensional parray with given type and size */
#define LUAGL_NEW_ARRAY2(type, size1, size2) ( (type *)malloc((size1) * (size2) * sizeof(type)) )

#define LUAGL_INIT_ARRAY(parray, _type, size, conversionFunc)   \
{                                                               \
  int i;                                                        \
  for(i = 0; i < (size); i++) {                                 \
    lua_rawgeti(L, index, i+1);                                 \
    (parray)[i] = (_type)(conversionFunc)(L, -1);               \
    lua_remove(L, -1);                                          \
  }                                                             \
}

#define LUAGL_INIT_ARRAY2(parray, _type, size1, size2, conversionFunc)  \
{                                                               \
  int i, j;                                                     \
  for(i = 0; i < size1; i++)  {                                 \
    lua_rawgeti(L, index, i+1);                                 \
    if(!lua_istable(L, -1)) return -1;                          \
    for(j = 0; j < size2; j++) {                                \
      lua_rawgeti(L, -1, j+1);                                  \
      (parray)[i*(size2) + j] = (_type)(conversionFunc)(L, -1); \
      lua_remove(L, -1);                                        \
    }                                                           \
    lua_remove(L, -1);                                          \
  }                                                             \
}

#define DEFINE_TO_ARRAY_FUNC(name, _type, conversionFunc) \
  void name(lua_State *L, int index, _type *parray)       \
{                                                         \
  int n;                                                  \
  luaL_checktype(L, index, LUA_TTABLE);                   \
  n = luagl_getn(L, index);                               \
  LUAGL_INIT_ARRAY(parray, _type, n, conversionFunc);     \
}

DEFINE_TO_ARRAY_FUNC(luagl_to_arrayuc, unsigned char, luagl_tounsigned)
DEFINE_TO_ARRAY_FUNC(luagl_to_arrayc, char, luagl_tointeger)
DEFINE_TO_ARRAY_FUNC(luagl_to_arrayus, unsigned short, luagl_tounsigned)
DEFINE_TO_ARRAY_FUNC(luagl_to_arrays, short, luagl_tointeger)
DEFINE_TO_ARRAY_FUNC(luagl_to_arrayui, unsigned int, luagl_tounsigned)
DEFINE_TO_ARRAY_FUNC(luagl_to_arrayi, int, luagl_tointeger)
DEFINE_TO_ARRAY_FUNC(luagl_to_arrayf, float, luagl_tonumber)
//DEFINE_TO_ARRAY_FUNC(luagl_to_arrayd, double, luagl_tonumber)


/* Gets an parray from a lua table, store it in 'parray' and returns the no. of elems of the parray
index refers to where the table is in stack. */
#define DEFINE_GET_ARRAY_FUNC(name, _type, conversionFunc) \
  int name(lua_State *L, int index, _type **parray)        \
{                                                          \
  int n;                                                   \
  luaL_checktype(L, index, LUA_TTABLE);                    \
  n = luagl_getn(L, index);                                 \
  *parray = LUAGL_NEW_ARRAY(_type, n);                     \
  LUAGL_INIT_ARRAY(*parray, _type, n, conversionFunc);     \
  return n;                                                \
}

#define DEFINE_GET_ARRAY2_FUNC(name, _type, conversionFunc)    \
  int name(lua_State *L, int index, _type **parray, int *size) \
{                                                              \
  int n;                                                       \
  luaL_checktype(L, index, LUA_TTABLE);                        \
  n = luagl_getn(L, index);                                     \
  lua_rawgeti(L, index, 1);                                    \
  if(!lua_istable(L, -1)) { lua_remove(L, -1); return -1; }    \
  *size = luagl_getn(L, -1);                                    \
  *parray = LUAGL_NEW_ARRAY2(_type, n, *size);                 \
  LUAGL_INIT_ARRAY2(*parray, _type, n, *size, conversionFunc);  \
  lua_remove(L, -1);                                           \
  return n;                                                    \
}

DEFINE_GET_ARRAY_FUNC(luagl_get_arrayb, unsigned char, lua_toboolean)
//DEFINE_GET_ARRAY_FUNC(luagl_get_arrayd, double, luagl_tonumber)
DEFINE_GET_ARRAY_FUNC(luagl_get_arrayf, float, luagl_tonumber)
DEFINE_GET_ARRAY_FUNC(luagl_get_arrayi, int, luagl_tointeger)
DEFINE_GET_ARRAY_FUNC(luagl_get_arrayui, unsigned int, luagl_tounsigned)
DEFINE_GET_ARRAY_FUNC(luagl_get_arrayuc, unsigned char, luagl_tounsigned)

DEFINE_GET_ARRAY2_FUNC(luagl_get_array2uc, unsigned char, luagl_tounsigned)
//DEFINE_GET_ARRAY2_FUNC(luagl_get_array2d, double, luagl_tonumber)
DEFINE_GET_ARRAY2_FUNC(luagl_get_array2f, float, luagl_tonumber)

#undef DEFINE_GET_ARRAY_FUNC
#undef DEFINE_GET_ARRAY2_FUNC

#define DEFINE_PUSH_ARRAY_FUNC(name, _type, pushFunc) \
  void name(lua_State *L, _type *parray, int size)    \
{                                                     \
  int i;                                              \
  lua_createtable(L, size, 0);                        \
  for(i = 0; i < size; i++)                           \
  {                                                   \
    lua_pushinteger(L, i+1);                          \
    pushFunc(L, (_type)parray[i]);                    \
    lua_settable(L, -3);                              \
  }                                                   \
}

DEFINE_PUSH_ARRAY_FUNC(luagl_push_arrayb, unsigned char, lua_pushboolean)
DEFINE_PUSH_ARRAY_FUNC(luagl_push_arrayuc, unsigned char, lua_pushunsigned)
DEFINE_PUSH_ARRAY_FUNC(luagl_push_arrayc, char, lua_pushinteger)
DEFINE_PUSH_ARRAY_FUNC(luagl_push_arrayus, unsigned short, lua_pushunsigned)
DEFINE_PUSH_ARRAY_FUNC(luagl_push_arrays, short, lua_pushinteger)
DEFINE_PUSH_ARRAY_FUNC(luagl_push_arrayui, unsigned int, lua_pushunsigned)
DEFINE_PUSH_ARRAY_FUNC(luagl_push_arrayi, int, lua_pushinteger)
DEFINE_PUSH_ARRAY_FUNC(luagl_push_arrayf, float, lua_pushnumber)
//DEFINE_PUSH_ARRAY_FUNC(luagl_push_arrayd, double, lua_pushnumber)

#define DEFINE_PUSH_ARRAY2_FUNC(name, _type, pushFunc) \
  void name(lua_State *L, _type *parray, int height, int width)    \
{                                                     \
  int i, j;                                           \
  lua_createtable(L, height, 0);                      \
  for(i = 0; i < height; i++)                         \
  {                                                   \
    lua_pushinteger(L, i+1);                          \
    lua_createtable(L, width, 0);                     \
    for(j = 0; j < width; j++)                        \
    {                                                 \
      lua_pushinteger(L, j+1);                        \
      pushFunc(L, (_type)parray[i*width+j]);          \
      lua_settable(L, -3);                            \
    }                                                 \
    lua_settable(L, -3);                              \
  }                                                   \
}

DEFINE_PUSH_ARRAY2_FUNC(luagl_push_array2f, float, lua_pushnumber)

#undef DEFINE_PUSH_ARRAY_FUNC
#undef DEFINE_PUSH_ARRAY2_FUNC

int luagl_str2mask(const char *str)
{
  int i, j;
  int mask = 0;
  int size = (int)strlen(str);
  for(i = 0, j = 0; j < size; i++)
  {
    if(str[i] == '1')
    {
      mask |= (1 << (size-1-j));
      j++;
    }
    else if(str[i] == '0')
      j++;

  }
  return mask;
}

const char *luagl_mask2str(int mask)
{
  unsigned int i;
  static char str[17];
  for(i = 0; i < 16; i++)
  {
    if(mask & (1 << (15 - i)))
      str[i] = '1';
    else
      str[i] = '0';
  }
  str[i] = 0;
  return str;
}


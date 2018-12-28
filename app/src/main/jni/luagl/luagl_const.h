#ifndef __LUAGL_CONST_H__
#define __LUAGL_CONST_H__


#define LUAGL_ENUM_ERROR (unsigned int)-2

typedef struct luaglConst  
{
  const char *str;
  unsigned int value;
} luaglConst;

void luagl_initconst(lua_State *L, const luaglConst *gl_const);
unsigned int luagl_get_enum(lua_State *L, int index, const luaglConst* gl_const);
unsigned int luagl_get_gl_enum(lua_State *L, int index);
const char *luagl_get_str_gl_enum(unsigned int num);
void luagl_pushenum(lua_State *L, unsigned int num);
void luagl_open_const(lua_State *L);

#endif

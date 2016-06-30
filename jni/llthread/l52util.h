#ifndef _LZUTILS_H_9B43D914_9652_4E22_9A43_8073502BF3F4_
#define _LZUTILS_H_9B43D914_9652_4E22_9A43_8073502BF3F4_

#include "lua.h"
#include "lauxlib.h"

#if LUA_VERSION_NUM >= 503 /* Lua 5.3 */

#ifndef luaL_optint
# define luaL_optint luaL_optinteger
#endif

#ifndef luaL_checkint
# define luaL_checkint luaL_checkinteger
#endif

#endif

#if LUA_VERSION_NUM >= 502 // lua 5.2

// lua_rawgetp
// lua_rawsetp
// luaL_setfuncs
// lua_absindex

#ifndef lua_objlen

#define lua_objlen      lua_rawlen

#endif

int   luaL_typerror (lua_State *L, int narg, const char *tname);

#ifndef luaL_register

void luaL_register (lua_State *L, const char *libname, const luaL_Reg *l);

#endif

#define lutil_require luaL_requiref

#else                      // lua 5.1

// functions form lua 5.2

# define lua_absindex(L, i) (((i)>0)?(i):((i)<=LUA_REGISTRYINDEX?(i):(lua_gettop(L)+(i)+1)))
# define lua_rawlen  lua_objlen

void  lua_rawgetp   (lua_State *L, int index, const void *p);
void  lua_rawsetp   (lua_State *L, int index, const void *p);
void  luaL_setfuncs  (lua_State *L, const luaL_Reg *l, int nup);

void lutil_require(lua_State *L, const char* name, lua_CFunction fn, int glb);

#endif

int   lutil_newmetatablep (lua_State *L, const void *p);
void  lutil_getmetatablep (lua_State *L, const void *p);
void  lutil_setmetatablep (lua_State *L, const void *p);

#define lutil_newudatap(L, TTYPE, TNAME) (TTYPE *)lutil_newudatap_impl(L, sizeof(TTYPE), TNAME)
int   lutil_isudatap      (lua_State *L, int ud, const void *p);
void *lutil_checkudatap   (lua_State *L, int ud, const void *p);
int   lutil_createmetap   (lua_State *L, const void *p, const luaL_Reg *methods, int nup);

void *lutil_newudatap_impl     (lua_State *L, size_t size, const void *p);

#endif


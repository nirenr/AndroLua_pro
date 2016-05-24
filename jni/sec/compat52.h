/* ==========================================================================
 * compat52.h - Routines for Lua 5.2 compatibility
 * --------------------------------------------------------------------------
 * Copyright (c) 2012  William Ahern
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 * ==========================================================================
 */
#if LUA_VERSION_NUM < 502

#define LUA_OK 0


static void luaL_setmetatable(lua_State *L, const char *tname) {
	luaL_getmetatable(L, tname);
	lua_setmetatable(L, -2);
} /* luaL_setmetatable() */


static int lua_absindex(lua_State *L, int idx) {
	return (idx > 0 || idx <= LUA_REGISTRYINDEX)? idx : lua_gettop(L) + idx + 1;
} /* lua_absindex() */


static void *luaL_testudata(lua_State *L, int arg, const char *tname) {
	void *p = lua_touserdata(L, arg);
	int eq;

	if (!p || !lua_getmetatable(L, arg))
		return 0;

	luaL_getmetatable(L, tname);
	eq = lua_rawequal(L, -2, -1);
	lua_pop(L, 2);

	return (eq)? p : 0;
} /* luaL_testudate() */


static void luaL_setfuncs(lua_State *L, const luaL_Reg *l, int nup) {
	int i, t = lua_absindex(L, -1 - nup);

	for (; l->name; l++) {
		for (i = 0; i < nup; i++)
			lua_pushvalue(L, -nup);
		lua_pushcclosure(L, l->func, nup);
		lua_setfield(L, t, l->name);
	}

	lua_pop(L, nup);
} /* luaL_setfuncs() */


#define luaL_newlibtable(L, l) \
	lua_createtable(L, 0, (sizeof (l) / sizeof *(l)) - 1)

#define luaL_newlib(L, l) \
	(luaL_newlibtable((L), (l)), luaL_setfuncs((L), (l), 0))


static void luaL_requiref(lua_State *L, const char *modname, lua_CFunction openf, int glb) {
	lua_pushcfunction(L, openf);
	lua_pushstring(L, modname);
	lua_call(L, 1, 1);

	lua_getglobal(L, "package");
	lua_getfield(L, -1, "loaded");
	lua_pushvalue(L, -3);
	lua_setfield(L, -2, modname);

	lua_pop(L, 2);
	
	if (glb) {
		lua_pushvalue(L, -1);
		lua_setglobal(L, modname);
	}
} /* luaL_requiref() */


#define lua_resume(L, from, nargs) lua_resume((L), (nargs))


static void lua_rawgetp(lua_State *L, int index, const void *p) {
	index = lua_absindex(L, index);
	lua_pushlightuserdata(L, (void *)p);
	lua_rawget(L, index);
} /* lua_rawgetp() */

static void lua_rawsetp(lua_State *L, int index, const void *p) {
	index = lua_absindex(L, index);
	lua_pushlightuserdata(L, (void *)p);
	lua_pushvalue(L, -2);
	lua_rawset(L, index);
	lua_pop(L, 1);
} /* lua_rawsetp() */


#ifndef LUA_UNSIGNED
#define LUA_UNSIGNED unsigned
#endif

typedef LUA_UNSIGNED lua_Unsigned;


static void lua_pushunsigned(lua_State *L, lua_Unsigned n) {
	lua_pushnumber(L, (lua_Number)n);
} /* lua_pushunsigned() */

static lua_Unsigned luaL_checkunsigned(lua_State *L, int arg) {
	return (lua_Unsigned)luaL_checknumber(L, arg);
} /* luaL_checkunsigned() */


static lua_Unsigned luaL_optunsigned(lua_State *L, int arg, lua_Unsigned def) {
	return (lua_Unsigned)luaL_optnumber(L, arg, (lua_Number)def);
} /* luaL_optunsigned() */


#ifndef LUA_FILEHANDLE /* Not defined by earlier LuaJIT releases */
#define LUA_FILEHANDLE "FILE*"
#endif

/*
 * Lua 5.1 userdata is a simple FILE *, while LuaJIT is a struct with the
 * first member a FILE *, similar to Lua 5.2.
 */
typedef struct luaL_Stream {
	FILE *f;
} luaL_Stream;


#define lua_rawlen(...) lua_objlen(__VA_ARGS__)


#define lua_pushstring(...) lua52_pushstring(__VA_ARGS__)

static const char *lua52_pushstring(lua_State *L, const char *s) {
	(lua_pushstring)(L, s);
	return lua_tostring(L, -1);
} /* lua52_pushstring() */


#endif /* LUA_VERSION_NUM < 502 */

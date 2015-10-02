#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <android/log.h>

#include "libtcc.h"

#include <jni.h>
#include <android/log.h>
#define null NULL
#define countof(a) (sizeof(a) / sizeof((a)[0]))

#if defined __WIN32__ || defined WIN32
# include <windows.h>
# define _EXPORT __declspec(dllexport)
#else
# define _EXPORT
#endif

#include <jni.h>
#include <stdio.h>
#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

void bar(const char* fmt, ...) {
    va_list vl;
    va_start(vl, fmt);
    __android_log_vprint(ANDROID_LOG_VERBOSE, "tcc", fmt, vl);
    va_end(vl);
}

void trace(const char* msg) {
    __android_log_print(ANDROID_LOG_ERROR, "tcc", "%s", msg);
}

#define LUAJAVAJNIENVTAG "_JNIEnv"

static JNIEnv *checkEnv(lua_State *L) {
  JNIEnv *javaEnv = (JNIEnv *)0;
  lua_pushstring(L, LUAJAVAJNIENVTAG);
  lua_rawget(L, LUA_REGISTRYINDEX);

  if (lua_isuserdata(L, -1))
    javaEnv = *(JNIEnv **)lua_touserdata(L, -1);
  lua_pop(L, 1);
  if (!javaEnv)
    luaL_error(L, "Invalid JNI Environment.");
  return javaEnv;
}

static void error_func(void *opaque, const char *msg) {
	trace(msg);
}


int l_tcc_new(lua_State * L)
{
    TCCState *s;
    s = tcc_new();
    if (!s) {
        trace("Could not create tcc state");
    	lua_pushstring(L, "Could not create tcc state");
		lua_error(L);
        return 0;
    }
    tcc_set_options(s, "-D__GNUC__ -D__ANDROID__");
    tcc_set_error_func(s, null, error_func);
	tcc_set_lib_path(s, "/system/lib"); // libc.so
    tcc_add_symbol(s, "printf", bar);

    /* MUST BE CALLED before any compilation */
    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);

	pushtccstate(L,s);
	return 1;
}

int l_tcc_delete(lua_State * L)
{
	TCCState *s;
	s=*(TCCState**)lua_touserdata(L,1);
	tcc_delete(s);
	return 0;
}

int l_tcc_set_lib_path(lua_State * L)
{
	TCCState *s;
	s=*(TCCState**)lua_touserdata(L,1);
	tcc_set_lib_path(s,lua_tostring(L,1));
	return 0;
}

int l_tcc_set_options(lua_State * L)
{
	TCCState *s;
	s=*(TCCState**)lua_touserdata(L,1);
	const char *str=lua_tostring(L,2);
    int ret=tcc_set_options(s, str);
	lua_pushinteger(L,ret);
	return 1;
}

int l_tcc_add_sysinclude_path(lua_State * L)
{
	TCCState *s;
	s=*(TCCState**)lua_touserdata(L,1);
	const char *str=lua_tostring(L,2);
    int ret=tcc_add_sysinclude_path(s, str);
	lua_pushinteger(L,ret);
	return 1;
}

int l_tcc_add_include_path(lua_State * L)
{
	TCCState *s;
	s=*(TCCState**)lua_touserdata(L,1);
	const char *str=lua_tostring(L,2);
    int ret=tcc_add_include_path(s, str);
	lua_pushinteger(L,ret);
	return 1;
}

int l_tcc_define_symbol(lua_State * L)
{
	TCCState *s;
	s=*(TCCState**)lua_touserdata(L,1);
	tcc_define_symbol(s,lua_tostring(L,2),lua_tostring(L,3));
	return 0;
}

int l_tcc_undefine_symbol(lua_State * L)
{
	TCCState *s;
	s=*(TCCState**)lua_touserdata(L,1);
	tcc_undefine_symbol(s,lua_tostring(L,2));
	return 0;
}

int l_tcc_add_file(lua_State * L)
{
	TCCState *s;
	s=*(TCCState**)lua_touserdata(L,1);
	const char *str=lua_tostring(L,2);
	int ret=tcc_add_file(s, str);
    if (ret == -1) {
        trace("Could not tcc_add_file");
      	lua_pushstring(L, "Could not tcc_add_file");
		lua_error(L);
    }
	lua_pushinteger(L,ret);
	return 1;
}

int l_tcc_compile_string(lua_State * L)
{
	TCCState *s;
	s=*(TCCState**)lua_touserdata(L,1);
	const char *m_program=lua_tostring(L,2);
	int ret=tcc_compile_string(s, m_program);
    if (ret == -1) {
        trace("Could not tcc_compile_string");
      	lua_pushstring(L, "Could not tcc_compile_string");
		lua_error(L);
    }
	tcc_add_symbol(s, "L", &L);
	JNIEnv *env=checkEnv(L);
	tcc_add_symbol(s, "env", &env);

	lua_pushinteger(L,ret);
	return 1;
}

int l_tcc_set_output_type(lua_State * L)
{
	TCCState *s;
	s=*(TCCState**)lua_touserdata(L,1);
	int i=(int)lua_tointeger(L,2);
    int ret=tcc_set_output_type(s, i);
	lua_pushinteger(L,ret);
	return 1;
}

int l_tcc_add_library_path(lua_State * L)
{
	TCCState *s;
	s=*(TCCState**)lua_touserdata(L,1);
	const char *str=lua_tostring(L,2);
    int ret=tcc_add_library_path(s, str);
	lua_pushinteger(L,ret);
	return 1;
}
int l_tcc_add_library(lua_State * L)
{
	TCCState *s;
	s=*(TCCState**)lua_touserdata(L,1);
	const char *str=lua_tostring(L,2);
    int ret=tcc_add_library(s, str);
	lua_pushinteger(L,ret);
	return 1;
}

int l_tcc_output_file(lua_State * L)
{
	TCCState *s;
	s=*(TCCState**)lua_touserdata(L,1);
	const char *str=lua_tostring(L,2);
    int ret=tcc_output_file(s, str);
	lua_pushinteger(L,ret);
	return 1;
}

int l_tcc_run(lua_State * L)
{
	TCCState *s;
	s=*(TCCState**)lua_touserdata(L,1);
	
	int top = lua_gettop(L);
	char* args[top - 1];

	int i;
	for (i = 2; i <= top; i++)
	{
		args[i - 2] = lua_tostring(L,i);
	}

    int ret=tcc_run(s, countof(args), args);
	lua_pushinteger(L,ret);
	return 1;
}

int l_tcc_get_symbol(lua_State * L)
{
	TCCState *s;
	int (*func)(int);
	s=*(TCCState**)lua_touserdata(L,1);
	const char *str=lua_tostring(L,2);
    pushvoid(L,tcc_get_symbol(s, str));
	return 1;
}

int l_tcc_get_function(lua_State * L)
{
	TCCState *s;
	int (*func)(lua_State * );
	s=*(TCCState**)lua_touserdata(L,1);
	const char *str=lua_tostring(L,2);
    func=tcc_get_symbol(s, str);
	lua_pushcfunction(L,func);
	return 1;
}

int l_tcc_call(lua_State * L)
{
	TCCState *s;
	int (*func)(void *);
	s=*(TCCState**)lua_touserdata(L,1);
    func = tcc_get_symbol(s, "foo");
    void * arg;
	switch(lua_type(L,2)){
		case LUA_TNUMBER:
		{
			lua_Number n=lua_tonumber(L,2);
			arg=&n;
			break;}
		case LUA_TSTRING:
		{
			const char *str=lua_tostring(L,2);
			arg=&str;
			break;}
		case LUA_TUSERDATA:
			arg=lua_touserdata(L,2);
			break;
	}
	
	int ret=func(arg);
	lua_pushinteger(L,ret);
	return 1;

}

int pushtccstate(lua_State * L, TCCState* p)
{
	TCCState **userData;
	userData = (TCCState **) lua_newuserdata(L, sizeof(p));
	*userData = p;
	return 1;
}
int pushvoid(lua_State * L, void* p)
{
	void **userData;
	userData = (void **) lua_newuserdata(L, sizeof(p));
	*userData = p;
	return 1;
}


int _EXPORT luaopen_tcc(lua_State * L)
{
	static const struct luaL_reg funcs[] = {
	{"new", l_tcc_new},
	{"delete", l_tcc_delete},
	{"set_lib_path", l_tcc_set_lib_path},
	{"set_options", l_tcc_set_options},
	{"add_include_path", l_tcc_add_include_path},
	{"add_sysinclude_path", l_tcc_add_sysinclude_path},
	{"define_symbol", l_tcc_define_symbol},
	{"undefine_symbol", l_tcc_undefine_symbol},
	{"add_file", l_tcc_add_file},
	{"compile_string", l_tcc_compile_string},
	{"set_output_type", l_tcc_set_output_type},
	{"add_library_path", l_tcc_add_library_path},
	{"add_library", l_tcc_add_library},
	{"output_file", l_tcc_output_file},
	{"run", l_tcc_run},
	{"get_symbol", l_tcc_get_symbol},
	{"get_function", l_tcc_get_function},
    {"call", l_tcc_call},
	{NULL, NULL}
	};
	luaL_newlib(L, funcs);
	return 1;
}

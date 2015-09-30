#include "manifest.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <android/log.h>

#include "libtcc.h"

#include <jni.h>
#include <android/log.h>

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

/* this function is called by the generated code */
int add(int a, int b)
{
    trace("a=%d b=%d", a, b);
    return a + b;
}

void bar(const char* fmt, ...) {
    va_list vl;
    va_start(vl, fmt);
    __android_log_vprint(ANDROID_LOG_VERBOSE, "tcc", fmt, vl);
    va_end(vl);
}

#define LUAJAVAJNIENVTAG "_JNIEnv"

static inline JNIEnv *checkEnv(lua_State *L) {
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

// "#define printf(const char* fmt, ...) __android_log_print(1, \"tcc-exec\", fmt, __VA_ARGS__)\n"

char my_program[] =
"int fib(int n)\n"
"{\n"
"    if (n <= 2)\n"
"        return 1;\n"
"    else\n"
"        return fib(n-1) + fib(n-2);\n"
"}\n"
"extern int env;\n"
"int main(int argc, const char** argv)\n"
"{\n"
"    printf(\"main!\\n\");\n"
"    printf(\"argc=%d\", env);\n"
"    printf(\"argv[0]=%s\", argv[0]);\n"
"    printf(\"argv[1]=%s\", argv[1]);\n"
/*
"    char* bad = 0x0;\n"
"    *bad = 1;\n"
*/
"    return 0;\n"
"}\n"
"\n"
" int foo(int n)\n"
"{\n"
"    printf(\"Hello World!\\n\");\n"
"    printf(\"fib(%d) = %d\\n\", n, fib(n));\n"
"    printf(\"add(%d, %d) = %d\\n\", n, 2 * n, add(n, 2 * n));\n"
"    return 0;\n"
"}\n";

static void error_func(void *opaque, const char *msg) {
   bar("%s", msg);
	trace("%s", msg);
}

int test(lua_State * L)
{
    TCCState *s;
    int (*func)(int);

    s = tcc_new();
    if (!s) {
        trace("Could not create tcc state");
        return -1;
    }

    tcc_set_error_func(s, null, error_func);

    /* if tcclib.h and libtcc1.a are not installed, where can we find them */

    tcc_set_lib_path(s, "/system/lib"); // libc.so

    /* MUST BE CALLED before any compilation */
    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);

/*
http://bellard.org/tcc/tcc-doc.html
`-g'
Generate run time debug information so that you get clear run time error messages: test.c:68: in function 'test5()': dereferencing invalid pointer instead of the laconic Segmentation fault.
`-b'
Generate additional support code to check memory allocations and array/pointer bounds. `-g' is implied. Note that the generated code is slower and bigger in this case.
`-bt N'
Display N callers in stack traces. This is useful with `-g' or `-b'.
*/
    //tcc_set_options(s, "g");
    //tcc_set_options(s, "b");
    //tcc_set_options(s, "bt 30");

    if (tcc_compile_string(s, my_program) == -1) {
        trace("Could not tcc_compile_string");
        return -1;
    }
int n=123;
    /* as a test, we add a symbol that the compiled program can use.
       You may also open a dll with tcc_add_dll() and use symbols from that */
    tcc_add_symbol(s, "add", add);
	tcc_add_symbol(s, "env", &n);

//  tcc_add_file(s, "/system/lib/liblog.so");
    tcc_add_symbol(s, "printf", bar);

    if (0) {
        /* relocate the code */
        if (tcc_relocate(s, TCC_RELOCATE_AUTO) < 0) {
            trace("Could not tcc_relocate");
            return -1;
        }
        /* get entry symbol */
        func = tcc_get_symbol(s, "foo");
        if (!func) {
            trace("Could not tcc_get_symbol");
            return -1;
        }
        /* run the code */
        clock_t c = clock();
        func(33);
        c = clock() - c;
        bar("time=%f", ((float)c)/CLOCKS_PER_SEC);
    } else {
        char* args[] = {"arg1", "arg2"};
        tcc_run(s, countof(args), args);
    }
    /* delete the state */
    tcc_delete(s);

    trace("done");
    return 0;
}

static lua_State * gL;

static lua_State * getL()
{
	return gL;
}

int l_tcc_new(lua_State * L)
{
    TCCState *s;
    gL=L;
	//JNIEnv *env=checkEnv(L) ;
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
	tcc_add_symbol(s, "getL", getL);

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

int l_tcc_call(lua_State * L)
{
	//void* (*func)(void*);
	int (*func)(int);

	func=*(void**)lua_touserdata(L,1);
	int i=(int)lua_tonumber(L,2);
		
	int ret=(*func)(i);
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
	{"call", l_tcc_call},
	{"test", test},
	{NULL, NULL}
	};
	luaL_newlib(L, funcs);
	return 1;
}

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

#include <android/log.h>

#include <android/log.h>
#define LOG_TAG "lua"
/*
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG , LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO  , LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN  , LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , LOG_TAG, __VA_ARGS__)
*/
int log_v(lua_State *L) {
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"%s",lua_tostring(L,1));
	return 0;
}

int log_d(lua_State *L) {
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"%s",lua_tostring(L,1));
	return 0;
}
int log_i(lua_State *L) {
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"%s",lua_tostring(L,1));
	return 0;
}
int log_w(lua_State *L) {
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"%s",lua_tostring(L,1));
	return 0;
}
int log_e(lua_State *L) {
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"%s",lua_tostring(L,1));
	return 0;
}


int _EXPORT luaopen_alog(lua_State * L)
{
	static const struct luaL_reg funcs[] = {
		{"v", log_v},
		{"d", log_d},
		{"i", log_i},
		{"w", log_w},
		{"e", log_e},
		{NULL, NULL}
	};
	luaL_register(L, "log", funcs);
	return 1;
}



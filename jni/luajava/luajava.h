#ifndef luajava_h
#define luajava_h


#include <jni.h>
#include <stdio.h>
#include <stdlib.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

 JNIEnv *checkEnv(lua_State *L);

 void pushJNIEnv(JNIEnv *env, lua_State *L);

 jlong checkIndex(lua_State *L);

 jobject *checkJavaObject(lua_State *L, int idx);

 void checkError(JNIEnv *javaEnv, lua_State *L);

 int gc(lua_State * L);

 JNIEnv *getEnvFromState(lua_State * L);

 int isJavaObject(lua_State * L, int idx);

 int pushJavaObject(lua_State * L, jobject javaObject);

#endif

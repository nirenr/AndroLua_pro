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
#include "luajava.h"
#include <android/log.h>

#define  LOG_TAG    "luajni"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)


int jFindClass(lua_State * L)
{
	jclass clazz;
	jclass *userData;
	const char *className;
	JNIEnv *javaEnv;

	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	className = lua_tostring(L, 1);
	clazz = (*javaEnv)->FindClass(javaEnv, className);
	checkError(javaEnv, L);
	if (clazz == NULL)
		return 0;
	return pushJavaObject(L, clazz);
}

int jGetStaticMethodID(lua_State * L)
{
	lua_Number stateIndex;
	jobject *userData;
	jclass clazz;
	jmethodID method;
	const char *methodName;
	const char *sign;
	JNIEnv *javaEnv;

	if (!isJavaObject(L, 1))
	{
		lua_pushstring(L, "Not a valid java class.");
		lua_error(L);
	}

	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	sign = lua_tostring(L, 3);
	methodName = lua_tostring(L, 2);
	userData = (jobject *) lua_touserdata(L, 1);
	clazz = (jclass) * userData;

	method = (*javaEnv)->GetStaticMethodID(javaEnv, clazz, methodName, sign);
	checkError(javaEnv, L);
	if (method == NULL)
	{
		lua_pushstring(L, "method no such");
		lua_error(L);
    }
	userData = (jobject *) lua_newuserdata(L, sizeof(method));
	*userData = method;
	return 1;
}

int jGetMethodID(lua_State * L)
{
	lua_Number stateIndex;
	jobject *userData;
	jclass clazz;
	jmethodID method;
	const char *methodName;
	const char *sign;
	JNIEnv *javaEnv;


	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	sign = lua_tostring(L, 3);
	methodName = lua_tostring(L, 2);
	userData = (jclass *) lua_touserdata(L, 1);
	clazz = (jclass) * userData;

	method = (*javaEnv)->GetMethodID(javaEnv, clazz, methodName, sign);
	checkError(javaEnv, L);
	if (method == NULL)
	{
		lua_pushstring(L, "method no such");
		lua_error(L);
	}

	userData = (jmethodID *) lua_newuserdata(L, sizeof(method));
	*userData = method;
	return 1;
}

int jCallBooleanMethod(lua_State * L)
{
	lua_Number stateIndex;
	jboolean ret;
	jobject *userData;
	jobject obj;
	jmethodID method;
	jvalue *value;
	JNIEnv *javaEnv;


	int top = lua_gettop(L);
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	jvalue args[top - 2];

	int i;
	for (i = 3; i <= top; i++)
	{
		value = (jvalue *) lua_touserdata(L, i);
		args[i - 3] = (jvalue) * value;
	}

	userData = (jobject *) lua_touserdata(L, 2);
	method = (jmethodID) * userData;
	userData = (jobject *) lua_touserdata(L, 1);
	obj = (jobject) * userData;
	ret = (*javaEnv)->CallBooleanMethodA(javaEnv, obj, method, args);
	lua_pushinteger(L, ret);
	return 1;
}

int jCallByteMethod(lua_State * L)
{
	lua_Number stateIndex;
	jbyte ret;
	jobject *userData;
	jobject obj;
	jmethodID method;
	jvalue *value;
	JNIEnv *javaEnv;


	int top = lua_gettop(L);
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	jvalue args[top - 2];

	int i;
	for (i = 3; i <= top; i++)
	{
		value = (jvalue *) lua_touserdata(L, i);
		args[i - 3] = (jvalue) * value;
	}

	userData = (jobject *) lua_touserdata(L, 2);
	method = (jmethodID) * userData;
	userData = (jobject *) lua_touserdata(L, 1);
	obj = (jobject) * userData;
	ret = (*javaEnv)->CallByteMethodA(javaEnv, obj, method, args);
	lua_pushinteger(L, ret);
	return 1;
}

int jCallCharMethod(lua_State * L)
{
	lua_Number stateIndex;
	jchar ret;
	jobject *userData;
	jobject obj;
	jmethodID method;
	jvalue *value;
	JNIEnv *javaEnv;


	int top = lua_gettop(L);
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	jvalue args[top - 2];

	int i;
	for (i = 3; i <= top; i++)
	{
		value = (jvalue *) lua_touserdata(L, i);
		args[i - 3] = (jvalue) * value;
	}

	userData = (jobject *) lua_touserdata(L, 2);
	method = (jmethodID) * userData;
	userData = (jobject *) lua_touserdata(L, 1);
	obj = (jobject) * userData;
	ret = (*javaEnv)->CallCharMethodA(javaEnv, obj, method, args);
	lua_pushinteger(L, ret);
	return 1;
}

int jCallShortMethod(lua_State * L)
{
	lua_Number stateIndex;
	jshort ret;
	jobject *userData;
	jobject obj;
	jmethodID method;
	jvalue *value;
	JNIEnv *javaEnv;


	int top = lua_gettop(L);
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	jvalue args[top - 2];

	int i;
	for (i = 3; i <= top; i++)
	{
		value = (jvalue *) lua_touserdata(L, i);
		args[i - 3] = (jvalue) * value;
	}

	userData = (jobject *) lua_touserdata(L, 2);
	method = (jmethodID) * userData;
	userData = (jobject *) lua_touserdata(L, 1);
	obj = (jobject) * userData;
	ret = (*javaEnv)->CallShortMethodA(javaEnv, obj, method, args);
	lua_pushinteger(L, ret);
	return 1;
}

int jCallIntMethod(lua_State * L)
{
	lua_Number stateIndex;
	jint ret;
	jobject *userData;
	jobject obj;
	jmethodID method;
	jvalue *value;
	JNIEnv *javaEnv;


	int top = lua_gettop(L);
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	jvalue args[top - 2];

	int i;
	for (i = 3; i <= top; i++)
	{
		value = (jvalue *) lua_touserdata(L, i);
		args[i - 3] = (jvalue) * value;
	}

	userData = (jobject *) lua_touserdata(L, 2);
	method = (jmethodID) * userData;
	userData = (jobject *) lua_touserdata(L, 1);
	obj = (jobject) * userData;
	ret = (*javaEnv)->CallIntMethodA(javaEnv, obj, method, args);
	checkError(javaEnv, L);
	lua_pushinteger(L, ret);
	return 1;
}

int jCallLongMethod(lua_State * L)
{
	lua_Number stateIndex;
	jlong ret;
	jobject *userData;
	jobject obj;
	jmethodID method;
	jvalue *value;
	JNIEnv *javaEnv;


	int top = lua_gettop(L);
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	jvalue args[top - 2];

	int i;
	for (i = 3; i <= top; i++)
	{
		value = (jvalue *) lua_touserdata(L, i);
		args[i - 3] = (jvalue) * value;
	}

	userData = (jobject *) lua_touserdata(L, 2);
	method = (jmethodID) * userData;
	userData = (jobject *) lua_touserdata(L, 1);
	obj = (jobject) * userData;
	ret = (*javaEnv)->CallLongMethodA(javaEnv, obj, method, args);
	lua_pushinteger(L, ret);
	return 1;
}

int jCallFloatMethod(lua_State * L)
{
	lua_Number stateIndex;
	jfloat ret;
	jobject *userData;
	jobject obj;
	jmethodID method;
	jvalue *value;
	JNIEnv *javaEnv;

	int top = lua_gettop(L);
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	jvalue args[top - 2];

	int i;
	for (i = 3; i <= top; i++)
	{
		value = (jvalue *) lua_touserdata(L, i);
		args[i - 3] = (jvalue) * value;
	}

	userData = (jobject *) lua_touserdata(L, 2);
	method = (jmethodID) * userData;
	userData = (jobject *) lua_touserdata(L, 1);
	obj = (jobject) * userData;
	ret = (*javaEnv)->CallFloatMethodA(javaEnv, obj, method, args);
	lua_pushnumber(L,ret);
	return 1;
}

int jCallDoubleMethod(lua_State * L)
{
	lua_Number stateIndex;
	jdouble ret;
	jobject *userData;
	jobject obj;
	jmethodID method;
	jvalue *value;
	JNIEnv *javaEnv;

	int top = lua_gettop(L);
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	jvalue args[top - 2];

	int i;
	for (i = 3; i <= top; i++)
	{
		value = (jvalue *) lua_touserdata(L, i);
		args[i - 3] = (jvalue) * value;
	}

	userData = (jobject *) lua_touserdata(L, 2);
	method = (jmethodID) * userData;
	userData = (jobject *) lua_touserdata(L, 1);
	obj = (jobject) * userData;
	ret = (*javaEnv)->CallDoubleMethodA(javaEnv, obj, method, args);
	lua_pushnumber(L,ret);
	return 1;
}

int jCallObjectMethod(lua_State * L)
{
	lua_Number stateIndex;
	jobject ret;
	jobject *userData;
	jobject obj;
	jmethodID method;
	jvalue *value;
	JNIEnv *javaEnv;

	int top = lua_gettop(L);
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	jvalue args[top - 2];

	int i;
	for (i = 3; i <= top; i++)
	{
		value = (jvalue *) lua_touserdata(L, i);
		args[i - 3] = (jvalue) * value;
	}

	userData = (jobject *) lua_touserdata(L, 2);
	method = (jmethodID) * userData;
	userData = (jobject *) lua_touserdata(L, 1);
	obj = (jobject) * userData;
	ret = (*javaEnv)->CallObjectMethodA(javaEnv, obj, method, args);
	return pushJavaObject(L, ret);
}

int jCallVoidMethod(lua_State * L)
{
	lua_Number stateIndex;
	jobject ret;
	jobject *userData;
	jvalue *value;
	jobject obj;
	jmethodID method;
	JNIEnv *javaEnv;

	int top = lua_gettop(L);
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	jvalue args[top - 2];

	int i;
	for (i = 3; i <= top; i++)
	{
		value = (jvalue *) lua_touserdata(L, i);
		args[i - 3] = (jvalue) * value;
	}

	userData = (jobject *) lua_touserdata(L, 2);
	method = (jmethodID) * userData;
	userData = (jobject *) lua_touserdata(L, 1);
	obj = (jobject) * userData;
	(*javaEnv)->CallVoidMethodA(javaEnv, obj, method, args);
	return 0;
}



int jCallStaticBooleanMethod(lua_State * L)
{
	lua_Number stateIndex;
	jboolean ret;
	jobject *userData;
	jclass clazz;
	jmethodID method;
	jvalue *value;
	JNIEnv *javaEnv;


	int top = lua_gettop(L);
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	jvalue args[top - 2];

	int i;
	for (i = 3; i <= top; i++)
	{
		value = (jvalue *) lua_touserdata(L, i);
		args[i - 3] = (jvalue) * value;
	}

	userData = (jobject *) lua_touserdata(L, 2);
	method = (jmethodID) * userData;
	userData = (jobject *) lua_touserdata(L, 1);
	clazz = (jclass) * userData;
	ret = (*javaEnv)->CallStaticBooleanMethodA(javaEnv, clazz, method, args);
	lua_pushinteger(L, ret);
	return 1;
}

int jCallStaticByteMethod(lua_State * L)
{
	lua_Number stateIndex;
	jbyte ret;
	jobject *userData;
	jclass clazz;
	jmethodID method;
	jvalue *value;
	JNIEnv *javaEnv;


	int top = lua_gettop(L);
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	jvalue args[top - 2];

	int i;
	for (i = 3; i <= top; i++)
	{
		value = (jvalue *) lua_touserdata(L, i);
		args[i - 3] = (jvalue) * value;
	}

	userData = (jobject *) lua_touserdata(L, 2);
	method = (jmethodID) * userData;
	userData = (jobject *) lua_touserdata(L, 1);
	clazz = (jclass) * userData;
	ret = (*javaEnv)->CallStaticByteMethodA(javaEnv, clazz, method, args);
	lua_pushinteger(L, ret);
	return 1;
}

int jCallStaticCharMethod(lua_State * L)
{
	lua_Number stateIndex;
	jchar ret;
	jobject *userData;
	jclass clazz;
	jmethodID method;
	jvalue *value;
	JNIEnv *javaEnv;


	int top = lua_gettop(L);
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	jvalue args[top - 2];

	int i;
	for (i = 3; i <= top; i++)
	{
		value = (jvalue *) lua_touserdata(L, i);
		args[i - 3] = (jvalue) * value;
	}

	userData = (jobject *) lua_touserdata(L, 2);
	method = (jmethodID) * userData;
	userData = (jobject *) lua_touserdata(L, 1);
	clazz = (jclass) * userData;
	ret = (*javaEnv)->CallStaticCharMethodA(javaEnv, clazz, method, args);
	lua_pushinteger(L, ret);
	return 1;
}

int jCallStaticShortMethod(lua_State * L)
{
	lua_Number stateIndex;
	jshort ret;
	jobject *userData;
	jclass clazz;
	jmethodID method;
	jvalue *value;
	JNIEnv *javaEnv;


	int top = lua_gettop(L);
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	jvalue args[top - 2];

	int i;
	for (i = 3; i <= top; i++)
	{
		value = (jvalue *) lua_touserdata(L, i);
		args[i - 3] = (jvalue) * value;
	}

	userData = (jobject *) lua_touserdata(L, 2);
	method = (jmethodID) * userData;
	userData = (jobject *) lua_touserdata(L, 1);
	clazz = (jclass) * userData;
	ret = (*javaEnv)->CallStaticShortMethodA(javaEnv, clazz, method, args);
	lua_pushinteger(L, ret);
	return 1;
}

int jCallStaticIntMethod(lua_State * L)
{
	lua_Number stateIndex;
	jint ret;
	jobject *userData;
	jclass clazz;
	jmethodID method;
	jvalue *value;
	JNIEnv *javaEnv;


	int top = lua_gettop(L);
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	jvalue args[top - 2];

	int i;
	for (i = 3; i <= top; i++)
	{
		value = (jvalue *) lua_touserdata(L, i);
		args[i - 3] = (jvalue) * value;
	}

	userData = (jobject *) lua_touserdata(L, 2);
	method = (jmethodID) * userData;
	userData = (jobject *) lua_touserdata(L, 1);
	clazz = (jclass) * userData;
	ret = (*javaEnv)->CallStaticIntMethodA(javaEnv, clazz, method, args);
	lua_pushinteger(L, ret);
	return 1;
}

int jCallStaticLongMethod(lua_State * L)
{
	lua_Number stateIndex;
	jlong ret;
	jobject *userData;
	jclass clazz;
	jmethodID method;
	jvalue *value;
	JNIEnv *javaEnv;


	int top = lua_gettop(L);
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	jvalue args[top - 2];

	int i;
	for (i = 3; i <= top; i++)
	{
		value = (jvalue *) lua_touserdata(L, i);
		args[i - 3] = (jvalue) * value;
	}

	userData = (jobject *) lua_touserdata(L, 2);
	method = (jmethodID) * userData;
	userData = (jobject *) lua_touserdata(L, 1);
	clazz = (jclass) * userData;
	ret = (*javaEnv)->CallStaticLongMethodA(javaEnv, clazz, method, args);
	lua_pushinteger(L, ret);
	return 1;
}

int jCallStaticFloatMethod(lua_State * L)
{
	lua_Number stateIndex;
	jfloat ret;
	jobject *userData;
	jclass clazz;
	jmethodID method;
	jvalue *value;
	JNIEnv *javaEnv;

	int top = lua_gettop(L);
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	jvalue args[top - 2];

	int i;
	for (i = 3; i <= top; i++)
	{
		value = (jvalue *) lua_touserdata(L, i);
		args[i - 3] = (jvalue) * value;
	}

	userData = (jobject *) lua_touserdata(L, 2);
	method = (jmethodID) * userData;
	userData = (jobject *) lua_touserdata(L, 1);
	clazz = (jclass) * userData;
	ret = (*javaEnv)->CallStaticFloatMethodA(javaEnv, clazz, method, args);
	lua_pushnumber(L,ret);
	return 1;
}

int jCallStaticDoubleMethod(lua_State * L)
{
	lua_Number stateIndex;
	jdouble ret;
	jobject *userData;
	jclass clazz;
	jmethodID method;
	jvalue *value;
	JNIEnv *javaEnv;

	int top = lua_gettop(L);
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	jvalue args[top - 2];

	int i;
	for (i = 3; i <= top; i++)
	{
		value = (jvalue *) lua_touserdata(L, i);
		args[i - 3] = (jvalue) * value;
	}

	userData = (jobject *) lua_touserdata(L, 2);
	method = (jmethodID) * userData;
	userData = (jobject *) lua_touserdata(L, 1);
	clazz = (jclass) * userData;
	ret = (*javaEnv)->CallStaticDoubleMethodA(javaEnv, clazz, method, args);
	lua_pushnumber(L,ret);
	return 1;
}

int jCallStaticObjectMethod(lua_State * L)
{
	lua_Number stateIndex;
	jobject ret;
	jobject *userData;
	jclass clazz;
	jmethodID method;
	jvalue *value;
	JNIEnv *javaEnv;

	int top = lua_gettop(L);
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	jvalue args[top - 2];

	int i;
	for (i = 3; i <= top; i++)
	{
		value = (jvalue *) lua_touserdata(L, i);
		args[i - 3] = (jvalue) * value;
	}

	userData = (jobject *) lua_touserdata(L, 2);
	method = (jmethodID) * userData;
	userData = (jobject *) lua_touserdata(L, 1);
	clazz = (jclass) * userData;
	ret = (*javaEnv)->CallStaticObjectMethodA(javaEnv, clazz, method, args);
	return pushJavaObject(L, ret);
}

int jCallStaticVoidMethod(lua_State * L)
{
	lua_Number stateIndex;
	jobject ret;
	jobject *userData;
	jvalue *value;
	jclass clazz;
	jmethodID method;
	JNIEnv *javaEnv;

	int top = lua_gettop(L);
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	jvalue args[top - 2];

	int i;
	for (i = 3; i <= top; i++)
	{
		value = (jvalue *) lua_touserdata(L, i);
		args[i - 3] = (jvalue) * value;
	}

	userData = (jobject *) lua_touserdata(L, 2);
	method = (jmethodID) * userData;
	userData = (jobject *) lua_touserdata(L, 1);
	clazz = (jclass) * userData;
	(*javaEnv)->CallStaticVoidMethodA(javaEnv, clazz, method, args);
	return 0;
}


int jNewObject(lua_State * L)
{
	lua_Number stateIndex;
	jobject ret;
	jclass *userData;
	jclass clazz;
	jmethodID method;
	jvalue *value;
	JNIEnv *javaEnv;

	int top = lua_gettop(L);
	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	jvalue args[top - 2];

	int i;
	for (i = 3; i <= top; i++)
	{
		value = (jvalue *) lua_touserdata(L, i);
		args[i - 3] = (jvalue) * value;
	}

	userData = (jobject *) lua_touserdata(L, 2);
	method = (jmethodID) * userData;
	userData = (jobject *) lua_touserdata(L, 1);
	clazz = (jclass) * userData;
	ret = (*javaEnv)->NewObjectA(javaEnv, clazz, method, args);
	return pushJavaObject(L, ret);
}

int jIsInstanceOf(lua_State *L) {
  int top;
  jobject *classInstance;
  jclass *clazz;
  JNIEnv *javaEnv;

  top = lua_gettop(L);

  if (top == 0) {
    lua_pushstring(L, "Error. Invalid number of parameters.");
    lua_error(L);
  }

  /* Gets the java Class reference */
  classInstance = checkJavaObject(L, 1);

  clazz = checkJavaObject(L, 2);

  /* Gets the JNI Environment */
  javaEnv = checkEnv(L);

  if ((*javaEnv)->IsInstanceOf(javaEnv, *classInstance, *clazz) == JNI_TRUE) {
    lua_pushboolean(L, 1);
  } else {
    lua_pushboolean(L, 0);
  }
  return 1;
}

/*
int pushJavaObject(lua_State * L, jobject javaObject)
{
	jobject *userData, globalRef;

	JNIEnv *javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}


	globalRef = (*javaEnv)->NewGlobalRef(javaEnv, javaObject);
	(*javaEnv)->DeleteLocalRef(javaEnv, javaObject);

	userData = (jobject *) lua_newuserdata(L, sizeof(jobject));
	*userData = globalRef;

	lua_newtable(L);

	lua_pushstring(L, LUAGCMETAMETHODTAG);
	lua_pushcfunction(L, &gc);
	lua_rawset(L, -3);

	lua_pushstring(L, LUAJAVAOBJECTIND);
	lua_pushboolean(L, 1);
	lua_rawset(L, -3);

	if (lua_setmetatable(L, -2) == 0)
	{
		lua_pushstring(L, "Cannot create proxy to java object.");
		lua_error(L);
	}

	return 1;
}
*/

int jValueD(lua_State * L)
{
	jvalue value;
	jvalue *userData;

	value.d = (jdouble)lua_tonumber(L, 1);
	userData = (jvalue *) lua_newuserdata(L, sizeof(jvalue));
	*userData = value;
	return 1;
}

int jValueF(lua_State * L)
{
	jvalue value;
	jvalue *userData;

	value.f = (jfloat)lua_tonumber(L, 1);
	userData = (jvalue *) lua_newuserdata(L, sizeof(jvalue));
	*userData = value;
	return 1;
}

int jValueJ(lua_State * L)
{
	jvalue value;
	jvalue *userData;

	value.j = (jlong)lua_tointeger(L, 1);
	userData = (jvalue *) lua_newuserdata(L, sizeof(jvalue));
	*userData = value;
	return 1;
}

int jValueI(lua_State * L)
{
	jvalue value;
	jvalue *userData;

	value.i = (jint)lua_tointeger(L, 1);
	userData = (jvalue *) lua_newuserdata(L, sizeof(jvalue));
	*userData = value;
	return 1;
}

int jValueS(lua_State * L)
{
	jvalue value;
	jvalue *userData;

	value.s = (jshort)lua_tointeger(L, 1);
	userData = (jvalue *) lua_newuserdata(L, sizeof(jvalue));
	*userData = value;
	return 1;
}
int jValueC(lua_State * L)
{
	jvalue value;
	jvalue *userData;

	value.c = (jchar)lua_tointeger(L, 1);
	userData = (jvalue *) lua_newuserdata(L, sizeof(jvalue));
	*userData = value;
	return 1;
}

int jValueB(lua_State * L)
{
	jvalue value;
	jvalue *userData;

	value.b = (jbyte)lua_tointeger(L, 1);
	userData = (jvalue *) lua_newuserdata(L, sizeof(jvalue));
	*userData = value;
	return 1;
}

int jValueZ(lua_State * L)
{
	jvalue value;
	jvalue *userData;

	value.z = lua_toboolean(L, 1);
	userData = (jvalue *) lua_newuserdata(L, sizeof(jvalue));
	*userData = value;
	return 1;
}

int jValueL(lua_State * L)
{
	jvalue value;
	jvalue *userData;
    jobject *obj;
	
	obj = (jobject*)lua_touserdata(L,1);
	value.l = (jobject)*obj;
	userData = (jvalue *) lua_newuserdata(L, sizeof(jvalue));
	*userData = value;
	return 1;
}

int jValueT(lua_State * L)
{
	jvalue value;
	jvalue *userData;
	jstring str;
	JNIEnv *javaEnv;
	
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	str = (*javaEnv)->NewStringUTF(javaEnv, lua_tostring(L,1));
	value.l = (*javaEnv)->NewGlobalRef(javaEnv, str);
	(*javaEnv)->DeleteLocalRef(javaEnv, str);
	userData = (jvalue *) lua_newuserdata(L, sizeof(jvalue));
	*userData = value;
	return 1;
}



int _EXPORT luaopen_jni(lua_State * L)
{
	static const struct luaL_reg funcs[] = {
		{"FindClass", jFindClass},
		{"GetStaticMethodID", jGetStaticMethodID},
		{"GetMethodID", jGetMethodID},
		{"CallBooleanMethod", jCallBooleanMethod},
		{"CallByteMethod", jCallByteMethod},
		{"CallCharMethod", jCallCharMethod},
		{"CallShortMethod", jCallShortMethod},
		{"CallIntMethod", jCallIntMethod},
		{"CallLongMethod", jCallLongMethod},
		{"CallFloatMethod", jCallFloatMethod},
		{"CallDoubleMethod", jCallDoubleMethod},
		{"CallObjectMethod", jCallObjectMethod},
		{"CallVoidMethod", jCallVoidMethod},
		{"CallStaticBooleanMethod", jCallStaticBooleanMethod},
		{"CallStaticByteMethod", jCallStaticByteMethod},
		{"CallStaticCharMethod", jCallStaticCharMethod},
		{"CallStaticShortMethod", jCallStaticShortMethod},
		{"CallStaticIntMethod", jCallStaticIntMethod},
		{"CallStaticLongMethod", jCallStaticLongMethod},
		{"CallStaticFloatMethod", jCallStaticFloatMethod},
		{"CallStaticDoubleMethod", jCallStaticDoubleMethod},
		{"CallStaticObjectMethod", jCallStaticObjectMethod},
		{"CallStaticVoidMethod", jCallStaticVoidMethod},
		{"NewObject", jNewObject},
		{"IsInstanceOf", jIsInstanceOf},
		{"jdouble", jValueD},
		{"jfloat", jValueF},
		{"jlong", jValueJ},
		{"jint", jValueI},
		{"jshort", jValueS},
		{"jchar", jValueC},
		{"jbyte", jValueB},
		{"jboolean", jValueZ},
		{"jobject", jValueL},
		{"jstring", jValueT},
		{NULL, NULL}
	};
	luaL_newlib(L, funcs);
	return 1;
}



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

#define  LOG_TAG    "luajni"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

/* Constant that is used to index the JNI Environment */
#define LUAJAVAJNIENVTAG      "_JNIEnv"
/* Defines the lua State Index Property Name */
#define LUAJAVASTATEINDEX     "_LuaJavaStateIndex"
/* Defines wheter the metatable is of a java Object */
#define LUAJAVAOBJECTIND      "__IsJavaObject"
/* Index metamethod name */
#define LUAINDEXMETAMETHODTAG "__index"
/* New index metamethod name */
#define LUANEWINDEXMETAMETHODTAG "__newindex"
/* Garbage collector metamethod name */
#define LUAGCMETAMETHODTAG    "__gc"
/* Call metamethod name */
#define LUACALLMETAMETHODTAG  "__call"
/* Constant that defines where in the metatable should I place the function
   name */
#define LUAJAVAOBJFUNCCALLED  "__FunctionCalled"

static int gc(lua_State * L);
static JNIEnv *getEnvFromState(lua_State * L);
static int isJavaObject(lua_State * L, int idx);
// (*env)->PopLocalFrame(env, NULL);
static jclass throwable_class = NULL;
static jmethodID get_message_method = NULL;


static void checkError(JNIEnv * javaEnv, lua_State * L)
{
	jthrowable exp = (*javaEnv)->ExceptionOccurred(javaEnv);

	/* Handles exception */
	if (exp != NULL)
	{
	(*javaEnv)->ExceptionClear(javaEnv);

	if (throwable_class == NULL)
	{
		jclass tempClass = (*javaEnv)->FindClass(javaEnv, "java/lang/Throwable");

		if (tempClass == NULL)
		{
			fprintf(stderr,
					"Error. Couldn't bind java class java.lang.Throwable\n");
			exit(1);
		}

		throwable_class = (*javaEnv)->NewGlobalRef(javaEnv, tempClass);
		(*javaEnv)->DeleteLocalRef(javaEnv, tempClass);

		if (throwable_class == NULL)
		{
			fprintf(stderr,
					"Error. Couldn't bind java class java.lang.Throwable\n");
			exit(1);
		}
	}

	if (get_message_method == NULL)
	{
		get_message_method =
			(*javaEnv)->GetMethodID(javaEnv, throwable_class, "getMessage",
								"()Ljava/lang/String;");

		if (get_message_method == NULL)
		{
			fprintf(stderr,
					"Could not find <getMessage> method in java.lang.Throwable\n");
			exit(1);
		}
	}

		jobject jstr;
		const char *cStr;

		jstr = (*javaEnv)->CallObjectMethod(javaEnv, exp, get_message_method);

		if (jstr == NULL)
		{
			jmethodID methodId;

			methodId =
				(*javaEnv)->GetMethodID(javaEnv, throwable_class, "toString",
										"()Ljava/lang/String;");
			jstr = (*javaEnv)->CallObjectMethod(javaEnv, exp, methodId);
		}

		cStr = (*javaEnv)->GetStringUTFChars(javaEnv, jstr, NULL);

		lua_pushstring(L, cStr);

		(*javaEnv)->ReleaseStringUTFChars(javaEnv, jstr, cStr);
		( *javaEnv )->DeleteLocalRef( javaEnv , exp );
		( *javaEnv )->DeleteLocalRef( javaEnv , jstr );
		lua_error(L);
	}

}



int jfindClass(lua_State * L)
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
	// jclass globalRef = ( *javaEnv )->NewGlobalRef( javaEnv , clazz );

	// userData = ( jclass * ) lua_newuserdata( L , sizeof( clazz ) );
	// *userData = globalRef;
	// ( *javaEnv )->DeleteLocalRef( javaEnv , clazz );
	// return 1;
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
	ret = (*javaEnv)->CallBooleanMethodA(javaEnv, clazz, method, args);
	lua_pushinteger(L, ret);
	return 1;
}

int jCallByteMethod(lua_State * L)
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
	ret = (*javaEnv)->CallByteMethodA(javaEnv, clazz, method, args);
	lua_pushinteger(L, ret);
	return 1;
}

int jCallCharMethod(lua_State * L)
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
	ret = (*javaEnv)->CallCharMethodA(javaEnv, clazz, method, args);
	lua_pushinteger(L, ret);
	return 1;
}

int jCallShortMethod(lua_State * L)
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
	ret = (*javaEnv)->CallShortMethodA(javaEnv, clazz, method, args);
	lua_pushinteger(L, ret);
	return 1;
}

int jCallIntMethod(lua_State * L)
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
	ret = (*javaEnv)->CallIntMethodA(javaEnv, clazz, method, args);
	lua_pushinteger(L, ret);
	return 1;
}

int jCallLongMethod(lua_State * L)
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
	ret = (*javaEnv)->CallLongMethodA(javaEnv, clazz, method, args);
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

int gc(lua_State * L)
{
	jobject *pObj;
	JNIEnv *javaEnv;

	if (!isJavaObject(L, 1))
	{
		return 0;
	}

	pObj = (jobject *) lua_touserdata(L, 1);

	/* Gets the JNI Environment */
	javaEnv = getEnvFromState(L);
	if (javaEnv == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	(*javaEnv)->DeleteGlobalRef(javaEnv, *pObj);

	return 0;
}

int pushJavaObject(lua_State * L, jobject javaObject)
{
	jobject *userData, globalRef;

	/* Gets the JNI Environment */
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

	/* Creates metatable */
	lua_newtable(L);

	/* pushes the __gc metamethod */
	lua_pushstring(L, LUAGCMETAMETHODTAG);
	lua_pushcfunction(L, &gc);
	lua_rawset(L, -3);

	/* Is Java Object boolean */
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

JNIEnv *getEnvFromState(lua_State * L)
{
	JNIEnv **udEnv;

	lua_pushstring(L, LUAJAVAJNIENVTAG);
	lua_rawget(L, LUA_REGISTRYINDEX);

	if (!lua_isuserdata(L, -1))
	{
		lua_pop(L, 1);
		return NULL;
	}

	udEnv = (JNIEnv **) lua_touserdata(L, -1);

	lua_pop(L, 1);

	return *udEnv;
}

int isJavaObject(lua_State * L, int idx)
{
	if (!lua_isuserdata(L, idx))
		return 0;

	if (lua_getmetatable(L, idx) == 0)
		return 0;

	lua_pushstring(L, LUAJAVAOBJECTIND);
	lua_rawget(L, -2);

	if (lua_isnil(L, -1))
	{
		lua_pop(L, 2);
		return 0;
	}
	lua_pop(L, 2);
	return 1;
}

int jValueD(lua_State * L)
{
	jvalue value;
	jvalue *userData;

	value.d = (double)lua_tonumber(L, 1);
	userData = (jvalue *) lua_newuserdata(L, sizeof(jvalue));
	*userData = value;
	return 1;
}

int jValueF(lua_State * L)
{
	jvalue value;
	jvalue *userData;

	value.f = (float)lua_tonumber(L, 1);
	userData = (jvalue *) lua_newuserdata(L, sizeof(jvalue));
	*userData = value;
	return 1;
}

int jValueJ(lua_State * L)
{
	jvalue value;
	jvalue *userData;

	value.j = (long)lua_tointeger(L, 1);
	userData = (jvalue *) lua_newuserdata(L, sizeof(jvalue));
	*userData = value;
	return 1;
}

int jValueI(lua_State * L)
{
	jvalue value;
	jvalue *userData;

	value.i = (int)lua_tointeger(L, 1);
	userData = (jvalue *) lua_newuserdata(L, sizeof(jvalue));
	*userData = value;
	return 1;
}

int jValueS(lua_State * L)
{
	jvalue value;
	jvalue *userData;

	value.s = (short)lua_tointeger(L, 1);
	userData = (jvalue *) lua_newuserdata(L, sizeof(jvalue));
	*userData = value;
	return 1;
}
int jValueC(lua_State * L)
{
	jvalue value;
	jvalue *userData;

	value.c = (char)lua_tointeger(L, 1);
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
		{"findClass", jfindClass},
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
		{"NewObject", jNewObject},
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




int coding(lua_State * L)
{
	/* Gets t((he JNI Environment */
	JNIEnv *env = getEnvFromState(L);
	if (env == NULL)
	{
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	size_t size = 0;
    const char* str = lua_tolstring(L, 1, &size);
	const char* srcEncoding = luaL_optstring(L,2,"GBK");
	const char* destEncoding = luaL_optstring(L,3,"UTF8");
	
	jbyteArray arr = (*env)->NewByteArray(env,size);
    jbyte* data = (*env)->GetByteArrayElements(env,arr,0);
    memcpy(data, str, size);
    (*env)->ReleaseByteArrayElements(env,arr, data, 0);

    jclass stringClass = (*env)->FindClass(env,"java/lang/String");
    jmethodID ctor = (*env)->GetMethodID(env,stringClass, "<init>", "([BLjava/lang/String;)V");
    jstring strEncoding = (*env)->NewStringUTF(env,srcEncoding);
    jstring newstr = (jstring)(*env)->NewObject(env,stringClass, ctor, arr, strEncoding);
 	(*env)->DeleteLocalRef(env, arr);
    (*env)->DeleteLocalRef(env, strEncoding);

	jmethodID methodGetBytes = (*env)->GetMethodID(env,stringClass, "getBytes", "(Ljava/lang/String;)[B");
    jstring strDestEncoding = (*env)->NewStringUTF(env,destEncoding);
    jbyteArray byteArr = (*env)->CallObjectMethod(env,newstr, methodGetBytes, strDestEncoding);
    (*env)->DeleteLocalRef(env, strDestEncoding);
	(*env)->DeleteLocalRef(env, newstr);

    jbyte* newArr = (*env)->GetByteArrayElements(env,byteArr,0);
    jsize newLen = (*env)->GetArrayLength(env,byteArr);

    lua_pushlstring(L, (const char*)newArr, newLen);
    (*env)->ReleaseByteArrayElements(env,byteArr, newArr, 0);
 	(*env)->DeleteLocalRef(env, byteArr);
    return 1;

}


package com.android.cglib.proxy;

import android.util.Log;

import java.lang.reflect.Method;

public class MethodProxyExecuter {
    public static final String EXECUTE_INTERCEPTOR = "executeInterceptor";
    public static final String EXECUTE_METHOD = "executeMethod";

    @SuppressWarnings({"rawtypes"})
    public static Object executeInterceptor(MethodInterceptor interceptor, Class<?> superClass, String methodName,
                                            Class[] argsType, Object[] argsValue, Object object) {
        if (interceptor == null)
            return executeMethod(superClass, methodName, argsType, argsValue, object);
        try {
            MethodProxy methodProxy = new MethodProxy(superClass, methodName, argsType);
            return interceptor.intercept(object, argsValue, methodProxy);
        } catch (Exception e) {
            throw new ProxyException(e.getMessage());
        }
    }

    @SuppressWarnings({"unchecked", "rawtypes"})
    public static Object executeMethod(Class subClass, String methodName, Class[] argsType, Object[] argsValue, Object object) {
        try {
            Method method = subClass.getMethod(methodName + Const.SUBCLASS_INVOKE_SUPER_SUFFIX, argsType);
            method.setAccessible(true);
            return method.invoke(object, argsValue);
        } catch (Exception e) {
            e.printStackTrace();
            throw new ProxyException(e.getCause());
        }
    }

}

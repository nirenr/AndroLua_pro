package com.android.cglib.proxy;

public interface EnhancerInterface {
	public static final String SET_METHOD_INTERCEPTOR_ENHANCER = "setMethodInterceptor_Enhancer";
	public static final String EXECUTE_SUPER_METHOD_ENHANCER = "executeSuperMethod_Enhancer";
	public void setMethodInterceptor_Enhancer(MethodInterceptor methodInterceptor);
	
	@SuppressWarnings("rawtypes")
	public Object executeSuperMethod_Enhancer(String methodName, Class[] argsType, Object[] argsValue);

}

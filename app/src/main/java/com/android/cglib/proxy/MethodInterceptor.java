package com.android.cglib.proxy;

public interface MethodInterceptor {
	
	public Object intercept(Object object, Object[] args, MethodProxy methodProxy) 
			throws Exception;

}

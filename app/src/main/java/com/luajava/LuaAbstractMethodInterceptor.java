package com.luajava;

import com.android.cglib.proxy.MethodInterceptor;
import com.android.cglib.proxy.MethodProxy;
import com.androlua.LuaContext;

import java.lang.reflect.Method;

/**
 * Created by nirenr on 2018/12/21.
 */

public class LuaAbstractMethodInterceptor implements MethodInterceptor {
    private final LuaContext mContext;
    private LuaObject obj;

    public LuaAbstractMethodInterceptor(LuaObject obj) {
        this.obj = obj;
        mContext = obj.getLuaState().getContext();
    }

    @Override
    public Object intercept(Object object, Object[] args, MethodProxy methodProxy) throws Exception {
        synchronized (obj.L) {
            Method method = methodProxy.getOriginalMethod();
            String methodName = method.getName();
            LuaObject func;
            if (obj.isFunction()) {
                func = obj;
            } else {
                func = obj.getField(methodName);
            }
            Class<?> retType = method.getReturnType();

            if (func.isNil()) {
                if (retType.equals(boolean.class) || retType.equals(Boolean.class))
                    return false;
                else if (retType.isPrimitive() || Number.class.isAssignableFrom(retType))
                    return 0;
                else
                    return null;
            }

            Object ret = null;
            try {
                // Checks if returned type is void. if it is returns null.
                if (retType.equals(Void.class) || retType.equals(void.class)) {
                    func.call(args);
                    ret = null;
                } else {
                    ret = func.call(args);
                    if (ret != null && ret instanceof Double) {
                        ret = LuaState.convertLuaNumber((Double) ret, retType);
                    }
                }
            } catch (LuaException e) {
                mContext.sendError(methodName, e);
            }
            if (ret == null)
                if (retType.equals(boolean.class) || retType.equals(Boolean.class))
                    return false;
                else if (retType.isPrimitive() || Number.class.isAssignableFrom(retType))
                    return 0;
            return ret;
        }
    }
}

package com.android.cglib.proxy;

import java.lang.reflect.Method;

/**
 * Created by nirenr on 2018/12/20.
 */

public interface MethodFilter {
    public boolean filter(Method mode,String name);
}

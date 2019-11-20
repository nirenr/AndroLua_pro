/*
 * $Id: LuaJavaAPI.java,v 1.4 2006/12/22 14:06:40 thiago Exp $
 * Copyright (C) 2003-2007 Kepler Project.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Softwarea.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

package com.luajava;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.util.Log;

import com.android.cglib.proxy.EnhancerInterface;
import com.android.cglib.proxy.MethodFilter;
import com.androlua.LuaBitmap;
import com.androlua.LuaEnhancer;
import com.androlua.LuaGcable;

import java.lang.reflect.Array;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Class that contains functions accessed by lua.
 *
 * @author Thiago Ponte
 */
public final class LuaJavaAPI {
    private final static HashMap<Class<?>, HashMap<String, ArrayList<Method>>> methodCache3 = new HashMap<Class<?>, HashMap<String, ArrayList<Method>>>();
    public final static HashMap<Class<?>, Method[]> methodsMap = new HashMap<Class<?>, Method[]>();
    public final static HashMap<String, Method[]> methodCache = new HashMap<String, Method[]>();
    private static HashMap<String, Method> stringMethodCache = new HashMap<>();
    private static HashMap<String, Method> integerMethodCache = new HashMap<>();
    private static HashMap<String, Method> doubleMethodCache = new HashMap<>();
    private static HashMap<String, Method> boolMethodCache = new HashMap<>();
    private static HashMap<String, Method> voidMethodCache = new HashMap<>();
    private static AtomicInteger sJavaObjectIdx =new AtomicInteger();

    private LuaJavaAPI() {
    }

    public static void clearCaches() {
        methodCache.clear();
        methodsMap.clear();

        stringMethodCache.clear();
        integerMethodCache.clear();
        doubleMethodCache.clear();
        boolMethodCache.clear();
        voidMethodCache.clear();
    }

    /**
     * Java implementation of the metamethod __index
     *
     * @param luaState   int that indicates the state used
     * @param obj        Object to be indexed
     * @param searchName the name of the method
     * @return number of returned objects
     */

    public static int objectIndex(long luaState, Object obj, String searchName, int type)
            throws LuaException {
        LuaState L = LuaStateFactory.getExistingState(luaState);
        synchronized (L) {
            int ret = 0;
            if (type == 0)
                if (checkMethod(L, obj, searchName) != 0)
                    return 2;

            if (type == 0 || type == 1 || type == 5)
                if ((ret = checkField(L, obj, searchName)) != 0)
                    return ret;

            if (type == 0 || type == 3)
                if (checkClass(L, obj, searchName) != 0)
                    return 3;

            if (type == 0 || type == 4)
                if (javaGetter(L, obj, searchName) != 0)
                    return 4;


            if ((type == 0 || type == 6) && obj instanceof LuaMetaTable) {
                Object res = ((LuaMetaTable) obj).__index(searchName);
                L.pushObjectValue(res);
                return 6;
            }

            return 0;
        }
    }

    public static int callMethod(long luaState, Object obj, String cacheName)
            throws LuaException {
        LuaState L = LuaStateFactory.getExistingState(luaState);

        synchronized (L) {
            StringBuilder msgBuilder = new StringBuilder();
            Method method = null;
            int top = L.getTop();
            int methodType = -1;
            if (top == 0) {
                methodType = LuaState.LUA_TNIL;
                method = voidMethodCache.get(cacheName);
                if (method != null) {
                    Object ret;
                    try {
                        ret = method.invoke(obj);
                    } catch (Exception e) {
                        msgBuilder.append("  at ").append(method).append("\n  -> ").append((e.getCause() != null) ? e.getCause() : e).append("\n");
                        throw new LuaException("Invalid method call.\n" + msgBuilder.toString());
                    }
                    // Void function returns null
                    if (ret == null && method.getReturnType().equals(Void.TYPE))
                        return 0;
                    // push result
                    L.pushObjectValue(ret);
                    return 1;
                }
            }

            Object[] objs = new Object[top];

            if (top == 1) {
                switch (L.type(1)) {
                    case LuaState.LUA_TSTRING:
                        methodType = LuaState.LUA_TSTRING;
                        method = stringMethodCache.get(cacheName);
                        if (method != null)
                            objs[0] = L.toString(1);
                        break;
                    case LuaState.LUA_TBOOLEAN:
                        methodType = LuaState.LUA_TBOOLEAN;
                        method = boolMethodCache.get(cacheName);
                        if (method != null)
                            objs[0] = L.toBoolean(1);
                        break;
                    case LuaState.LUA_TNUMBER:
                        if (L.isInteger(1)) {
                            methodType = LuaState.LUA_TINTEGER;
                            method = integerMethodCache.get(cacheName);
                            if (method != null)
                                objs[0] = LuaState.convertLuaNumber(L.toInteger(1), method.getParameterTypes()[0]);
                        } else {
                            methodType = LuaState.LUA_TNUMBER;
                            method = doubleMethodCache.get(cacheName);
                            if (method != null)
                                objs[0] = LuaState.convertLuaNumber(L.toNumber(1), method.getParameterTypes()[0]);

                        }
                        break;
                }
                if (method != null) {
                    Object ret;
                    try {
                        if (!Modifier.isPublic(method.getModifiers()))
                            method.setAccessible(true);

                        ret = method.invoke(obj, objs);
                    } catch (Exception e) {
                        msgBuilder.append("  at ").append(method).append("\n  -> ").append((e.getCause() != null) ? e.getCause() : e).append("\n");
                        throw new LuaException("Invalid method call.\n" + msgBuilder.toString());
                    }

                    // Void function returns null
                    if (ret == null && method.getReturnType().equals(Void.TYPE))
                        return 0;

                    // push result
                    L.pushObjectValue(ret);
                    return 1;
                }
            }

            Method[] methods = methodCache.get(cacheName);
            int[] type = new int[top];
            for (int i = 0; i < top; i++) {
                type[i] = L.type(i + 1);
            }
            // gets method and arguments
            for (Method m : methods) {

                Class[] parameters = m.getParameterTypes();
                if (parameters.length != top)
                    continue;

                boolean okMethod = true;

                for (int j = 0; j < parameters.length; j++) {
                    try {
                        objs[j] = compareTypes(L, parameters[j], type[j], j + 1);
                    } catch (Exception e) {
                        okMethod = false;
                        break;
                    }
                }

                if (okMethod) {
                    method = m;
                    Object ret;
                    try {
                        if (!Modifier.isPublic(method.getModifiers()))
                            method.setAccessible(true);

                        ret = method.invoke(obj, objs);
                    } catch (Exception e) {
                        msgBuilder.append("  at ").append(method).append("\n  -> ").append((e.getCause() != null) ? e.getCause() : e).append("\n");
                        continue;
                    }

                    switch (methodType) {
                        case LuaState.LUA_TSTRING:
                            stringMethodCache.put(cacheName, method);
                            break;
                        case LuaState.LUA_TINTEGER:
                            integerMethodCache.put(cacheName, method);
                            break;
                        case LuaState.LUA_TNUMBER:
                            doubleMethodCache.put(cacheName, method);
                            break;
                        case LuaState.LUA_TBOOLEAN:
                            boolMethodCache.put(cacheName, method);
                            break;
                        case LuaState.LUA_TNIL:
                            voidMethodCache.put(cacheName, method);
                            break;
                    }
                    // Void function returns null
                    if (ret == null && method.getReturnType().equals(Void.TYPE))
                        return 0;

                    // push result
                    L.pushObjectValue(ret);
                    return 1;
                }
            }

            if (msgBuilder.length() > 0) {
                throw new LuaException("Invalid method call.\n" + msgBuilder.toString());
            }
            // If method is null means there isn't one receiving the given arguments
            for (Method m : methods) {
                msgBuilder.append(m.toString());
                msgBuilder.append("\n");
            }
            throw new LuaException("Invalid method call. Invalid Parameters.\n" + msgBuilder.toString());

        }
    }

    /**
     * Java implementation of the metamethod __newindex
     *
     * @param luaState   int that indicates the state used
     * @param obj        Object to be indexed
     * @param searchName the name of the method
     * @return number of returned objects
     */

    public static int objectNewIndex(long luaState, Object obj, String searchName, int type)
            throws LuaException {
        LuaState L = LuaStateFactory.getExistingState(luaState);
        synchronized (L) {
            int res;
            if (type == 0 || type == 1) {
                res = setFieldValue(L, obj, searchName);
                if (res != 0)
                    return 1;
            }

            if (type == 0 || type == 2) {
                res = javaSetter(L, obj, searchName);
                if (res != 0)
                    return 2;
            }
            if (type == 0 || type == 3) {
                if (obj instanceof LuaMetaTable) {
                    ((LuaMetaTable) obj).__newIndex(searchName, L.toJavaObject(-1));
                    return 3;
                }
            }
            return 0;
        }
    }

    public static int setFieldValue(LuaState L, Object obj, String fieldName) throws LuaException {
        synchronized (L) {
            Field field = null;
            Class objClass;
            boolean isClass = false;

            if (obj == null)
                return 0;

            if (obj instanceof Class) {
                objClass = (Class) obj;
                isClass = true;
            } else {
                objClass = obj.getClass();
            }

            try {
                field = objClass.getField(fieldName);
            } catch (NoSuchFieldException e) {
                return 0;
            }

            if (field == null)
                return 0;
            if (isClass && !Modifier.isStatic(field.getModifiers()))
                return 0;
            Class type = field.getType();
            try {
                if (!Modifier.isPublic(field.getModifiers()))
                    field.setAccessible(true);

                field.set(obj, compareTypes(L, type, L.getTop()));
            } catch (LuaException e) {
                argError(L, fieldName, -1, type);
            } catch (Exception e) {
                throw new LuaException(e);
            }

            return 1;
        }
    }

    private static String argError(LuaState L, String name, int idx, Class type) throws LuaException {
        throw new LuaException("bad argument to '" + name + "' (" + type.getName() + " expected, got " + typeName(L, idx) + " value)");

    }

    private static String argError(LuaState L, String name, int idx, String type) throws LuaException {
        throw new LuaException("bad argument #" +idx+
                " to '" + name + "' (" + type + " expected, got " + typeName(L, idx+1) + " value)");

    }

    private static String typeName(LuaState L, int idx) throws LuaException {
        if (L.isObject(idx)) {
            return L.getObjectFromUserdata(idx).getClass().getName();
        }
        switch (L.type(idx)) {
            case LuaState.LUA_TSTRING:
                return "string";
            case LuaState.LUA_TNUMBER:
                return "number";
            case LuaState.LUA_TBOOLEAN:
                return "boolean";
            case LuaState.LUA_TFUNCTION:
                return "function";
            case LuaState.LUA_TTABLE:
                return "table";
            case LuaState.LUA_TTHREAD:
                return "thread";
            case LuaState.LUA_TLIGHTUSERDATA:
            case LuaState.LUA_TUSERDATA:
                return "userdata";
        }
        return "unkown";
    }

    /**
     * Java implementation of the metamethod __index
     *
     * @param luaState int that indicates the state used
     * @param obj      Object to be indexed
     * @param index    the Array index
     * @return number of returned objects
     */
    public static int setArrayValue(long luaState, Object obj, int index) throws LuaException {
        LuaState L = LuaStateFactory.getExistingState(luaState);

        synchronized (L) {
            if (obj.getClass().isArray()) {
                Class<?> type = obj.getClass().getComponentType();
                try {
                    Object value = compareTypes(L, type, 3);
                    Array.set(obj, index, value);
                } catch (LuaException e) {
                    argError(L, obj.getClass().getName() + " [" + index + "]", 3, type);
                }
            } else if (obj instanceof List) {
                ((List<Object>) obj).set(index, L.toJavaObject(3));
            } else if (obj instanceof Map) {
                ((Map<Long, Object>) obj).put((long) index, L.toJavaObject(3));
            } else {
                throw new LuaException("can not set " + obj.getClass().getName() + " value: " + L.toJavaObject(3) + " in " + index);
            }
            return 0;
        }
    }

    public static int setArrayValue(LuaState L, Object obj, int index) throws LuaException {

        synchronized (L) {
            if (obj.getClass().isArray()) {
                Class<?> type = obj.getClass().getComponentType();
                try {
                    Object value = compareTypes(L, type, -1);
                    Array.set(obj, index, value);
                } catch (LuaException e) {
                    argError(L, obj.getClass().getName() + " [" + index + "]", 3, type);
                }
            } else if (obj instanceof List) {
                ((List<Object>) obj).set(index, L.toJavaObject(-1));
            } else if (obj instanceof Map) {
                ((Map<Long, Object>) obj).put((long) index, L.toJavaObject(-1));
            } else {
                throw new LuaException("can not set " + obj.getClass().getName() + " value: " + L.toJavaObject(-1) + " in " + index);
            }
            return 0;
        }
    }

    public static int getArrayValue(long luaState, Object obj, int index) throws LuaException {
        LuaState L = LuaStateFactory.getExistingState(luaState);

        synchronized (L) {
            Object ret = null;
            if (obj.getClass().isArray()) {
                ret = Array.get(obj, index);
            } else if (obj instanceof List) {
                ret = ((List) obj).get(index);
            } else if (obj instanceof Map) {
                ret = ((Map) obj).get((long) index);
            } else {
                throw new LuaException("can not get " + obj.getClass().getName() + " value in " + index);
            }
            L.pushObjectValue(ret);
            return 1;
        }
    }

    public static int asTable(long luaState, Object obj) throws LuaException {
        LuaState L = LuaStateFactory.getExistingState(luaState);

        synchronized (L) {
            if (L.isBoolean(-1) && L.toBoolean(-1)) {
                L.pop(1);
                return asTable(L, obj);
            }
            try {
                L.newTable();
                if (obj.getClass().isArray()) {
                    int n = Array.getLength(obj);
                    for (int i = 0; i <= n - 1; i++) {
                        Object v = Array.get(obj, i);
                        L.pushObjectValue(v);
                        L.rawSetI(-2, i + 1);
                    }
                } else if (obj instanceof Collection) {
                    Collection list = (Collection) obj;
                    int i = 1;
                    for (Object v : list) {
                        L.pushObjectValue(v);
                        L.rawSetI(-2, i++);
                    }
                } else if (obj instanceof Map) {
                    Map map = (Map) obj;
                    for (Object o : map.entrySet()) {
                        Map.Entry entry = (Map.Entry) o;
                        L.pushObjectValue(entry.getKey());
                        L.pushObjectValue(entry.getValue());
                        L.setTable(-3);
                    }
                }
                L.pushValue(-1);
                return 1;
            } catch (Exception e) {
                throw new LuaException("can not astable: " + e.getMessage());
            }

        }
    }

    private static int asTable(LuaState L, Object obj) throws LuaException {
        synchronized (L) {
            try {
                L.newTable();
                if (obj.getClass().isArray()) {
                    int n = Array.getLength(obj);
                    for (int i = 0; i <= n - 1; i++) {
                        asTable(L, Array.get(obj, i));
                        L.rawSetI(-2, i + 1);
                    }
                } else if (obj instanceof Collection) {
                    Collection list = (Collection) obj;
                    int i = 1;
                    for (Object v : list) {
                        asTable(L, v);
                        L.rawSetI(-2, i++);
                    }
                } else if (obj instanceof Map) {
                    Map map = (Map) obj;
                    for (Object o : map.entrySet()) {
                        Map.Entry entry = (Map.Entry) o;
                        L.pushObjectValue(entry.getKey());
                        asTable(L, entry.getValue());
                        L.setTable(-3);
                    }
                } else {
                    L.pop(1);
                    L.pushObjectValue(obj);
                }
                return 1;
            } catch (Exception e) {
                throw new LuaException("can not astable: " + e.getMessage());
            }

        }
    }


    public static int newArray(long luaState, Class<?> clazz, int size) throws LuaException {
        LuaState L = LuaStateFactory.getExistingState(luaState);
        synchronized (L) {
            try {
                Object obj = Array.newInstance(clazz, size);
                L.pushJavaObject(obj);
            } catch (Exception e) {
                throw new LuaException("can not create a array: " + e.getMessage());
            }
            return 1;
        }
    }

    public static int newArray(long luaState, Class<?> clazz) throws LuaException {
        LuaState L = LuaStateFactory.getExistingState(luaState);
        synchronized (L) {
            try {
                int top = L.getTop();
                int[] dimensions = new int[top - 1];
                for (int i = 0; i < top - 1; i++) {
                    dimensions[i] = (int) L.toInteger(i + 2);
                }
                Object obj = Array.newInstance(clazz, dimensions);
                L.pushJavaObject(obj);
            } catch (Exception e) {
                throw new LuaException("can not create a array: " + e.getMessage());
            }
            return 1;
        }
    }

    public static Class javaBindClass(String className) throws LuaException {
        Class clazz;
        try {
            clazz = Class.forName(className);
        } catch (Exception e) {
            switch (className) {
                case "boolean":
                    clazz = Boolean.TYPE;
                    break;
                case "byte":
                    clazz = Byte.TYPE;
                    break;
                case "char":
                    clazz = Character.TYPE;
                    break;
                case "short":
                    clazz = Short.TYPE;
                    break;
                case "int":
                    clazz = Integer.TYPE;
                    break;
                case "long":
                    clazz = Long.TYPE;
                    break;
                case "float":
                    clazz = Float.TYPE;
                    break;
                case "double":
                    clazz = Double.TYPE;
                    break;
                default:
                    throw new LuaException("Class not found: " + className);
            }
        }
        return clazz;
    }

    /**
     * Pushes a new instance of a java Object of the type className
     *
     * @param luaState  int that represents the state to be used
     * @param className name of the class
     * @return number of returned objects
     * @throws LuaException
     */
    public static int javaNewInstance(long luaState, String className) throws LuaException {
        LuaState L = LuaStateFactory.getExistingState(luaState);

        synchronized (L) {
            Class clazz;
            clazz = javaBindClass(className);
            if (clazz.isPrimitive())
                return toPrimitive(L, clazz, -1);
            else
                return getObjInstance(L, clazz);
        }
    }

    /**
     * javaNew returns a new instance of a given clazz
     *
     * @param luaState int that represents the state to be used
     * @param clazz    class to be instanciated
     * @return number of returned objects
     * @throws LuaException
     */
    public static int javaNew(long luaState, Class<?> clazz) throws LuaException {
        LuaState L = LuaStateFactory.getExistingState(luaState);

        synchronized (L) {
            if (clazz.isPrimitive()) {
                int top = L.getTop();
                for (int i = 2; i <= top; i++) {
                    toPrimitive(L, clazz, i);
                }
                return top - 1;
            } else if ((clazz.getModifiers() & Modifier.ABSTRACT) != 0) {
                if (!L.isTable(2))
                    argError(L, "javaOverride", 1, "table");
                return javaOverride(luaState, clazz);
            } else {
                return getObjInstance(L, clazz);
            }
        }
    }

    public static int javaOverride(long luaState, Class<?> clazz) throws LuaException {
        LuaState L = LuaStateFactory.getExistingState(luaState);

        synchronized (L) {
            final LuaTable<String, LuaFunction> t = new LuaTable<String, LuaFunction>(L, 2);
            L.remove(2);
            Class<?> cls = new LuaEnhancer(clazz).create(new MethodFilter() {
                @Override
                public boolean filter(Method method, String name) {
                    if (t.containsKey(name))
                        return true;
                    return false;
                }
            });
            int r = getObjInstance(L, cls);
            if (r == 0)
                return 0;
            EnhancerInterface obj = (EnhancerInterface) L.toJavaObject(-1);
            obj.setMethodInterceptor_Enhancer(new LuaMethodInterceptor(t));
            L.pushJavaObject(obj);
            return 1;
        }
    }

    public static int javaCreate(long luaState, Class<?> clazz) throws LuaException {
        LuaState L = LuaStateFactory.getExistingState(luaState);

        synchronized (L) {
            if (clazz.isPrimitive() || clazz == String.class) {
                return createArray(L, clazz);
            } else if (clazz.isArray()) {
                return createArray(L, clazz);
            } else if (List.class.isAssignableFrom(clazz)) {
                return createList(L, clazz);
            } else if (Map.class.isAssignableFrom(clazz)) {
                return createMap(L, clazz);
            } else if (clazz.isInterface()) {
                return createProxyObject(L, clazz);
            } else if ((clazz.getModifiers() & Modifier.ABSTRACT) != 0) {
                return createAbstractProxy(L, clazz);
            } else {
                if (L.objLen(-1) == 0) {
                    return createArray(L, clazz);
                } else {
                    L.getI(-1, 1);
                    Object o = L.toJavaObject(-1);
                    L.pop(1);
                    if (clazz.isAssignableFrom(o.getClass()))
                        return createArray(L, clazz);
                    else
                        return getObjInstance(L, clazz);
                }

            }
        }

    }

    private static int createAbstractProxy(LuaState L, Class<?> clazz) {
        Class<?> cls = new LuaEnhancer(clazz).create(new MethodFilter() {
            @Override
            public boolean filter(Method method, String name) {
                if ((method.getModifiers() & Modifier.ABSTRACT) == 0)
                    return true;
                return false;
            }
        });
        try {
            EnhancerInterface obj = (EnhancerInterface) cls.newInstance();
            obj.setMethodInterceptor_Enhancer(new LuaAbstractMethodInterceptor(L.getLuaObject(-1)));
            L.pushJavaObject(obj);
            return 1;
        } catch (Exception e) {
            e.printStackTrace();
        }
        return 0;
    }

    public static int objectCall(long luaState, Object obj) throws LuaException {
        LuaState L = LuaStateFactory.getExistingState(luaState);

        synchronized (L) {
            if (obj instanceof LuaMetaTable) {
                int n = L.getTop();
                Object[] args = new Object[n - 1];
                for (int i = 2; i <= n; i++) {
                    args[i - 2] = L.toJavaObject(i);
                }
                Object ret = ((LuaMetaTable) obj).__call(args);
                L.pushObjectValue(ret);
                return 1;
            } else {
                if (L.isTable(2)) {
                    if (obj.getClass().isArray() && Array.getLength(obj) == 0)
                        return createArray(L, obj.getClass());
                    L.pushNil();
                    if (obj instanceof List) {
                        List list = (List) obj;
                        while (L.next(2) != 0) {
                            list.add(L.toJavaObject(-1));
                            L.pop(1);
                        }
                    } else {
                        while (L.next(2) != 0) {
                            if (L.isNumber(-2))
                                setArrayValue(L, obj, (int) L.toInteger(-2));
                            else
                                javaSetter(L, obj, L.toString(-2));
                            L.pop(1);
                        }
                    }
                    L.setTop(1);
                    return 1;
                } else {
                    return 0;
                }
            }
        }
    }

    /**
     * Function that creates an object proxy and pushes it into the stack
     *
     * @param luaState int that represents the state to be used
     * @param implem   interfaces implemented separated by comma (<code>,</code>)
     * @return number of returned objects
     * @throws LuaException
     */
    public static int createProxy(long luaState, String implem)
            throws LuaException {
        LuaState L = LuaStateFactory.getExistingState(luaState);
        synchronized (L) {
            return createProxyObject(L, implem);
        }
    }

    public static int createArray(long luaState, String className)
            throws LuaException {
        LuaState L = LuaStateFactory.getExistingState(luaState);
        synchronized (L) {
            Class type = javaBindClass(className);
            return createArray(L, type);
        }
    }

    /**
     * Calls the static method <code>methodName</code> in class <code>className</code>
     * that receives a LuaState as first parameter.
     *
     * @param luaState   int that represents the state to be used
     * @param className  name of the class that has the open library method
     * @param methodName method to open library
     * @return number of returned objects
     * @throws LuaException
     */
    public static int javaLoadLib(long luaState, String className, String methodName)
            throws LuaException {
        LuaState L = LuaStateFactory.getExistingState(luaState);

        synchronized (L) {
            Class<?> clazz;
            try {
                clazz = Class.forName(className);
            } catch (ClassNotFoundException e) {
                throw new LuaException(e);
            }

            try {
                Method mt = clazz.getMethod(methodName, LuaState.class);
                Object obj = mt.invoke(null, L);

                if (obj != null && obj instanceof Integer) {
                    return (Integer) obj;
                } else
                    return 0;
            } catch (Exception e) {
                throw new LuaException("Error on calling method. Library could not be loaded. " + e.getMessage());
            }
        }
    }

    public static int javaToString(long luaState, Object obj) throws LuaException {
        LuaState L = LuaStateFactory.getExistingState(luaState);

        synchronized (L) {
            if (obj == null)
                L.pushString("null");
            else
                L.pushString(obj.toString());
            return 1;
        }
    }

    public static void javaGc(Object obj) throws LuaException {
        Log.i("javaGc: ", obj + "");
        if (obj == null)
            return;
        sJavaObjectIdx.addAndGet(-1);
    }

    public static void javaClose(Object obj) throws LuaException {
        Log.i("javaClose: ", obj + "");
        if (obj == null)
            return;
        sJavaObjectIdx.addAndGet(-1);
        try {
            if (obj instanceof LuaGcable)
                ((LuaGcable) obj).gc();
            else if (obj instanceof Bitmap) {
                LuaBitmap.removeBitmap((Bitmap) obj);
                ((Bitmap) obj).recycle();
            } else if (obj instanceof BitmapDrawable)
                ((BitmapDrawable) obj).getBitmap().recycle();
            else if (obj instanceof AutoCloseable)
                ((AutoCloseable) obj).close();

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    public static int javaGetType(long luaState, Object obj) throws LuaException {
        LuaState L = LuaStateFactory.getExistingState(luaState);

        synchronized (L) {
            if (obj == null)
                L.pushString("null");
            else
                L.pushString(obj.getClass().getName());
            return 1;
        }
    }

    public static int javaEquals(long luaState, Object obj, Object obj2) throws LuaException {
        LuaState L = LuaStateFactory.getExistingState(luaState);

        synchronized (L) {
            boolean eq = obj.equals(obj2);
            L.pushBoolean(eq);
            return 1;
        }
    }

    public static int javaObjectLength(long luaState, Object obj) throws LuaException {
        LuaState L = LuaStateFactory.getExistingState(luaState);

        synchronized (L) {
            int ret;
            try {
                if (obj instanceof CharSequence)
                    ret = ((CharSequence) obj).length();
                else if (obj instanceof Collection)
                    ret = ((Collection) obj).size();
                else if (obj instanceof Map)
                    ret = ((Map) obj).size();
                else
                    ret = Array.getLength(obj);
            } catch (Exception e) {
                throw new LuaException(e);
            }

            L.pushInteger(ret);

            return 1;
        }
    }

    private static int getObjInstance(LuaState L, Class<?> clazz) throws LuaException {
        synchronized (L) {
            int top = L.getTop();
            if (top == 1) {
                try {
                    Object ret = clazz.newInstance();
                    L.pushJavaObject(ret);
                    return 1;
                } catch (Exception e) {
                    try {
                        Constructor<?> ctr = clazz.getConstructor(Context.class);
                        Object ret = ctr.newInstance(L.getContext().getContext());
                        L.pushJavaObject(ret);
                        return 1;
                    } catch (Exception ignored) {
                    }
                }
            }
            Object[] objs = new Object[top - 1];

            Constructor[] constructors = clazz.getConstructors();
            Constructor constructor = null;

            StringBuilder msgBuilder = new StringBuilder();
            // gets method and arguments
            for (Constructor c : constructors) {
                Class<?>[] parameters = c.getParameterTypes();
                if (parameters.length != top - 1)
                    continue;

                boolean okConstructor = true;

                for (int j = 0; j < parameters.length; j++) {
                    try {
                        objs[j] = compareTypes(L, parameters[j], j + 2);
                    } catch (Exception e) {
                        okConstructor = false;
                        break;
                    }
                }

                if (okConstructor) {
                    constructor = c;
                    Object ret;
                    try {
                        ret = constructor.newInstance(objs);
                    } catch (Exception e) {
                        msgBuilder.append("  at ").append(constructor).append("\n  -> ").append((e.getCause() != null) ? e.getCause() : e).append("\n");
                        continue;
                    }
                    L.pushJavaObject(ret);
                    return 1;
                    //break;
                }
            }

            if (msgBuilder.length() > 0) {
                throw new LuaException("Invalid constructor method call.\n" + msgBuilder.toString());
            }

            for (Constructor c : constructors) {
                msgBuilder.append(c.toString());
                msgBuilder.append("\n");
            }

            throw new LuaException("Invalid constructor method call. Invalid Parameters.\n" + msgBuilder.toString());

            /*// If method is null means there isn't one receiving the given arguments
            if (constructor == null) {
                StringBuilder msgBuilder = new StringBuilder();
                for (Constructor c : constructors) {
                    msgBuilder.append(c.toString());
                    msgBuilder.append("\n");
                }
                throw new LuaException("Invalid constructor method call. Invalid Parameters.\n" + msgBuilder.toString());
            }

            Object ret;
            try {
                ret = constructor.newInstance(objs);
            } catch (Exception e) {
                throw new LuaException(e);
            }

            if (ret == null) {
                throw new LuaException("Couldn't instantiate java Object");
            }
            L.pushJavaObject(ret);
            return 1;*/
        }
    }

    public static int getContext(long luaState) throws LuaException {
        LuaState L = LuaStateFactory.getExistingState(luaState);
        synchronized (L) {
            L.pushJavaObject(L.getContext());
        }
        return 1;
    }

    public static ArrayList<Method> getMethod(Class<?> clazz, String methodName, boolean isClass) {
        //String className = clazz.getName();
        HashMap<String, ArrayList<Method>> cList = methodCache3.get(clazz);
        if (cList == null) {
            cList = new HashMap<>();
            methodCache3.put(clazz, cList);
        }

        ArrayList<Method> mlist = cList.get(methodName);
        if (mlist == null) {
            Method[] methods = methodsMap.get(clazz);
            if (methods == null) {
                methods = clazz.getMethods();
                methodsMap.put(clazz, methods);
            }
            for (Method method : methods) {
                String name = method.getName();
                ArrayList<Method> list = cList.get(name);
                if (list == null) {
                    list = new ArrayList<Method>();
                    cList.put(name, list);
                }
                list.add(method);
            }
            mlist = cList.get(methodName);
        }

        if (mlist == null) {
            mlist = new ArrayList<Method>();
        }
        if (isClass) {
            ArrayList<Method> slist = new ArrayList<Method>();
            for (Method m : mlist) {
                if (Modifier.isStatic(m.getModifiers()))
                    slist.add(m);
            }

            if (slist.isEmpty()) {
                slist = getMethod(Class.class, methodName, false);
            }
            return slist;
        }
        return mlist;
    }


    /**
     * Checks if there is a field on the obj with the given name
     *
     * @param L         int that represents the state to be used
     * @param obj       object to be inspected
     * @param fieldName name of the field to be inpected
     * @return number of returned objects
     */
    public static int checkField(LuaState L, Object obj, String fieldName)
            throws LuaException {
        synchronized (L) {
            Field field = null;
            Class objClass;
            boolean isClass = false;

            if (obj instanceof Class) {
                objClass = (Class) obj;
                isClass = true;
            } else {
                objClass = obj.getClass();
            }

            try {
                field = objClass.getField(fieldName);
            } catch (NoSuchFieldException e) {
                return 0;
            }

            if (field == null)
                return 0;

            if (isClass && !Modifier.isStatic(field.getModifiers()))
                return 0;

            Object ret = null;
            try {
                if (!Modifier.isPublic(field.getModifiers()))
                    field.setAccessible(true);
                ret = field.get(obj);
            } catch (Exception e) {
                throw new LuaException(e);
            }

            L.pushObjectValue(ret);
            if (Modifier.isFinal(field.getModifiers()))
                return 5;
            else
                return 1;
        }
    }


    /**
     * Checks to see if there is a method with the given name.
     *
     * @param L          int that represents the state to be used
     * @param obj        object to be inspected
     * @param methodName name of the field to be inpected
     * @return number of returned objects
     */
    public static int checkMethod(LuaState L, Object obj, String methodName) throws LuaException {
        synchronized (L) {
            Class<?> clazz;
            boolean isClass = false;
            if (obj instanceof Class) {
                clazz = (Class<?>) obj;
                isClass = true;
            } else {
                clazz = obj.getClass();
            }
            //String className=clazz.getName();
            String cacheName = L.toString(-1);
            Method[] mlist = methodCache.get(cacheName);
            if (mlist == null) {
                ArrayList<Method> list = getMethod(clazz, methodName, isClass);
                mlist = new Method[list.size()];
                list.toArray(mlist);
                methodCache.put(cacheName, mlist);
            }
            if (mlist.length == 0)
                return 0;
            return 2;
        }
    }

    /**
     * Checks to see if there is a class with the given name.
     *
     * @param L         int that represents the state to be used
     * @param obj       object to be inspected
     * @param className name of the field to be inpected
     * @return number of returned objects
     */
    public static int checkClass(LuaState L, Object obj, String className) throws LuaException {
        synchronized (L) {
            Class clazz;

            if (obj instanceof Class) {
                clazz = (Class) obj;
            } else {
                return 0;
            }

            Class[] clazzes = clazz.getClasses();

            for (Class c : clazzes) {
                if (c.getSimpleName().equals(className)) {
                    L.pushJavaObject(c);
                    return 3;
                }
            }
            return 0;
        }
    }

    public static int javaGetter(LuaState L, Object obj, String methodName) throws LuaException {
        synchronized (L) {
            Class<?> clazz;

            Method method = null;
            boolean isClass = false;
            if (obj instanceof Map) {
                Map map = (Map) obj;
                L.pushObjectValue(map.get(methodName));
                return 1;
            } else if (obj instanceof Class) {
                clazz = (Class) obj;
                isClass = true;
            } else {
                clazz = obj.getClass();
            }
            char c = methodName.charAt(0);
            if (Character.isLowerCase(c)) {
                methodName = Character.toUpperCase(c) + methodName.substring(1);
            }
            String cacheName = clazz.toString() + "@<-" + methodName;
            if (!isClass)
                method = voidMethodCache.get(cacheName);
            if (method == null) {
                try {
                    method = clazz.getMethod("get" + methodName);
                } catch (NoSuchMethodException e) {
                    try {
                        method = clazz.getMethod("is" + methodName);
                    } catch (NoSuchMethodException e1) {
                        return 0;
                    }
                }
                if (isClass && !Modifier.isStatic(method.getModifiers()))
                    return 0;
                voidMethodCache.put(cacheName, method);
            }

            Object ret;
            try {
                ret = method.invoke(obj);
            } catch (Exception e) {
                throw new LuaException(e);
            }

            if (ret instanceof CharSequence)
                L.pushString(ret.toString());
            else
                L.pushObjectValue(ret);
            return 1;
        }
    }

    public static int javaSetter(LuaState L, Object obj, String methodName, Object value) throws LuaException {
        L.pushObjectValue(value);
        int ret = javaSetter(L, obj, methodName);
        L.pop(1);
        return ret;
    }

    public static int javaSetter(LuaState L, Object obj, String methodName) throws LuaException {
        synchronized (L) {
            Class clazz;
            boolean isClass = false;

            if (obj instanceof Map) {
                Map map = (Map) obj;
                map.put(methodName, L.toJavaObject(-1));
                return 1;
            } else if (obj instanceof Class) {
                clazz = (Class) obj;
                isClass = true;
            } else {
                clazz = obj.getClass();
            }

            if (methodName.length() > 2 && methodName.substring(0, 2).equals("on") && L.type(-1) == LuaState.LUA_TFUNCTION)
                return javaSetListener(L, obj, methodName, isClass);

            int ret = javaSetMethod(L, obj, methodName, isClass);
            if (ret != 0)
                return ret;
            return setDeclaredFieldValue(L, obj, methodName);
        }
    }

    private static int setDeclaredFieldValue(LuaState L, Object obj, String fieldName) throws LuaException {
        synchronized (L) {
            Field field = null;
            Class objClass;
            boolean isClass = false;

            if (obj == null)
                return 0;

            if (obj instanceof Class) {
                objClass = (Class) obj;
                isClass = true;
            } else {
                objClass = obj.getClass();
            }

            String name = null;
            if (!fieldName.startsWith("m")) {
                char c = fieldName.charAt(0);
                if (Character.isLowerCase(c)) {
                    name = Character.toUpperCase(c) + fieldName.substring(1);
                }
                name = "m" + name;
            }

            while (objClass != null) {
                try {
                    field = objClass.getDeclaredField(fieldName);
                } catch (NoSuchFieldException e) {
                    try {
                        if (name != null)
                            field = objClass.getDeclaredField(name);
                    } catch (NoSuchFieldException ignored) {
                    }
                }
                if (field != null)
                    break;
                objClass = objClass.getSuperclass();
            }

            if (field == null)
                return 0;
            if (isClass && !Modifier.isStatic(field.getModifiers()))
                return 0;
            Class type = field.getType();
            try {
                if (!Modifier.isPublic(field.getModifiers()))
                    field.setAccessible(true);

                field.set(obj, compareTypes(L, type, L.getTop()));
            } catch (LuaException e) {
                argError(L, fieldName, 3, type);
            } catch (Exception e) {
                throw new LuaException(e);
            }

            return 1;
        }
    }


    private static int javaSetListener(LuaState L, Object obj, String methodName, boolean isClass) throws LuaException {
        synchronized (L) {
            String name = "setOn" + methodName.substring(2) + "Listener";
            ArrayList<Method> methods = getMethod(obj.getClass(), name, isClass);
            for (Method m : methods) {
                if (isClass && !Modifier.isStatic(m.getModifiers()))
                    continue;

                Class<?>[] tp = m.getParameterTypes();
                if (tp.length == 1 && tp[0].isInterface()) {
                    L.newTable();
                    L.pushValue(-2);
                    L.setField(-2, methodName);
                    try {
                        Object listener = L.getLuaObject(-1).createProxy(tp[0]);
                        L.pop(1);
                        m.invoke(obj, listener);
                        return 1;
                    } catch (Exception e) {
                        throw new LuaException(e);
                    }
                }
            }
        }
        return 0;
    }

    private static int javaSetMethod(LuaState L, Object obj, String methodName, boolean isClass) throws LuaException {
        synchronized (L) {
            char c = methodName.charAt(0);
            if (Character.isLowerCase(c)) {
                methodName = Character.toUpperCase(c) + methodName.substring(1);
            }
            String name = "set" + methodName;
            Method method = null;
            int methodType = -1;
            String cacheName = obj.getClass().getName() + "@->" + name;

            Object[] objs = new Object[1];
            switch (L.type(-1)) {
                case LuaState.LUA_TSTRING:
                    methodType = LuaState.LUA_TSTRING;
                    method = stringMethodCache.get(cacheName);
                    if (method != null)
                        objs[0] = L.toString(-1);
                    break;
                case LuaState.LUA_TBOOLEAN:
                    methodType = LuaState.LUA_TBOOLEAN;
                    method = boolMethodCache.get(cacheName);
                    if (method != null)
                        objs[0] = L.toBoolean(-1);
                    break;
                case LuaState.LUA_TNUMBER:
                    if (L.isInteger(-1)) {
                        methodType = LuaState.LUA_TINTEGER;
                        method = integerMethodCache.get(cacheName);
                        if (method != null)
                            objs[0] = LuaState.convertLuaNumber(L.toInteger(-1), method.getParameterTypes()[0]);
                    } else {
                        methodType = LuaState.LUA_TNUMBER;
                        method = doubleMethodCache.get(cacheName);
                        if (method != null)
                            objs[0] = LuaState.convertLuaNumber(L.toNumber(-1), method.getParameterTypes()[0]);
                    }
                    break;
            }

            if (method != null) {
                try {
                    method.invoke(obj, objs);
                    return 1;
                } catch (Exception e) {
                    throw new LuaException(e);
                }
            }

            ArrayList<Method> methods = getMethod(obj.getClass(), name, isClass);
            Object arg = null;
            StringBuilder buf = new StringBuilder();
            for (Method m : methods) {
                if (isClass && !Modifier.isStatic(m.getModifiers()))
                    continue;

                Class<?>[] tp = m.getParameterTypes();
                if (tp.length != 1)
                    continue;

                try {
                    arg = compareTypes(L, tp[0], L.getTop());
                } catch (LuaException e) {
                    buf.append("-> ").append(tp[0]);
                    buf.append("\n");
                    continue;
                }

                switch (methodType) {
                    case LuaState.LUA_TSTRING:
                        stringMethodCache.put(cacheName, m);
                        break;
                    case LuaState.LUA_TINTEGER:
                        integerMethodCache.put(cacheName, m);
                        break;
                    case LuaState.LUA_TNUMBER:
                        doubleMethodCache.put(cacheName, m);
                        break;
                    case LuaState.LUA_TBOOLEAN:
                        boolMethodCache.put(cacheName, m);
                        break;
                }

                try {
                    m.invoke(obj, arg);
                    return 1;
                } catch (Exception e) {
                    throw new LuaException(e);
                }

            }
            int top = L.getTop();
            if (L.type(top) == LuaState.LUA_TTABLE) {
                L.getField(1, name);
                LuaFunction func = L.getFunction(-1);
                if (L.type(-1) == LuaState.LUA_TFUNCTION) {
                    if (func != null) {
                        int len = L.rawLen(top);
                        //Object[] args = new Object[len];
                        for (int i = 0; i < len; i++) {
                            L.getI(top, i + 1);
                            //args[i] = L.toJavaObject(-1);
                        }
                        // throw new LuaException("Invalid setter "+L.typeName(1)+";"+L.typeName(2)+";"+L.typeName(3)+";"+L.typeName(4)+";"+L.typeName(5)+";"+len + Arrays.toString(args));
                        // func.call(args);
                        int ok = L.pcall(len, 0, 0);
                        if (ok == 0)
                            return 1;
                        else
                            throw new LuaException(L.toString(-1));
                    }
                }
            }
            if (buf.length() > 0)
                throw new LuaException("Invalid setter " + methodName + ". Invalid Parameters.\n" + buf.toString() + L.toJavaObject(-1).getClass());
        }
        return 0;
    }

    private static int createProxyObject(LuaState L, String implem)
            throws LuaException {
        synchronized (L) {
            try {
                LuaObject luaObj = L.getLuaObject(2);
                Object proxy = luaObj.createProxy(implem);
                L.pushJavaObject(proxy);
            } catch (Exception e) {
                throw new LuaException(e);
            }

            return 1;
        }
    }

    private static int createProxyObject(LuaState L, Class implem) throws LuaException {
        synchronized (L) {
            L.pushJavaObject(createProxyObject(L, implem, 2));
            return 1;
        }
    }

    private static Object createProxyObject(LuaState L, Class implem, int idx) throws LuaException {
        synchronized (L) {
            try {
                LuaObject luaObj = L.getLuaObject(idx);
                return luaObj.createProxy(implem);
            } catch (Exception e) {
                throw new LuaException(e);
            }
        }
    }

    private static int createArray(LuaState L, Class<?> type) throws LuaException {
        synchronized (L) {
            L.pushJavaObject(createArray(L, type, 2));
            return 1;
        }
    }


    private static Object createArray(LuaState L, Class<?> type, int idx) throws LuaException {
        synchronized (L) {
            try {
                int n = L.objLen(idx);
                Object array = Array.newInstance(type, n);
                /*if(n==0)
                    return array.getClass();
*/
                if (type == String.class) {
                    for (int i = 1; i <= n; i++) {
                        L.pushNumber(i);
                        L.getTable(idx);
                        Array.set(array, i - 1, L.toString(-1));
                        L.pop(1);
                    }
                } else if (type == Double.TYPE) {
                    for (int i = 1; i <= n; i++) {
                        L.pushNumber(i);
                        L.getTable(idx);
                        Array.set(array, i - 1, L.toNumber(-1));
                        L.pop(1);
                    }
                } else if (type == Float.TYPE) {
                    for (int i = 1; i <= n; i++) {
                        L.pushNumber(i);
                        L.getTable(idx);
                        Array.set(array, i - 1, (float) L.toNumber(-1));
                        L.pop(1);
                    }
                } else if (type == Long.TYPE) {
                    for (int i = 1; i <= n; i++) {
                        L.pushNumber(i);
                        L.getTable(idx);
                        Array.set(array, i - 1, L.toInteger(-1));
                        L.pop(1);
                    }
                } else if (type == Integer.TYPE) {
                    for (int i = 1; i <= n; i++) {
                        L.pushNumber(i);
                        L.getTable(idx);
                        Array.set(array, i - 1, (int) L.toInteger(-1));
                        L.pop(1);
                    }
                } else if (type == Short.TYPE) {
                    for (int i = 1; i <= n; i++) {
                        L.pushNumber(i);
                        L.getTable(idx);
                        Array.set(array, i - 1, (short) L.toInteger(-1));
                        L.pop(1);
                    }
                } else if (type == Character.TYPE) {
                    for (int i = 1; i <= n; i++) {
                        L.pushNumber(i);
                        L.getTable(idx);
                        Array.set(array, i - 1, (char) L.toInteger(-1));
                        L.pop(1);
                    }
                } else if (type == Byte.TYPE) {
                    for (int i = 1; i <= n; i++) {
                        L.pushNumber(i);
                        L.getTable(idx);
                        Array.set(array, i - 1, (byte) L.toInteger(-1));
                        L.pop(1);
                    }
                } else {
                    for (int i = 1; i <= n; i++) {
                        L.pushNumber(i);
                        L.getTable(idx);
                        Array.set(array, i - 1, compareTypes(L, type, L.getTop()));
                        L.pop(1);
                    }
                }
                return array;
            } catch (Exception e) {
                throw new LuaException(e);
            }
        }
    }

    private static int createList(LuaState L, Class<?> type) throws LuaException {
        synchronized (L) {
            L.pushJavaObject(createList(L, (Class<List<Object>>) type, 2));
            return 1;
        }
    }


    private static Object createList(LuaState L, Class<List<Object>> type, int idx) throws LuaException {
        synchronized (L) {
            int n = L.objLen(idx);
            try {
                List<Object> list;
                if (type.equals(List.class))
                    list = new ArrayList<>();
                else
                    list = type.newInstance();
                for (int i = 1; i <= n; i++) {
                    L.pushNumber(i);
                    L.getTable(idx);
                    list.add(L.toJavaObject(-1));
                    L.pop(1);
                }
                return list;
            } catch (Exception e) {
                throw new LuaException(e);
            }
        }
    }


    private static int createMap(LuaState L, Class<?> clazz) throws LuaException {
        // TODO: Implement this method
        synchronized (L) {
            L.pushJavaObject(createMap(L, (Class<Map<Object, Object>>) clazz, 2));
            return 1;
        }
    }

    private static Object createMap(LuaState L, Class<Map<Object, Object>> clazz, int idx) throws LuaException {
        // TODO: Implement this method
        synchronized (L) {
            try {
                Map<Object, Object> map;
                if (clazz.equals(Map.class))
                    map = new HashMap<>();
                else
                    map = (Map<Object, Object>) clazz.newInstance();
                L.pushNil();
                while (L.next(idx) != 0) {
                    map.put(L.toJavaObject(-2), L.toJavaObject(-1));
                    L.pop(1);
                }
                return map;
            } catch (Exception e) {
                throw new LuaException(e);
            }
        }
    }

    public static Object compareTypes(LuaState L, Class<?> parameter, int idx)
            throws LuaException {
        return compareTypes(L, parameter, L.type(idx), idx);
    }


    private static Object compareTypes(LuaState L, Class<?> parameter, int type, int idx)
            throws LuaException {
        boolean okType = true;
        Object obj = null;
        if (type == LuaState.LUA_TNIL)
            return null;
        switch (type) {
            case LuaState.LUA_TBOOLEAN: //boolean
            {
                if ((parameter.isPrimitive() && parameter != Boolean.TYPE) && !Boolean.class.isAssignableFrom(parameter)) {
                    okType = false;
                    break;
                }
                obj = L.toBoolean(idx);
            }
            break;
            case LuaState.LUA_TSTRING: //string
            {
                if (!parameter.isAssignableFrom(String.class)) {
                    okType = false;
                } else {
                    obj = L.toString(idx);
                }
            }
            break;
            case LuaState.LUA_TFUNCTION: //function
            {
                if (parameter.isInterface()) {
                    obj = createProxyObject(L, parameter, idx);
                } else if (!parameter.isAssignableFrom(LuaFunction.class)) {
                    okType = false;
                } else {
                    obj = L.getLuaObject(idx);
                }
            }
            break;
            case LuaState.LUA_TTABLE: //table
            {
                if (parameter.isAssignableFrom(LuaTable.class)) {
                    obj = L.getLuaObject(idx);
                } else if (parameter.isArray()) {
                    obj = createArray(L, parameter.getComponentType(), idx);
                } else if (List.class.isAssignableFrom(parameter)) {
                    obj = createList(L, (Class<List<Object>>) parameter, idx);
                } else if (Map.class.isAssignableFrom(parameter)) {
                    obj = createMap(L, (Class<Map<Object, Object>>) parameter, idx);
                } else if (parameter.isInterface()) {
                    obj = createProxyObject(L, parameter, idx);
                } else {
                    okType = false;
                }
            }
            break;
            case LuaState.LUA_TNUMBER: //number
            {
                if (!parameter.isPrimitive() && !Number.class.isAssignableFrom(parameter)) {
                    okType = false;
                    break;
                }
                if (L.isInteger(idx)) {
                    Long lg = L.toInteger(idx);
                    obj = LuaState.convertLuaNumber(lg, parameter);
                } else if (L.isNumber(idx)) {
                    Double db = L.toNumber(idx);
                    obj = LuaState.convertLuaNumber(db, parameter);
                }
            }
            break;
            case LuaState.LUA_TUSERDATA: //userdata
            {
                if (L.isObject(idx)) {
                    Object userObj = L.getObjectFromUserdata(idx);
                    if (userObj == null) {
                        return null;
                    } else if (parameter.isPrimitive()) {
                        Class<?> clazz = userObj.getClass();
                        if (parameter == byte.class && userObj instanceof Byte) {
                            obj = userObj;
                        } else if (parameter == short.class && userObj instanceof Short) {
                            obj = userObj;
                        } else if (parameter == int.class && userObj instanceof Integer) {
                            obj = userObj;
                        } else if (parameter == long.class && userObj instanceof Long) {
                            obj = userObj;
                        } else if (parameter == float.class && userObj instanceof Float) {
                            obj = userObj;
                        } else if (parameter == double.class && userObj instanceof Double) {
                            obj = userObj;
                        } else if (parameter == char.class && userObj instanceof Character) {
                            obj = userObj;
                        }
                    }
                    if (obj == null) {
                        if (parameter.isAssignableFrom(userObj.getClass())) {
                            obj = userObj;
                        } else {
                            okType = false;
                        }
                    }
                } else {
                    if (!parameter.isAssignableFrom(LuaObject.class)) {
                        okType = false;
                    } else {
                        obj = L.getLuaObject(idx);
                    }
                }
            }
            break;
            default: //other
            {
                throw new LuaException("Invalid Parameters.");
            }
        }
        if (!okType || obj == null) {
            throw new LuaException("Invalid Parameter.");
        }
        return obj;
    }


    private static int toPrimitive(LuaState L, Class type, int idx) throws LuaException {
        Object obj = null;

        if (type == Character.TYPE && L.type(idx) == LuaState.LUA_TSTRING) {
            String s = L.toString(idx);
            if (s.length() == 1)
                obj = s.charAt(0);
            else
                obj = s.toCharArray();
        } else if (!L.isNumber(idx)) {
            throw new LuaException(L.toString(idx) + " is not number");
        } else if (type == Double.TYPE) {
            obj = L.toNumber(idx);
        } else if (type == Float.TYPE) {
            obj = (float) L.toNumber(idx);
        } else if (type == Long.TYPE) {
            obj = L.toInteger(idx);
        } else if (type == Integer.TYPE) {
            obj = (int) L.toInteger(idx);
        } else if (type == Short.TYPE) {
            obj = (short) L.toInteger(idx);
        } else if (type == Character.TYPE) {
            obj = (char) L.toInteger(idx);
        } else if (type == Byte.TYPE) {
            obj = (byte) L.toInteger(idx);
        } else if (type == Boolean.TYPE) {
            obj = L.toBoolean(idx);
        }
        L.pushJavaObject(obj);
        return 1;
    }

    public static void pushJavaObject() {
        if(sJavaObjectIdx.addAndGet(1)>50000)
            throw new RuntimeException("Out of JavaObject");
    }
}


package com.android.cglib.proxy;

public class Const {
	
	public static final String SUBCLASS_SUFFIX = "_Enhancer";
	
	public static final String SUBCLASS_INVOKE_SUPER_SUFFIX = "_Super";
	
	public static Class getPackedType(Class primitive) {
		if (primitive == boolean.class) {
			return Boolean.class;
		} else if (primitive == byte.class) {
			return Byte.class;
		} else if (primitive == char.class) {
			return Character.class;
		} else if (primitive == double.class) {
			return Double.class;
		} else if (primitive == float.class) {
			return Float.class;
		} else if (primitive == int.class) {
			return Integer.class;
		} else if (primitive == long.class) {
			return Long.class;
		} else if (primitive == short.class) {
			return Short.class;
		} else {
			return primitive;
		}
	}
	
	public static String getPrimitiveValueMethodName(Class primitive) {
		if (primitive == boolean.class) {
			return "booleanValue";
		} else if (primitive == byte.class) {
			return "byteValue";
		} else if (primitive == char.class) {
			return "charValue";
		} else if (primitive == double.class) {
			return "doubleValue";
		} else if (primitive == float.class) {
			return "floatValue";
		} else if (primitive == int.class) {
			return "intValue";
		} else if (primitive == long.class) {
			return "longValue";
		} else if (primitive == short.class) {
			return "shortValue";
		} else {
			throw new ProxyException(primitive.getName() + " dit not primitive class");
		}
	}

}

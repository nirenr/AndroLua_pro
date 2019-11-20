package com.android.cglib.proxy;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;

import com.android.cglib.dx.Code;
import com.android.cglib.dx.Comparison;
import com.android.cglib.dx.DexMaker;
import com.android.cglib.dx.FieldId;
import com.android.cglib.dx.Label;
import com.android.cglib.dx.Local;
import com.android.cglib.dx.MethodId;
import com.android.cglib.dx.TypeId;
import android.content.Context;

public class Enhancer {
	
	private Context context;
	private Class<?> superclass;
	private MethodInterceptor interceptor;
	private MethodFilter methodFilter;

	public Enhancer(Context context) {
		this.context = context;
	}
	
	public void setSuperclass(Class<?> cls) {
		this.superclass = cls;
	}
	
	public void setInterceptor(MethodInterceptor interceptor) {
		this.interceptor = interceptor;
	}

	public Class<?> create() {
		String superClsName = superclass.getName().replace(".", "/");
		String subClsName = superClsName + Const.SUBCLASS_SUFFIX+"_"+hashCode();

		TypeId<?> superType = TypeId.get("L" + superClsName + ";");
		TypeId<?> subType = TypeId.get("L" + subClsName + ";");
		TypeId<?> interfaceTypeId = TypeId.get(EnhancerInterface.class);

		String cacheDir = context.getExternalFilesDir("dexfiles").getAbsolutePath();
//		System.out.println("[Enhancer::create()] Create class extends from \"" + superclass.getName() + "\" stored in " + cacheDir);

		DexMaker dexMaker = new DexMaker();
		dexMaker.declare(subType, superClsName + ".proxy", Modifier.PUBLIC, superType, interfaceTypeId);
		generateFieldsAndMethods(dexMaker, superType, subType);
		try {
			ClassLoader loader = dexMaker.generateAndLoad(Enhancer.class.getClassLoader(), new File(cacheDir));
			return loader.loadClass(subClsName);//superclass.getName() + Const.SUBCLASS_SUFFIX);
		} catch (IOException e) {
			e.printStackTrace();
		} catch (ClassNotFoundException e) {
			e.printStackTrace();
		}
		return null;
	}

	@SuppressWarnings({ "unchecked", "rawtypes" })
 	private <S> void generateFieldsAndMethods(DexMaker dexMaker, TypeId<?> superType, TypeId<S> subType) {
		TypeId<MethodInterceptor> methodInterceptorType = TypeId.get(MethodInterceptor.class);
		TypeId<MethodProxyExecuter> methodProxyExecuterType = TypeId.get(MethodProxyExecuter.class);
		TypeId<Class> classType = TypeId.get(Class.class);
		TypeId<Class[]> classesType = TypeId.get(Class[].class);
		TypeId<String> stringType = TypeId.get(String.class);
		TypeId<Object> objectType = TypeId.get(Object.class);
		TypeId<Object[]> objectsType = TypeId.get(Object[].class);
		
		// generate fields
		FieldId<S, MethodInterceptor> fieldId = subType.getField(methodInterceptorType, "methodInterceptor");
		dexMaker.declare(fieldId, Modifier.PRIVATE, null);
		
		// generate methods
		// constructor
		/*Code code = dexMaker.declare(subType.getConstructor(), Modifier.PUBLIC);
		Local thisRef = code.getThis(subType);
		code.invokeDirect(superType.getConstructor(), null, thisRef);
		code.returnVoid();*/


		Constructor<?>[] constructors = superclass.getDeclaredConstructors();
		for (Constructor constructor : constructors) {

			if((constructor.getModifiers()&Modifier.STATIC)!=0||(constructor.getModifiers()&Modifier.FINAL)!=0)
				continue;
			try{
				hookConstructor(dexMaker,superType,subType,constructor,fieldId);
			}catch (Exception e){
				e.printStackTrace();
			}

		}

		// setMethodInterceptor$Enhancer$
		MethodId<?, Void> setMethodInterceptorMethodId = subType.getMethod(TypeId.VOID, EnhancerInterface.SET_METHOD_INTERCEPTOR_ENHANCER, methodInterceptorType);
		Code code = dexMaker.declare(setMethodInterceptorMethodId, Modifier.PUBLIC);
		code.iput(fieldId, code.getThis(subType), code.getParameter(0, methodInterceptorType));
		code.returnVoid();


		// executeSuperMethod$Enhancer$
		MethodId<?, Object> executeSuperMethodMethodId = subType.getMethod(TypeId.OBJECT, EnhancerInterface.EXECUTE_SUPER_METHOD_ENHANCER, stringType, classesType, objectsType);
		code = dexMaker.declare(executeSuperMethodMethodId, Modifier.PUBLIC);
		Local<Object> retObjLocal = code.newLocal(objectType);
		Local<Class> subClassLocal = code.newLocal(classType);
		Local thisLocal = code.getThis(subType);

		code.invokeVirtual(subType.getMethod(classType, "getClass"), subClassLocal, thisLocal);
		MethodId methodId = methodProxyExecuterType.getMethod(TypeId.OBJECT, MethodProxyExecuter.EXECUTE_METHOD, classType, stringType, classesType, objectsType, objectType);
		code.invokeStatic(methodId, retObjLocal, subClassLocal, code.getParameter(0, stringType), code.getParameter(1, classesType), code.getParameter(2, objectsType), thisLocal);
		code.returnValue(retObjLocal);

		// override super's methods
		String methodName = null;
		Method[] methods = superclass.getMethods();
	    for (Method method : methods) {
			methodName = method.getName();
			if (methodName.contains(Const.SUBCLASS_SUFFIX)) {
				continue ;
			}
			if (methodName.contains(Const.SUBCLASS_INVOKE_SUPER_SUFFIX)) {
				continue ;
			}
			if((method.getModifiers()&Modifier.STATIC)!=0||(method.getModifiers()&Modifier.FINAL)!=0||(method.getModifiers()&Modifier.NATIVE)!=0)
				continue;
			if((method.getModifiers()&Modifier.ABSTRACT)==0&&methodFilter!=null&&!methodFilter.filter(method,methodName))
            	continue;
			try{
				hookMethod(dexMaker,superType,subType,method,methodName,fieldId);
			}catch (Exception e){
				e.printStackTrace();
			}

		}
	}

	private void hookConstructor(DexMaker dexMaker, TypeId<?> superType, TypeId<?> subType, Constructor method, FieldId<?, MethodInterceptor> fieldId) {
		TypeId<MethodInterceptor> methodInterceptorType = TypeId.get(MethodInterceptor.class);

		TypeId<Class> classType = TypeId.get(Class.class);
		TypeId<Class[]> classesType = TypeId.get(Class[].class);
		TypeId<String> stringType = TypeId.get(String.class);
		TypeId<Object> objectType = TypeId.get(Object.class);
		TypeId<Object[]> objectsType = TypeId.get(Object[].class);

		MethodId<?, ?> superMethodId = null;
		MethodId<?, ?> subMethodId = null;
		TypeId<?>[] argsTypeId = null;
		boolean hasParams = false;
		Class<?>[] argsClass = method.getParameterTypes();
		hasParams = argsClass != null && argsClass.length > 0;
		if (hasParams) {
			argsTypeId = new TypeId[argsClass.length];
			for (int i=0; i<argsClass.length; i++) {
				argsTypeId[i] = TypeId.get(argsClass[i]);
			}
			//subMethodId = subType.getConstructor(argsTypeId);
		} else {
			//subMethodId = subType.getConstructor();
		}

		/*Code code = dexMaker.declare(subMethodId, method.getModifiers());

		Local<Integer> intLocal = code.newLocal(TypeId.INT);
		Local<MethodInterceptor> methodInterceptorLocal = code.newLocal(methodInterceptorType);
		Local<String> methodNameLocal = code.newLocal(TypeId.get(String.class));
		Local<Class> tmpClassLocal = code.newLocal(classType);
		Local<Class> subClassLocal = code.newLocal(classType);
		Local<Class[]> argsTypeLocal = code.newLocal(classesType);
		Local<Object[]> argsValueLocal = code.newLocal(objectsType);
		Local tmpNumberLocal = code.newLocal(objectType);

		Local thisLocal = code.getThis(subType);
		code.iget(fieldId, methodInterceptorLocal, thisLocal);
		code.loadConstant(methodNameLocal, "<init>");
		code.invokeVirtual(subType.getMethod(classType, "getClass"), subClassLocal, thisLocal);

		MethodId methodId;
		if (hasParams) {
			code.loadConstant(intLocal, argsClass.length);
			code.newArray(argsTypeLocal, intLocal);
			code.newArray(argsValueLocal, intLocal);

			for (int i=0; i<argsClass.length; i++) {
				code.loadConstant(intLocal, i);
				code.loadConstant(tmpClassLocal, argsClass[i]);
				code.aput(argsTypeLocal, intLocal, tmpClassLocal);

				if (argsClass[i].isPrimitive()) {
					TypeId packedClassType = TypeId.get(Const.getPackedType(argsClass[i]));
					methodId = packedClassType.getMethod(packedClassType, "valueOf", argsTypeId[i]);
					code.invokeStatic(methodId, tmpNumberLocal, code.getParameter(i, argsTypeId[i]));
					code.aput(argsValueLocal, intLocal, tmpNumberLocal);
				} else {
					code.aput(argsValueLocal, intLocal, code.getParameter(i, argsTypeId[i]));
				}
			}
		} else {
			// must add below code, or "bad method" error will occurs.
			code.loadConstant(argsTypeLocal, null);
			code.loadConstant(argsValueLocal, null);
		}

		code.returnVoid();*/

		// generate method {methodName}$Super$ to invoke super's
		if (hasParams) {
			subMethodId = subType.getConstructor(argsTypeId);
			superMethodId = superType.getConstructor(argsTypeId);
		} else {
			subMethodId = subType.getConstructor();
			superMethodId = superType.getConstructor();
		}
		Code code = dexMaker.declare(subMethodId, method.getModifiers());
		Local[] superArgsValueLocal = null;
		Local thisLocal = code.getThis(subType);
		if (hasParams) {
			superArgsValueLocal = new Local[argsClass.length];
			for (int i=0; i<argsClass.length; i++) {
				superArgsValueLocal[i] = code.getParameter(i, argsTypeId[i]);
			}
			code.invokeDirect(superMethodId, null, thisLocal, superArgsValueLocal);
		} else {
			code.invokeDirect(superMethodId, null, thisLocal);
		}
		code.returnVoid();

	}


	private void hookMethod(DexMaker dexMaker, TypeId<?> superType, TypeId<?> subType, Method method, String methodName, FieldId<?, MethodInterceptor> fieldId) {
		TypeId<MethodInterceptor> methodInterceptorType = TypeId.get(MethodInterceptor.class);
		TypeId<MethodProxyExecuter> methodProxyExecuterType = TypeId.get(MethodProxyExecuter.class);
		TypeId<Class> classType = TypeId.get(Class.class);
		TypeId<Class[]> classesType = TypeId.get(Class[].class);
		TypeId<String> stringType = TypeId.get(String.class);
		TypeId<Object> objectType = TypeId.get(Object.class);
		TypeId<Object[]> objectsType = TypeId.get(Object[].class);


		MethodId<?, ?> superMethodId = null;
		MethodId<?, ?> subMethodId = null;
		TypeId<?>[] argsTypeId = null;
		TypeId<?> methodReturnType = null;
		boolean isVoid = false;
		boolean hasParams = false;
		Class retClass = null;

		retClass = method.getReturnType();
		isVoid = retClass.getSimpleName().equals("void");
		methodReturnType = TypeId.get(retClass);
		Class<?>[] argsClass = method.getParameterTypes();
		hasParams = (argsClass != null && argsClass.length > 0) ? true : false;
		if (hasParams) {
			argsTypeId = new TypeId[argsClass.length];
			for (int i=0; i<argsClass.length; i++) {
				argsTypeId[i] = TypeId.get(argsClass[i]);
			}
			subMethodId = subType.getMethod(methodReturnType, methodName, argsTypeId);
		} else {
			subMethodId = subType.getMethod(methodReturnType, methodName);
		}
		Code code = dexMaker.declare(subMethodId, method.getModifiers() & ~Modifier.ABSTRACT);

		Local retLocal = code.newLocal(methodReturnType);
		Local retPackLocal = null;
		if (retClass.isPrimitive()) {
			retPackLocal = code.newLocal(TypeId.get(Const.getPackedType(retClass)));
		}

		Local<Integer> intLocal = code.newLocal(TypeId.INT);
		Local<MethodInterceptor> methodInterceptorLocal = code.newLocal(methodInterceptorType);
		Local<String> methodNameLocal = code.newLocal(TypeId.get(String.class));
		Local<Class> tmpClassLocal = code.newLocal(classType);
		Local<Class> subClassLocal = code.newLocal(classType);
		Local<Class[]> argsTypeLocal = code.newLocal(classesType);
		Local<Object[]> argsValueLocal = code.newLocal(objectsType);
		Local tmpNumberLocal = code.newLocal(objectType);
		Local<Object> retObjLocal = code.newLocal(TypeId.OBJECT);

		Local thisLocal = code.getThis(subType);
		code.iget(fieldId, methodInterceptorLocal, thisLocal);
		code.loadConstant(methodNameLocal, methodName);
		code.invokeVirtual(subType.getMethod(classType, "getClass"), subClassLocal, thisLocal);

		MethodId methodId;
		if (hasParams) {
			code.loadConstant(intLocal, argsClass.length);
			code.newArray(argsTypeLocal, intLocal);
			code.newArray(argsValueLocal, intLocal);

			for (int i=0; i<argsClass.length; i++) {
				code.loadConstant(intLocal, i);
				code.loadConstant(tmpClassLocal, argsClass[i]);
				code.aput(argsTypeLocal, intLocal, tmpClassLocal);

				if (argsClass[i].isPrimitive()) {
					TypeId packedClassType = TypeId.get(Const.getPackedType(argsClass[i]));
					methodId = packedClassType.getMethod(packedClassType, "valueOf", argsTypeId[i]);
					code.invokeStatic(methodId, tmpNumberLocal, code.getParameter(i, argsTypeId[i]));
					code.aput(argsValueLocal, intLocal, tmpNumberLocal);
				} else {
					code.aput(argsValueLocal, intLocal, code.getParameter(i, argsTypeId[i]));
				}
			}
		} else {
			// must add below code, or "bad method" error will occurs.
			code.loadConstant(argsTypeLocal, null);
			code.loadConstant(argsValueLocal, null);
		}

		methodId = methodProxyExecuterType.getMethod(TypeId.OBJECT, MethodProxyExecuter.EXECUTE_INTERCEPTOR, methodInterceptorType, classType, stringType, classesType, objectsType, objectType);
		code.invokeStatic(methodId, isVoid ? null : retObjLocal, methodInterceptorLocal, subClassLocal, methodNameLocal, argsTypeLocal, argsValueLocal, thisLocal);

		if (isVoid) {
			code.returnVoid();
		} else {
			if (retClass.isPrimitive()) {
				// here use one label, if use two, need jump once and mark twice
				Label ifBody = new Label();
				code.loadConstant(retPackLocal, null);
				code.compare(Comparison.EQ, ifBody, retObjLocal, retPackLocal);

				code.cast(retPackLocal, retObjLocal);
				methodId = TypeId.get(Const.getPackedType(retClass)).getMethod(methodReturnType, Const.getPrimitiveValueMethodName(retClass));
				code.invokeVirtual(methodId, retLocal, retPackLocal);
				code.returnValue(retLocal);

				code.mark(ifBody);
				code.loadConstant(retLocal, 0);
				code.returnValue(retLocal);
			} else {
				code.cast(retLocal, retObjLocal);
				code.returnValue(retLocal);
			}
		}

		// generate method {methodName}$Super$ to invoke super's
		if (hasParams) {
			subMethodId = subType.getMethod(methodReturnType, methodName + Const.SUBCLASS_INVOKE_SUPER_SUFFIX, argsTypeId);
			superMethodId = superType.getMethod(methodReturnType, methodName, argsTypeId);
		} else {
			subMethodId = subType.getMethod(methodReturnType, methodName + Const.SUBCLASS_INVOKE_SUPER_SUFFIX);
			superMethodId = superType.getMethod(methodReturnType, methodName);
		}
		code = dexMaker.declare(subMethodId, method.getModifiers());
		retLocal = code.newLocal(methodReturnType);
		Local[] superArgsValueLocal = null;
		thisLocal = code.getThis(subType);
		if (hasParams) {
			superArgsValueLocal = new Local[argsClass.length];
			for (int i=0; i<argsClass.length; i++) {
				superArgsValueLocal[i] = code.getParameter(i, argsTypeId[i]);
			}
			code.invokeSuper(superMethodId, isVoid ? null : retLocal, thisLocal, superArgsValueLocal);
		} else {
			code.invokeSuper(superMethodId, isVoid ? null : retLocal, thisLocal);
		}
		if (isVoid) {
			code.returnVoid();
		} else {
			code.returnValue(retLocal);
		}
	}


	public void setMethodFilter(MethodFilter methodFilter) {
		this.methodFilter = methodFilter;
	}
}

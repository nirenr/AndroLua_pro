package com.luajava;

import android.widget.*;
import com.androlua.*;
import java.lang.reflect.*;

public class Jlua

{


	public static void init(LuaState L) throws LuaException
	{
		L.pushJavaObject(boolean.class);
		L.setGlobal("boolean");
		L.pushJavaObject(byte.class);
		L.setGlobal("byte");
		L.pushJavaObject(char.class);
		L.setGlobal("char");
		L.pushJavaObject(short.class);
		L.setGlobal("short");
		L.pushJavaObject(int.class);
		L.setGlobal("int");
		L.pushJavaObject(long.class);
		L.setGlobal("long");
		L.pushJavaObject(float.class);
		L.setGlobal("float");
		L.pushJavaObject(double.class);
		L.setGlobal("double");
		L.pushJavaObject(byte[].class);
		L.setGlobal("bytes");
		L.pushJavaObject(char[].class);
		L.setGlobal("chars");
		L.pushJavaObject(short[].class);
		L.setGlobal("shorts");
		L.pushJavaObject(int[].class);
		L.setGlobal("ints");
		L.pushJavaObject(long[].class);
		L.setGlobal("longs");
		L.pushJavaObject(float[].class);
		L.setGlobal("floats");
		L.pushJavaObject(double[].class);
		L.setGlobal("doubles");
		L.pushJavaObject(String[].class);

		JavaFunction tostring = new JavaFunction(L) {
			@Override
			public int execute() throws LuaException
			{
				String val = null;
				int i = 2;

				int type = L.type(i);
				String stype = L.typeName(type);
				if (stype.equals("userdata"))
				{
					Object obj = L.toJavaObject(i);
					if (obj != null)
						val = obj.toString();
				}
				else if (stype.equals("boolean"))
				{
					val = L.toBoolean(i) ? "true" : "false";
				}
				else
				{
					val = L.toString(i);
				}
				if (val == null)
					val = stype;						

				L.pushString(val);
				return 1;
			}
		};
		L.getGlobal("luajava"); 
		L.pushJavaFunction(tostring);
		L.setField(-2, "tostring"); 
		L.pop(1);                        

		JavaFunction createArray = new JavaFunction(L) {
			@Override
			public int execute()
			{
				try
				{
					int n = L.rawLen(-1);
					Class type = (Class) L.getObjectFromUserdata(2);
					Object array = Array.newInstance(type, n);
					if (n == 0)
					{
						L.pushJavaObject(array.getClass());
						return 1;
					}
					if (type == String.class)
					{ 
						for (int i = 1;i <= n;i++)
						{
							L.pushNumber(i);
							L.getTable(-2);
							Array.set(array, i - 1, L.toString(-1));
							L.pop(1);
						}
					}  
					else if (type == double.class)
					{ 
						for (int i = 1;i <= n;i++)
						{
							L.pushNumber(i);
							L.getTable(-2);
							Array.set(array, i - 1, L.toNumber(-1));
							L.pop(1);
						}
					}  
					else if (type == float.class)
					{ 
						for (int i = 1;i <= n;i++)
						{
							L.pushNumber(i);
							L.getTable(-2);
							Array.set(array, i - 1, (float)L.toNumber(-1));
							L.pop(1);
						}
					}  
					else if (type == long.class)
					{ 
						for (int i = 1;i <= n;i++)
						{
							L.pushNumber(i);
							L.getTable(-2);
							Array.set(array, i - 1, (long)L.toNumber(-1));
							L.pop(1);
						}
					}  
					else if (type == int.class)
					{ 
						for (int i = 1;i <= n;i++)
						{
							L.pushNumber(i);
							L.getTable(-2);
							Array.set(array, i - 1, L.toInteger(-1));
							L.pop(1);
						}
					}
					else if (type == short.class)
					{ 
						for (int i = 1;i <= n;i++)
						{
							L.pushNumber(i);
							L.getTable(-2);
							Array.set(array, i - 1, (short)L.toInteger(-1));
							L.pop(1);
						}
					}  
					else if (type == char.class)
					{ 
						for (int i = 1;i <= n;i++)
						{
							L.pushNumber(i);
							L.getTable(-2);
							Array.set(array, i - 1, (char)L.toInteger(-1));
							L.pop(1);
						}
					}  
					else if (type == byte.class)
					{ 
						for (int i = 1;i <= n;i++)
						{
							L.pushNumber(i);
							L.getTable(-2);
							Array.set(array, i - 1, (byte)L.toInteger(-1));
							L.pop(1);
						}
					}  
					else
					{ 
						for (int i = 1;i <= n;i++)
						{
							L.pushNumber(i);
							L.getTable(-2);
							Array.set(array, i - 1, L.toJavaObject(-1));
							L.pop(1);
						}
					}  
					L.pushJavaObject(array);
					return 1;
				}
				catch (Exception e)
				{
					L.pushNil();
					L.pushString(e.getMessage());
					return 2;
				}

			}
		};
		/*		L.getGlobal("luajava"); 
		 L.pushJavaFunction(createArray);
		 L.setField(-2, "createArray"); 
		 L.pop(1);                        
		 */
		JavaFunction totable = new JavaFunction(L) {
			@Override
			public int execute()
			{
				try
				{
					Object array = L.getObjectFromUserdata(2);
					int n=Array.getLength(array);
					L.newTable();
					for (int i=0;i <= n - 1 ;i++)
					{
						L.pushObjectValue(Array.get(array, i));
						L.pushNumber(i + 1);
						L.setTable(-3);
					}
					L.pushValue(-1);
					return 1;
				}
				catch (Exception e)
				{
					L.pushNil();
					L.pushString(e.getMessage());
					return 2;
				}

			}
		};

		JavaFunction getClass = new JavaFunction(L) {
			@Override
			public int execute()
			{
				String classname=L.toString(2);
				int n=L.getTop();
				try
				{
					Class clazz=Class.forName(classname);
					if (n > 2)
					{
						Class[] arg=new Class[n - 2];
						for (int i=3;i <= n;i++)
						{
							arg[i - 3] = (Class) L.getObjectFromUserdata(i);
						}
						Constructor constructor=clazz.getDeclaredConstructor(arg);
						L.pushJavaObject(constructor);
						return 1;
					}
					L.pushJavaObject(clazz);
					return 1;
				}
				catch (Exception e)
				{
					L.pushNil();
					L.pushString(e.getMessage());
					return 2;
				}
			}
		};


		JavaFunction New = new JavaFunction(L) {
			@Override
			public int execute()
			{
				int n=L.getTop();
				Object[] args=null;
				try
				{
					if (n > 2)
					{
						Constructor constructor=(Constructor) L.getObjectFromUserdata(2);
						args = new Object[n - 2];
						for (int i=3;i <= n;i++)
						{
							args[i - 3] = L.getObjectFromUserdata(i);
						}
						Object obj=constructor.newInstance(args);
						L.pushJavaObject(obj);
						return 1;
					}
					else
					{
						Class clazz=(Class)L.getObjectFromUserdata(2);
						Object obj=clazz.newInstance();
						L.pushJavaObject(obj);
						return 1;
					}
				}
				catch (Exception e)
				{
					L.pushNil();
					L.pushString(e.getMessage());
					return 2;
				}

			}
		};

		JavaFunction getMethod = new JavaFunction(L) {
			@Override
			public int execute()
			{
				Class clazz=null;
				Method method = null;
				int n=L.getTop();
				Class[] args=null;
				try
				{
					Object obj = L.getObjectFromUserdata(2);
					if (obj instanceof Class)
					{
						clazz = (Class) obj;
					}
					else
					{
						clazz = obj.getClass();
					}
					String methodname=L.toString(3);
					if (n > 3)
					{
						args = new Class[n - 3];
						for (int i=4;i <= n;i++)
						{
							args[i - 4] = (Class) L.getObjectFromUserdata(i);
						}
					}
					try
					{
						method = clazz.getDeclaredMethod(methodname, args);
					}
					catch (NoSuchMethodException e)
					{
						method = clazz.getMethod(methodname, args);
					}
					L.pushJavaObject(method);
					return 1;
				}
				catch (Exception e)
				{
					L.pushNil();
					L.pushString(e.getMessage());
					return 2;
				}

			}
		};


		JavaFunction Call = new JavaFunction(L) {
			@Override
			public int execute()
			{
				int n=L.getTop();
				Object[] args=null;
				try
				{
					Object clazz=L.getObjectFromUserdata(2);
					Method method=(Method) L.getObjectFromUserdata(3);
					method.setAccessible(true);
					if (n > 3)
					{
						args = new Object[n - 3];
						for (int i=4;i <= n;i++)
						{
							args[i - 4] = L.getObjectFromUserdata(i);
						}
					}
					Object obj=method.invoke(clazz, args);
					L.pushObjectValue(obj);
					return 1;
				}
				catch (Exception e)
				{
					L.pushNil();
					L.pushString(e.getMessage());
					return 2;
				}

			}
		};


		JavaFunction getField = new JavaFunction(L) {
			@Override
			public int execute()
			{
				Class clazz=null;
				Field field=null;				
				try
				{
					Object obj = L.getObjectFromUserdata(2);
					if (obj instanceof Class)
					{
						clazz = (Class) obj;
					}
					else
					{
						clazz = obj.getClass();
					}
					String fieldname=L.toString(3);
					try
					{
						field = clazz.getDeclaredField(fieldname);
					}
					catch (NoSuchFieldException e)
					{
						field = clazz.getField(fieldname);
					}

					L.pushJavaObject(field);
					return 1;
				}
				catch (Exception e)
				{

					L.pushNil();
					L.pushString(e.getMessage());

					return 2;
				}

			}
		};

		JavaFunction Get = new JavaFunction(L) {
			@Override
			public int execute()
			{
				try
				{
					Object clazz = L.getObjectFromUserdata(2);
					Field field = (Field) L.getObjectFromUserdata(3);
					field.setAccessible(true);
					Object obj = field.get(clazz);
					L.pushObjectValue(obj);
					return 1;
				}
				catch (Exception e)
				{
					L.pushNil();
					L.pushString(e.getMessage());
					return 2;
				}

			}
		};

		JavaFunction Set = new JavaFunction(L) {
			@Override
			public int execute()
			{
				try
				{
					Object clazz = L.getObjectFromUserdata(2);
					Field field = (Field) L.getObjectFromUserdata(3);
					Object obj = L.getObjectFromUserdata(4);
					field.setAccessible(true);
					field.set(clazz, obj);
					L.pushBoolean(true);
					return 1;
				}
				catch (Exception e)
				{
					L.pushNil();
					L.pushString(e.getMessage());
					return 2;
				}

			}
		};

		JavaFunction As = new JavaFunction(L) {
			@Override
			public int execute() throws LuaException
			{
				try
				{
					Class type = (Class) L.getObjectFromUserdata(2);
					L.remove(2);
					int n = L.getTop();
					if (type == String.class)
					{ 
						for (int i = 2;i <= n;i++)
						{
							L.pushJavaObject(L.toString(2));
							L.remove(2);
						}
					}  
					else if (type == double.class)
					{ 
						for (int i = 2;i <= n;i++)
						{
							L.pushJavaObject(L.toNumber(2));
							L.remove(2);
						}
					}  
					else if (type == float.class)
					{ 
						for (int i = 2;i <= n;i++)
						{
							L.pushJavaObject((float)L.toNumber(2));
							L.remove(2);
						}
					}  
					else if (type == long.class)
					{ 
						for (int i = 2;i <= n;i++)
						{
							L.pushJavaObject((long)L.toNumber(2));
							L.remove(2);
						}
					}  
					else if (type == int.class)
					{ 
						for (int i = 2;i <= n;i++)
						{
							L.pushJavaObject(L.toInteger(2));
							L.remove(2);
						}
					}
					else if (type == short.class)
					{ 
						for (int i = 2;i <= n;i++)
						{
							L.pushJavaObject((short)L.toInteger(2));
							L.remove(2);
						}
					}  
					else if (type == char.class)
					{ 
						for (int i = 2;i <= n;i++)
						{
							L.pushJavaObject((char)L.toInteger(2));
							L.remove(2);
						}
					}  
					else if (type == byte.class)
					{ 
						for (int i = 2;i <= n;i++)
						{
							L.pushJavaObject((byte)L.toInteger(2));
							L.remove(2);
						}
					}  
					else
					{ 
						for (int i = 2;i <= n;i++)
						{
							L.pushJavaObject(L.toJavaObject(2));
							L.remove(2);
						}
					}  
					return n - 1;
				}
				catch (Exception e)
				{
					L.pushNil();
					L.pushString(e.getMessage());
					return 2;
				}

			}
		};
		L.newTable();
		L.setGlobal("jlua");
		L.getGlobal("jlua");
		L.pushJavaFunction(getClass);
		L.setField(-2, "getClass");
		L.pushJavaFunction(New);
		L.setField(-2, "new");
		L.pushJavaFunction(getMethod);
		L.setField(-2, "getMethod");
		L.pushJavaFunction(Call);
		L.setField(-2, "call");
		L.pushJavaFunction(getField);
		L.setField(-2, "getField");
		L.pushJavaFunction(Get);
		L.setField(-2, "get");
		L.pushJavaFunction(Set);
		L.setField(-2, "set");
		L.pushJavaFunction(As);
		L.setField(-2, "as");
		L.pushJavaFunction(createArray);
		L.setField(-2, "createArray"); 
		L.pushJavaFunction(totable);
		L.setField(-2, "totable");
		L.pop(1);
	}

}

package com.luajava;

import java.util.*;
import java.util.Map.*;

public class LuaTable extends LuaObject implements Map
{

	@Override
	public void clear()
	{
		// TODO: Implement this method
	}

	@Override
	public boolean containsKey(Object p1)
	{
		// TODO: Implement this method
		return false;
	}

	@Override
	public boolean containsValue(Object p1)
	{
		// TODO: Implement this method
		return false;
	}

	@Override
	public Set entrySet()
	{
		// TODO: Implement this method
		return null;
	}

	@Override
	public Object get(Object p1)
	{
		// TODO: Implement this method
		push();
		Object obj=null;
		try
		{
			L.pushObjectValue(p1);
			L.getTable(-2);
			obj=L.toJavaObject(-1);
			L.pop(1);
		}
		catch (LuaException e)
		{}
		L.pop(1);
		return obj;
	}


	@Override
	public boolean isEmpty()
	{
		// TODO: Implement this method
		return false;
	}

	@Override
	public Set keySet()
	{
		// TODO: Implement this method
		return null;
	}

	@Override
	public Object put(Object p1, Object p2)
	{
		// TODO: Implement this method
		push();
		try
		{
			L.pushObjectValue(p1);
			L.pushObjectValue(p2);
			L.setTable(-3);
		}
		catch (LuaException e)
		{}
		L.pop(1);
		return p1;
	}

	@Override
	public void putAll(Map p1)
	{
		// TODO: Implement this method
	}

	@Override
	public Object remove(Object p1)
	{
		// TODO: Implement this method
		return null;
	}

	@Override
	public int size()
	{
		// TODO: Implement this method
		return 0;
	}

	@Override
	public Collection values()
	{
		// TODO: Implement this method
		return null;
	}
	


	protected LuaTable(LuaState L, String globalName)
	{
		super(L,globalName);
	}
	
	protected LuaTable(LuaState L, int index)
	{
		super(L,index);
	}
	
	public LuaTable(LuaState L)
	{
		super(L);
		L.newTable();
		registerValue(-1);
		L.pop(1);
	}
	
}

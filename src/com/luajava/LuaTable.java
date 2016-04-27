package com.luajava;

import java.util.*;
import java.util.Map.*;

public class LuaTable extends LuaObject implements List//,Map
{

	@Override
	public void add(int p1, Object p2)
	{
		// TODO: Implement this method
	}

	@Override
	public boolean add(Object p1)
	{
		// TODO: Implement this method
		return false;
	}

	@Override
	public boolean addAll(int p1, Collection p2)
	{
		// TODO: Implement this method
		return false;
	}

	@Override
	public boolean addAll(Collection p1)
	{
		// TODO: Implement this method
		return false;
	}

	@Override
	public void clear()
	{
		// TODO: Implement this method
	}

	@Override
	public boolean contains(Object p1)
	{
		// TODO: Implement this method
		return false;
	}

	@Override
	public boolean containsAll(Collection p1)
	{
		// TODO: Implement this method
		return false;
	}

	@Override
	public Object get(int p1)
	{
		// TODO: Implement this method
		return null;
	}

	@Override
	public int indexOf(Object p1)
	{
		// TODO: Implement this method
		return 0;
	}

	@Override
	public boolean isEmpty()
	{
		// TODO: Implement this method
		return false;
	}

	@Override
	public Iterator iterator()
	{
		// TODO: Implement this method
		return null;
	}

	@Override
	public int lastIndexOf(Object p1)
	{
		// TODO: Implement this method
		return 0;
	}

	@Override
	public ListIterator listIterator()
	{
		// TODO: Implement this method
		return null;
	}

	@Override
	public ListIterator listIterator(int p1)
	{
		// TODO: Implement this method
		return null;
	}

	@Override
	public Object remove(int p1)
	{
		// TODO: Implement this method
		return null;
	}

	@Override
	public boolean remove(Object p1)
	{
		// TODO: Implement this method
		return false;
	}

	@Override
	public boolean removeAll(Collection p1)
	{
		// TODO: Implement this method
		return false;
	}

	@Override
	public boolean retainAll(Collection p1)
	{
		// TODO: Implement this method
		return false;
	}

	@Override
	public Object set(int p1, Object p2)
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
	public List subList(int p1, int p2)
	{
		// TODO: Implement this method
		return null;
	}

	@Override
	public Object[] toArray()
	{
		// TODO: Implement this method
		return null;
	}

	@Override
	public Object[] toArray(Object[] p1)
	{
		// TODO: Implement this method
		return null;
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
		return null;
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
		return null;
	}

	@Override
	public void putAll(Map p1)
	{
		// TODO: Implement this method
	}

	@Override
	public Collection values()
	{
		// TODO: Implement this method
		return null;
	}
	
	protected LuaTable(LuaState L, String globalName) throws LuaException
	{
		super(L,globalName);
		if(!isTable())
		{
			throw new LuaException("not a table");
		}
	}
}

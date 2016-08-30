package com.luajava;

import java.util.*;
import java.util.Map.*;

public class LuaTable extends LuaObject implements Map {

	@Override
	public void clear() {
		// TODO: Implement this method
		push();
		L.pushNil();
		while (L.next(-2) != 0) {
			L.pop(1);
			L.pushValue(-1);
			L.pushNil();
			L.setTable(-4);
		}
		L.pop(1);
	}

	@Override
	public boolean containsKey(Object p1) {
		// TODO: Implement this method
		boolean b=false;
		push();
		try {
			L.pushObjectValue(p1);
			b = L.getTable(-2) == LuaState.LUA_TNIL;
			L.pop(1);
		}
		catch (LuaException e) {
			return false;
		}
		L.pop(1);
		return b;
	}

	@Override
	public boolean containsValue(Object p1) {
		// TODO: Implement this method
		return false;
	}

	@Override
	public Set entrySet() {
		// TODO: Implement this method
		HashSet<LuaEntry> sets=new HashSet<LuaEntry>();
		push();
		L.pushNil();
		while (L.next(-2) != 0) {
			try {
				sets.add(new LuaEntry(L.toJavaObject(-2), L.toJavaObject(-1)));
			}
			catch (LuaException e) {}
			L.pop(1);
		}
		L.pop(1);
		return sets;
	}

	@Override
	public Object get(Object p1) {
		// TODO: Implement this method
		push();
		Object obj=null;
		try {
			L.pushObjectValue(p1);
			L.getTable(-2);
			obj = L.toJavaObject(-1);
			L.pop(1);
		}
		catch (LuaException e) {}
		L.pop(1);
		return obj;
	}


	@Override
	public boolean isEmpty() {
		// TODO: Implement this method
		push();
		L.pushNil();
		boolean b=L.next(-2) == 0;
		if (b)
			L.pop(3);
		else
			L.pop(1);
		return b;
	}

	@Override
	public Set keySet() {
		// TODO: Implement this method
		HashSet<Object> sets=new HashSet<Object>();
		push();
		L.pushNil();
		while (L.next(-2) != 0) {
			try {
				sets.add(L.toJavaObject(-2));
			}
			catch (LuaException e) {}
			L.pop(1);
		}
		L.pop(1);
		return sets;
	}

	@Override
	public Object put(Object p1, Object p2) {
		// TODO: Implement this method
		push();
		try {
			L.pushObjectValue(p1);
			L.pushObjectValue(p2);
			L.setTable(-3);
		}
		catch (LuaException e) {}
		L.pop(1);
		return p1;
	}

	@Override
	public void putAll(Map p1) {
		// TODO: Implement this method
	}

	@Override
	public Object remove(Object p1) {
		// TODO: Implement this method
		return null;
	}

	@Override
	public int size() {
		// TODO: Implement this method
		int n=0;
		push();
		L.pushNil();
		while (L.next(-2) != 0) {
			n++;
			L.pop(1);
		}
		L.pop(1);
		return n;
	}

	@Override
	public Collection values() {
		// TODO: Implement this method
		return null;
	}



	protected LuaTable(LuaState L, String globalName) {
		super(L, globalName);
	}

	protected LuaTable(LuaState L, int index) {
		super(L, index);
	}

	public LuaTable(LuaState L) {
		super(L);
		L.newTable();
		registerValue(-1);
	}



	private class LuaEntry implements Entry {

		private Object mKey;

		private Object mValue;

		@Override
		public Object getKey() {
			// TODO: Implement this method
			return mKey;
		}

		@Override
		public Object getValue() {
			// TODO: Implement this method
			return mValue;
		}

		@Override
		public Object setValue(Object value) {
			// TODO: Implement this method
			Object old=mValue;
			mValue = value;
			return old;
		}

		public LuaEntry(Object k, Object v) {
			mKey = k;
			mValue = v;
		}
	}
}

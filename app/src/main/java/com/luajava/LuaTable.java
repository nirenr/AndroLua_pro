package com.luajava;

import java.util.*;
import java.util.Map.*;

public class LuaTable <K, V>extends LuaObject implements Map <K,V>{

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
	public boolean containsKey(Object key) {
		// TODO: Implement this method
		boolean b=false;
		push();
		try {
			L.pushObjectValue(key);
			b = L.getTable(-2) != LuaState.LUA_TNIL;
			L.pop(1);
		}
		catch (LuaException e) {
			return false;
		}
		L.pop(1);
		return b;
	}

	@Override
	public boolean containsValue(Object value) {
		// TODO: Implement this method
		return false;
	}

	@Override
	public Set<Entry<K,V>> entrySet() {
		// TODO: Implement this method
		HashSet<Entry<K,V>> sets=new HashSet<Entry<K,V>>();
		push();
		L.pushNil();
		while (L.next(-2) != 0) {
			try {
				sets.add(new LuaEntry<K,V>((K)L.toJavaObject(-2), (V)L.toJavaObject(-1)));
			}
			catch (LuaException e) {}
			L.pop(1);
		}
		L.pop(1);
		return sets;
	}

	@Override
	public V get(Object key) {
		// TODO: Implement this method
		push();
		V obj=null;
		try {
			L.pushObjectValue(key);
			L.getTable(-2);
			obj = (V) L.toJavaObject(-1);
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
			L.pop(1);
		else
			L.pop(3);
		return b;
	}

	@Override
	public Set<K> keySet() {
		// TODO: Implement this method
		HashSet<K> sets=new HashSet<K>();
		push();
		L.pushNil();
		while (L.next(-2) != 0) {
			try {
				sets.add((K)L.toJavaObject(-2));
			}
			catch (LuaException e) {}
			L.pop(1);
		}
		L.pop(1);
		return sets;
	}

	@Override
	public V put(K key, V value) {
		// TODO: Implement this method
		push();
		try {
			L.pushObjectValue(key);
			L.pushObjectValue(value);
			L.setTable(-3);
		}
		catch (LuaException e) {}
		L.pop(1);
		return null;
	}

	@Override
	public void putAll(Map p1) {
		// TODO: Implement this method
	}

	@Override
	public V remove(Object key) {
		// TODO: Implement this method
		push();
		try {
			L.pushObjectValue(key);
			L.setTable(-2);
		}
		catch (LuaException e) {}
		L.pop(1);
		return null;
	}

	public boolean isList() {
		push();
		int len=L.rawLen(-1);
		if(len!=0){
			pop();
			return true;
		}
		L.pushNil();
		boolean b=L.next(-2) == 0;
		if (b)
			L.pop(1);
		else
			L.pop(3);
		return b;
	}

	public int length() {
		// TODO: Implement this method
		push();
		int len=L.rawLen(-1);
		pop();
		return len;
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
	public Collection<V> values() {
		ArrayList<V> sets=new ArrayList<>();
		push();
		L.pushNil();
		while (L.next(-2) != 0) {
			try {
				sets.add((V)L.toJavaObject(-1));
			}
			catch (LuaException e) {}
			L.pop(1);
		}
		L.pop(1);
		return sets;
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

	public class LuaEntry <K,V> implements Entry <K,V>{

		private K mKey;

		private V mValue;

		@Override
		public K getKey() {
			// TODO: Implement this method
			return mKey;
		}

		@Override
		public V getValue() {
			// TODO: Implement this method
			return mValue;
		}

		public V setValue(V value) {
			// TODO: Implement this method
			V old=mValue;
			mValue = value;
			return old;
		}

		public LuaEntry(K k, V v) {
			mKey = k;
			mValue = v;
		}
	}
}

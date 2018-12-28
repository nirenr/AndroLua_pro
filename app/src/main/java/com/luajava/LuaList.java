package com.luajava;
import java.util.*;

public class LuaList extends LuaObject implements List
{

	@Override
	public void clear() {
		// TODO: Implement this method
	}

	@Override
	public boolean isEmpty() {
		// TODO: Implement this method
		push();
		int len=L.rawLen(-1);
		L.pop(1);
		return len==0;
	}

	@Override
	public boolean remove(Object p1) {
		// TODO: Implement this method
		return false;
	}

	@Override
	public int size() {
		// TODO: Implement this method
		push();
		int len=L.rawLen(-1);
		L.pop(1);
		return len;
	}


	@Override
	public void add(int p1, Object p2) {
		// TODO: Implement this method
	}

	@Override
	public boolean add(Object p1) {
		// TODO: Implement this method
		push();
		int len=L.rawLen(-1);
		try {
			L.pushObjectValue(p1);
			L.setI(-2, len + 1);
			pop();
			return true;
		}
		catch (LuaException e) {
			pop();
			return false;
		}
	}

	@Override
	public boolean addAll(int p1, Collection p2) {
		// TODO: Implement this method
		return false;
	}

	@Override
	public boolean addAll(Collection p1) {
		// TODO: Implement this method
		return false;
	}

	@Override
	public boolean contains(Object p1) {
		// TODO: Implement this method
		return false;
	}

	@Override
	public boolean containsAll(Collection p1) {
		// TODO: Implement this method
		return false;
	}

	@Override
	public Object get(int p1) {
		// TODO: Implement this method
		return null;
	}

	@Override
	public int indexOf(Object p1) {
		// TODO: Implement this method
		return 0;
	}

	@Override
	public Iterator iterator() {
		// TODO: Implement this method
		return null;
	}

	@Override
	public int lastIndexOf(Object p1) {
		// TODO: Implement this method
		return 0;
	}

	@Override
	public ListIterator listIterator() {
		// TODO: Implement this method
		return null;
	}

	@Override
	public ListIterator listIterator(int p1) {
		// TODO: Implement this method
		return null;
	}

	@Override
	public Object remove(int p1) {
		// TODO: Implement this method
		return null;
	}

	@Override
	public boolean removeAll(Collection p1) {
		// TODO: Implement this method
		return false;
	}

	@Override
	public boolean retainAll(Collection p1) {
		// TODO: Implement this method
		return false;
	}

	@Override
	public Object set(int p1, Object p2) {
		// TODO: Implement this method
		return null;
	}

	@Override
	public List subList(int p1, int p2) {
		// TODO: Implement this method
		return null;
	}

	@Override
	public Object[] toArray() {
		// TODO: Implement this method
		return null;
	}

	@Override
	public Object[] toArray(Object[] p1) {
		// TODO: Implement this method
		return null;
	}
	
	protected LuaList(LuaState L, String globalName) {
		super(L, globalName);
	}

	protected LuaList(LuaState L, int index) {
		super(L, index);
	}

	public LuaList(LuaState L) {
		super(L);
		L.newTable();
		registerValue(-1);
	}
	
}

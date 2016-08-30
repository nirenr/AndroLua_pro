package com.androlua;
import java.util.*;
import com.luajava.*;

public class LuaMap extends HashMap implements LuaMetaTable
{

	@Override
	public Object __call(Object[] arg) {
		// TODO: Implement this method
		return arg;
	}

	@Override
	public Object __index(String key) {
		// TODO: Implement this method
		return get(key);
	}

	@Override
	public void __newIndex(String key, Object value) {
		// TODO: Implement this method
		put(key,value);
	}
	
}

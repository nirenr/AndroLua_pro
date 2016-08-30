package com.luajava;

public class LuaFunction extends LuaObject {
	protected LuaFunction(LuaState L, String globalName) {
		super(L, globalName);
	}

	protected LuaFunction(LuaState L, int index) {
		super(L, index);
	}
}

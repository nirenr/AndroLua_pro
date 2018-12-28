package com.androlua;

import com.luajava.*;

public class LuaExAdapter extends LuaExpandableListAdapter 
{
	public LuaExAdapter(LuaContext context,  LuaTable groupLayout, LuaTable childLayout) throws LuaException {
		this(context,null,null,groupLayout,childLayout);
	}
	
	public LuaExAdapter(LuaContext context, LuaTable<Integer,LuaTable<String,Object>> groupData, LuaTable<Integer,LuaTable<Integer,LuaTable<String,Object>>> childData, LuaTable groupLayout, LuaTable childLayout) throws LuaException {
		super(context,groupData,childData,groupLayout,childLayout);
	}
}

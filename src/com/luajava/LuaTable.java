package com.luajava;

import java.util.*;
import java.util.Map.*;

public class LuaTable extends LuaObject
{


	protected LuaTable(LuaState L, String globalName) throws LuaException
	{
		super(L,globalName);
		if(!isTable())
		{
			throw new LuaException("not a table");
		}
	}
}

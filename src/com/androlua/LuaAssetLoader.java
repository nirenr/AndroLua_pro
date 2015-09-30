package com.androlua;

import com.luajava.*;
import java.io.*;

public class LuaAssetLoader extends JavaFunction
{

	private LuaState L;

	private Main mMain;

	public LuaAssetLoader(Main main,LuaState L)
	{
		super(L);
		this.L = L;
		mMain=main;
	}

	@Override
	public int execute() throws LuaException
	{
		String name = L.toString(-1);
		name = name.replace('.', '/') + ".lua";
		try
		{
			byte[] bytes = mMain.readAsset(name);
			int ok=L.LloadBuffer(bytes, name);
			if (ok != 0)
				L.pushString("\n\t" + L.toString(-1));
			return 1;
		}
		catch (IOException e)
		{
			L.pushString("\n\tno file \'/assets/" + name + "\'");
			return 1;
		}
	}

}


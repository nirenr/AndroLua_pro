package com.androlua;

import android.content.*;
import android.content.res.*;
import com.luajava.*;
import java.io.*;

public class LuaAssetLoader extends JavaFunction
{

	private LuaState L;

	private Context mContext;

	public LuaAssetLoader(LuaContext luaContext,LuaState L)
	{
		super(L);
		this.L = L;
		mContext=luaContext.getContext();
	}

	@Override
	public int execute() throws LuaException
	{
		String name = L.toString(-1);
		name = name.replace('.', '/') + ".lua";
		try
		{
			byte[] bytes = readAsset(name);
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
	public byte[] readAsset(String name) throws IOException 
	{
		AssetManager am = mContext.getAssets();
		InputStream is = am.open(name);
		byte[] ret= readAll(is);
		is.close();
		//am.close();
		return ret;
	}

	private static byte[] readAll(InputStream input) throws IOException 
	{
		ByteArrayOutputStream output = new ByteArrayOutputStream(4096);
		byte[] buffer = new byte[4096];
		int n = 0;
		while (-1 != (n = input.read(buffer)))
		{
			output.write(buffer, 0, n);
		}
		byte[] ret= output.toByteArray();
		output.close();
		return ret;
	}
}


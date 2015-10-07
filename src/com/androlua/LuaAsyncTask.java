package com.androlua;

import android.os.*;
import com.luajava.*;
import java.io.*;
import android.view.Window.*;

public class LuaAsyncTask extends AsyncTask
{

	private LuaState L;
		
	private Main mMain;

	private String luaDir;

	private byte[] mBuffer;

	private LuaObject mCallback;

	private String luaCpath;

	public LuaAsyncTask(Main main,String src,LuaObject callback) throws LuaException
	{
		mMain=main;
		luaDir=main.luaDir;
		luaCpath=mMain.luaCpath;
		mBuffer=src.getBytes();
		mCallback=callback;
	}
	
	
	public LuaAsyncTask(Main main,LuaObject func,LuaObject callback) throws LuaException
	{
		mMain=main;
		luaDir=main.luaDir;
		luaCpath=mMain.luaCpath;
		mBuffer=func.dump();
		mCallback=callback;
	}


	public void execute(LuaObject params) throws IllegalArgumentException, ArrayIndexOutOfBoundsException, LuaException
	{
		// TODO: Implement this method
		super.execute(params.asArray());
	}
	
	@Override
	protected Object doInBackground(Object[] args) 
	{
		L = LuaStateFactory.newLuaState();
		L.openLibs();
		L.pushJavaObject(mMain);
		L.setGlobal("activity");

		L.getGlobal("luajava");
		L.pushString(luaDir);
		L.setField(-2, "luadir"); 
		L.pop(1);

		try
		{
			JavaFunction print = new LuaPrint(mMain,L);
			print.register("print");

			JavaFunction assetLoader = new LuaAssetLoader(mMain,L); 

			L.getGlobal("package");  
			L.getField(-1, "loaders");
			int nLoaders = L.objLen(-1);
			for (int i=nLoaders;i >= 2;i--)
			{
				L.rawGetI(-1, i);
				L.rawSetI(-2, i + 1);
			}
			L.pushJavaFunction(assetLoader); 
			L.rawSetI(-2, 2);

			L.pop(1);

			L.pushString("./?.lua;" + luaDir + "/?.lua;" + luaDir + "/lua/?.lua;" + luaDir + "/?/init.lua;");
			L.setField(-2, "path");
			L.pushString(luaCpath);
			L.setField(-2, "cpath");
			L.pop(1); 
		}
		catch (LuaException e)
		{
			mMain.sendMsg(e.getMessage());
		}
		try
		{
			L.setTop(0);
			int ok = L.LloadBuffer(mBuffer,"LuaAsyncTask");

			if (ok == 0)
			{
				L.getGlobal("debug");
				L.getField(-1, "traceback");
				L.remove(-2);
				L.insert(-2);
				int l=args.length;
				for (int i=0;i < l;i++)
				{
					L.pushObjectValue(args[i]);
				}
				ok = L.pcall(l, 1, -2 - l);
				if (ok == 0)
				{				
					return L.toJavaObject(-1);
				}
			}
			throw new LuaException(errorReason(ok) + ": " + L.toString(-1));
		} 
		catch (LuaException e)
		{			
			mMain.sendMsg(e.getMessage());
		}


		return null;
	}

	@Override
	protected void onPostExecute(Object result)
	{
		// TODO: Implement this method

		try
		{
			mCallback.call(result);
		}
		catch (LuaException e)
		{
			mMain.sendMsg(e.getMessage());
		}
		super.onPostExecute(result);
		L.gc(LuaState.LUA_GCCOLLECT, 1);
		System.gc();
		//L.close();
	}
	
	private String errorReason(int error)
	{
		switch (error)
		{
			case 4:
				return "Out of memory";
			case 3:
				return "Syntax error";
			case 2:
				return "Runtime error";
			case 1:
				return "Yield error";
		}
		return "Unknown error " + error;
	}
	
}


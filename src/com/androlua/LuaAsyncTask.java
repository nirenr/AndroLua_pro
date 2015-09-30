package com.androlua;

import android.os.*;
import com.luajava.*;
import java.io.*;
import android.view.Window.*;

public class LuaAsyncTask extends AsyncTask
{

	private LuaState L;
	
	private StringBuilder output = new StringBuilder();
	
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
			JavaFunction print = new LuaPrint(L);
			print.register("print");

			JavaFunction assetLoader = new LuaAssetLoader(L); 

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
	
	
	public class LuaAssetLoader extends JavaFunction
	{

		protected LuaState L;

		public LuaAssetLoader(LuaState L)
		{
			super(L);
			this.L = L;
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

	public class LuaPrint extends JavaFunction
	{

		protected LuaState L;

		public LuaPrint(LuaState L)
		{
			super(L);
			this.L = L;
		}

		@Override
		public int execute() throws LuaException
		{
			if (L.getTop() < 2)
			{
				mMain.sendMsg("");
				return 0;
			}
			for (int i = 2; i <= L.getTop(); i++)
			{
				int type = L.type(i);
				String val = null;
				String stype = L.typeName(type);
				if (stype.equals("userdata"))
				{
					Object obj = L.toJavaObject(i);
					if (obj != null)
						val = obj.toString();
				}
				else if (stype.equals("boolean"))
				{
					val = L.toBoolean(i) ? "true" : "false";
				}
				else
				{
					val = L.toString(i);
				}
				if (val == null)
					val = stype;						
				output.append("\t");
				output.append(val);
				output.append("\t");
			}
			mMain.sendMsg(output.toString().substring(1, output.length() - 1));
			output.setLength(0);
			return 0;
		}
	}
	
}


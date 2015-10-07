package com.androlua;

import android.content.res.*;
import android.os.*;
import com.luajava.*;
import java.io.*;
import java.util.regex.*;
import java.util.*;

public class LuaTimerTask extends TimerTask
{
	private LuaState L;
	private String luaDir;
	private Main mMain;

	private String mSrc;

	private Object[] mArg=new Object[0];

	private boolean mEnabled=true;

	private byte[] mBuffer;

	private String luaCpath;

	public LuaTimerTask(Main main, String src) throws LuaException
	{
		this(main, src, null);
	}

	public LuaTimerTask(Main main, String src, Object[] arg) throws LuaException
	{
		mMain = main;
		luaDir = mMain.luaDir;
		mSrc = src;
		luaCpath = mMain.luaCpath;
		if (arg != null)
			mArg = arg;
	}

	public LuaTimerTask(Main main, LuaObject func) throws LuaException
	{
		this(main, func, null);
	}

	public LuaTimerTask(Main main, LuaObject func, Object[] arg) throws LuaException
	{

		mMain = main;
		luaDir = mMain.luaDir;
		luaCpath = mMain.luaCpath;
		if (arg != null)
			mArg = arg;

		mBuffer = func.dump();
	}
	

	@Override
	public void run()
	{
		if (mEnabled == false)
			return;
		try
		{
			if (L == null)
			{
				initLua();

				if (mBuffer != null)
					newLuaThread(mBuffer, mArg);
				else
					newLuaThread(mSrc, mArg);
			}
			else
			{
				L.getGlobal("run");
				if (!L.isNil(-1))
					runFunc("run");
				else
				{
					if (mBuffer != null)
						newLuaThread(mBuffer, mArg);
					else
						newLuaThread(mSrc, mArg);
				}
			}
		}
		catch (LuaException e)
		{
			mMain.sendMsg(e.getMessage());
		}
		L.gc(LuaState.LUA_GCCOLLECT, 1);
		System.gc();
		
	}

	@Override
	public boolean cancel()
	{
		// TODO: Implement this method
		return super.cancel();
	}

	public void setArg(Object[] arg)
	{
		mArg=arg;
	}
	
	public void setArg(LuaObject arg) throws ArrayIndexOutOfBoundsException, LuaException, IllegalArgumentException
	{
		mArg=arg.asArray();
	}
	
	public void setEnabled(boolean enabled)
	{
		mEnabled = enabled;
	}

	public boolean getEnabled()
	{
		return mEnabled;
	}


	public void set(String key, Object value) throws LuaException
	{
		L.pushObjectValue(value);
		L.setGlobal(key);
	}

	public Object get(String key) throws LuaException
	{
		L.getGlobal(key);
		return L.toJavaObject(-1);
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

	private void initLua() throws LuaException
	{
		L = LuaStateFactory.newLuaState();
		L.openLibs();
		L.pushJavaObject(mMain);
		L.setGlobal("activity");

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

		L.pushString(luaDir + "/?.lua;" + luaDir + "/lua/?.lua;" + luaDir + "/?/init.lua;");
		L.setField(-2, "path");
		L.pushString(luaCpath);
		L.setField(-2, "cpath");
		L.pop(1);          

		JavaFunction set = new JavaFunction(L) {
			@Override
			public int execute() throws LuaException
			{

				mMain.set(L.toString(2), L.toJavaObject(3));
				return 0;
			}
		};
		set.register("set");

		JavaFunction call = new JavaFunction(L) {
			@Override
			public int execute() throws LuaException
			{

				int top=L.getTop();
				if (top > 2)
				{
					Object[] args = new Object[top - 2];
					for (int i=3;i <= top;i++)
					{
						args[i - 3] = L.toJavaObject(i);
					}				
					mMain.call(L.toString(2), args);
				}
				else if (top == 2)
				{
					mMain.call(L.toString(2));
				}
				return 0;
			}
		};
		call.register("call");
	}

	private void newLuaThread(String str, Object...args)
	{
		try
		{

			if (Pattern.matches("^\\w+$", str))
			{
				doAsset(str + ".lua", args);
			}
			else if (Pattern.matches("^[\\w\\.\\_/]+$", str))
			{
				L.getGlobal("luajava");
				L.pushString(luaDir);
				L.setField(-2, "luadir"); 
				L.pushString(str);
				L.setField(-2, "luapath"); 
				L.pop(1);

				doFile(str, args);
			}
			else
			{
				doString(str, args);
			}

		}
		catch (Exception e)
		{
			mMain.sendMsg(this.toString() + " " + e.getMessage());

		}

	}
	
	private void newLuaThread(byte[] buf, Object...args) throws LuaException 
	{
		int ok = 0;
		L.setTop(0);
		ok = L.LloadBuffer(buf, "TimerTask");

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
			ok = L.pcall(l, 0, -2 - l);
			if (ok == 0)
			{				
				return;
			}
		}
		throw new LuaException(errorReason(ok) + ": " + L.toString(-1));
	}

	private void doFile(String filePath, Object...args) throws LuaException 
	{
		int ok = 0;
		L.setTop(0);
		ok = L.LloadFile(filePath);

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
			ok = L.pcall(l, 0, -2 - l);
			if (ok == 0)
			{				
				return;
			}
		}
		throw new LuaException(errorReason(ok) + ": " + L.toString(-1));
	}

	public void doAsset(String name, Object...args) throws LuaException, IOException 
	{
		int ok = 0;
		byte[] bytes = mMain.readAsset(name);
		L.setTop(0);
		ok = L.LloadBuffer(bytes, name);

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
			ok = L.pcall(l, 0, -2 - l);
			if (ok == 0)
			{				
				return;
			}
		}
		throw new LuaException(errorReason(ok) + ": " + L.toString(-1));
	}

	private void doString(String src, Object...args) throws LuaException
	{			
		L.setTop(0);
		int ok = L.LloadString(src);

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
			ok = L.pcall(l, 0, -2 - l);
			if (ok == 0)
			{				

				return;
			}
		}
		throw new LuaException(errorReason(ok) + ": " + L.toString(-1));
	}


	private void runFunc(String funcName, Object...args)
	{
		try
		{
			L.setTop(0);
			L.getGlobal(funcName);
			if (L.isFunction(-1))
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

				int ok = L.pcall(l, 1, -2 - l);
				if (ok == 0)
				{				
					return ;
				}
				throw new LuaException(errorReason(ok) + ": " + L.toString(-1));
			}
		}
		catch (LuaException e)
		{
			mMain.sendMsg(funcName + " " + e.getMessage());
		}

	}

	private void setField(String key, Object value)
	{
		try
		{
			L.pushObjectValue(value);
			L.setGlobal(key);
		}
		catch (LuaException e)
		{
			mMain.sendMsg(e.getMessage());
		}
	}

};

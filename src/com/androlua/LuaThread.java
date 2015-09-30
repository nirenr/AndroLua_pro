package com.androlua;

import android.content.res.*;
import android.os.*;
import com.luajava.*;
import java.io.*;
import java.util.regex.*;

public class LuaThread extends Thread implements Runnable
{
	private LuaState L;
	private Handler thandler;
	public boolean isRun = false;
	private String luaDir;
	private Main mMain;

	private boolean mIsLoop;

	private String mSrc;

	private Object[] mArg=new Object[0];

	private byte[] mBuffer;

	private String luaCpath;

	public LuaThread(Main main, String src) throws LuaException
	{
		this(main, src, false,null);
	}

	public LuaThread(Main main, String src, Object[] arg) throws LuaException
	{
		this(main, src, false, arg);
	}

	public LuaThread(Main main, String src, boolean isLoop) throws LuaException
	{
		this(main, src, isLoop, null);
	}

	public LuaThread(Main main, String src, boolean isLoop, Object[] arg) throws LuaException
	{
		mMain = main;
		luaDir = mMain.luaDir;
		luaCpath=mMain.luaCpath;
		mSrc = src;
		mIsLoop = isLoop;
		if (arg != null)
			mArg = arg;
	}
	public LuaThread(Main main, LuaObject func) throws LuaException
	{
		this(main, func, false,null);
	}
	public LuaThread(Main main, LuaObject func, Object[] arg) throws LuaException
	{
		this(main, func, false,arg);
	}
	public LuaThread(Main main, LuaObject func, boolean isLoop) throws LuaException
	{
		this(main, func, false,null);
	}
	
	public LuaThread(Main main, LuaObject func, boolean isLoop, Object[] arg) throws LuaException
	{
		mMain = main;
		luaDir = mMain.luaDir;
		luaCpath=mMain.luaCpath;
		if (arg != null)
			mArg = arg;
		mIsLoop = isLoop;
		mBuffer = func.dump();
	}

	@Override
	public void run()
	{
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
			return;
		}
		if (mIsLoop)
		{
			Looper.prepare();
			thandler = new ThreadHandler();
			isRun = true;
			Looper.loop();
			isRun = false;
		}
		L.gc(LuaState.LUA_GCCOLLECT, 1);
		System.gc();
		return ;
	}

	public void call(String func)
	{
		push(3, func);
	}

	public void call(String func, Object[] args)
	{
		if (args.length == 0)
			push(3, func);
		else
			push(1, func, args);
	}

	public void set(String key, Object value)
	{
		push(4, key, new Object[]{ value});
	}

	public Object get(String key) throws LuaException
	{
		L.getGlobal(key);
		return L.toJavaObject(-1);
	}

	public void quit()
	{
		if (isRun)
			thandler.getLooper().quit();
	}

	public void push(int what, String s)
	{
		if (!isRun)
		{
			mMain.sendMsg("thread is not running");
			return;
		}

		Message message = new Message();
		Bundle bundle = new Bundle();
		bundle.putString("data", s);
		message.setData(bundle);  
		message.what = what;

		thandler.sendMessage(message);

	}

	public void push(int what, String s, Object[] args)
	{
		if (!isRun)
		{
			mMain.sendMsg("thread is not running");
			return;
		}

		Message message = new Message();
		Bundle bundle = new Bundle();
		bundle.putString("data", s);
		bundle.putSerializable("args", args);
		message.setData(bundle);  
		message.what = what;

		thandler.sendMessage(message);

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
		L.pushJavaObject(this);
		L.setGlobal("thread");

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
			quit();
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

	private class ThreadHandler extends Handler
	{
		@Override 
		public void handleMessage(Message msg)
		{ 
			super.handleMessage(msg);
			Bundle data=msg.getData();
			switch (msg.what)
			{
				case 0:
					newLuaThread(data.getString("data"), (Object[])data.getSerializable("args"));
					break;
				case 1:
					runFunc(data.getString("data"), (Object[])data.getSerializable("args"));
					break;
				case 2:
					newLuaThread(data.getString("data"));
					break;
				case 3:
					runFunc(data.getString("data"));
					break;
				case 4:
					setField(data.getString("data"), ((Object[])data.getSerializable("args"))[0]);
					break;
			}
		}
	};

};

package com.androlua;

import android.content.res.*;
import android.os.*;
import com.luajava.*;
import java.io.*;
import java.util.regex.*;

public class LuaRunnable implements Runnable
{
	protected LuaState L;
	private Handler thandler;
	private boolean isRun = false;
	private LuaContext mLuaContext;

	private boolean mIsLoop;

	private String mSrc;

	private Object[] mArg=new Object[0];

	public LuaRunnable(LuaContext luaContext, String src) throws LuaException
	{
		this(luaContext,src,new Object[0],false);
	}
	
	public LuaRunnable(LuaContext luaContext, String src,Object[] arg) throws LuaException
	{
		this(luaContext,src,arg,false);
	}
	
	public LuaRunnable(LuaContext luaContext, String src, boolean isLoop) throws LuaException
	{
		this(luaContext,src,new Object[0],isLoop);
	}
	
	public LuaRunnable(LuaContext luaContext, String src,Object[] arg, boolean isLoop) throws LuaException
	{
		mLuaContext = luaContext;
		mSrc = src;
		mIsLoop = isLoop;
		mArg=arg;
		L = LuaStateFactory.newLuaState();
		L.openLibs();
		L.pushJavaObject(mLuaContext);
		if(mLuaContext instanceof LuaActivity)
		{
			L.setGlobal("activity");
		}
		else if(mLuaContext instanceof LuaService)
		{
			L.setGlobal("service");
		}
		L.pushJavaObject(this);
		L.setGlobal("this");
		L.pushContext(mLuaContext.getContext());
		initJavaFunction();
	}
	
	@Override
	public void run()
	{
		newLuaThread(mSrc,mArg);
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
			mLuaContext.sendMsg("thread is not running");
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
			mLuaContext.sendMsg("thread is not running");
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
			case 6:
				return "error error";
			case 5:
				return "GC error";
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

	private void initJavaFunction() throws LuaException
	{
		JavaFunction print = new LuaPrint(mLuaContext,L);
		print.register("print");

		L.getGlobal("package");
		L.pushString(mLuaContext.getLuaLpath());
		L.setField(-2, "path");
		L.pushString(mLuaContext.getLuaCpath());
		L.setField(-2, "cpath");
		L.pop(1);          

		JavaFunction set = new JavaFunction(L) {
			@Override
			public int execute() throws LuaException
			{

				mLuaContext.set(L.toString(2), L.toJavaObject(3));
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
					mLuaContext.call(L.toString(2), args);
				}
				else if (top == 2)
				{
					mLuaContext.call(L.toString(2));
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
				L.pushString(mLuaContext.getLuaDir());
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
			mLuaContext.sendMsg(this.toString() + " " + e.getMessage());
			quit();
		}

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
		byte[] bytes = LuaUtil.readAsset(mLuaContext.getContext(),name);
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
			mLuaContext.sendMsg(funcName + " " + e.getMessage());
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
			mLuaContext.sendMsg(e.getMessage());
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

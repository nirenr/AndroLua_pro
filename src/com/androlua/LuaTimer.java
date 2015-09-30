package com.androlua;

import com.luajava.*;
import java.util.*;

public class LuaTimer extends Timer
{

	private LuaTimerTask task;
	
	public LuaTimer(Main main,String src) throws LuaException
	{
		this(main,src,null);
	}
	public LuaTimer(Main main,String src,Object[] arg) throws LuaException
	{
		super("LuaTimer");
		task= new LuaTimerTask(main, src,arg);
	}
	
	public LuaTimer(Main main,LuaObject func) throws LuaException
	{
		this(main,func,null);
	}
	public LuaTimer(Main main,LuaObject func,Object[] arg) throws LuaException
	{
		super("LuaTimer");
		task= new LuaTimerTask(main, func, arg);
	}
	
	public void start(long delay, long period)
	{
		schedule(task,delay,period);
	}
	
	public void start(long delay)
	{
		schedule(task,delay);
	}
	
	public void stop()
	{
		task.cancel();
	}
	
	public void setEnabled(boolean enabled)
	{
		task.setEnabled(enabled);
	}

	public boolean getEnabled()
	{
		return task.getEnabled();
	}
	
}

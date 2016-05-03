package com.androlua;

import android.content.*;
import com.luajava.*;

public interface LuaContext
{

	public void call(String func, Object...args);


	public void set(String name, Object value);

	public String getLuaDir();
	
	public String getLuaExtDir();
	
	public String getLuaLpath();
	
	public String getLuaCpath();
	
	public Context getContext();
	
	public LuaState getLuaState();
	
	public Object doFile(String path,Object...arg);
	
	public void sendMsg(String msg);
}

package com.androlua;

import android.content.*;
import com.luajava.*;
import java.util.*;

public interface LuaContext
{

	public ArrayList<ClassLoader> getClassLoaders();
	
	public void call(String func, Object...args);

	public void set(String name, Object value);

	public String getLuaDir();
	
	public String getLuaDir(String dir);
	
	public String getLuaExtDir();
	
	public String getLuaExtDir(String dir);
	
	public String getLuaLpath();
	
	public String getLuaCpath();
	
	public Context getContext();
	
	public LuaState getLuaState();
	
	public Object doFile(String path,Object...arg);
	
	public void sendMsg(String msg);
	
	public int getWidth();
	
	public int getHeight();
	
	public void regGc(LuaGcable obj);
	
}

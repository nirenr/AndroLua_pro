package com.androlua;

import android.app.*;
import android.content.*;
import android.content.pm.*;
import android.os.*;
import android.widget.*;
import com.luajava.*;
import java.io.*;
import java.util.*;
import java.util.zip.*;
import android.util.*;  
 
public class LuaApplication extends Application implements LuaContext
{

	private static LuaApplication mApp;
	
	public static LuaApplication getInstance(){
		return mApp;
	}
	
	@Override
	public ArrayList<ClassLoader> getClassLoaders() {
		// TODO: Implement this method
		return null;
	}


	@Override
	public void regGc(LuaGcable obj) {
		// TODO: Implement this method
	}

	public int getWidth()
	{
		return getResources().getDisplayMetrics().widthPixels;
	}

	public int getHeight()
	{
		return getResources().getDisplayMetrics().heightPixels;
	}
	
	@Override
	public String getLuaDir(String dir)
	{
		// TODO: Implement this method
		return null;
	}


	@Override
	public String getLuaExtDir(String dir)
	{
		// TODO: Implement this method
		return null;
	}


	
	private boolean isUpdata;

	protected String localDir;

	protected String odexDir;

	protected String libDir;

	protected String luaMdDir;

	protected String luaCpath;

	protected String luaLpath;
	
	protected String luaExtDir;

	public String getLibDir()
	{
		// TODO: Implement this method
		return libDir;
	}

	public String getOdexDir()
	{
		// TODO: Implement this method
		return odexDir;
	} 
	
	
    @Override  
    public void onCreate() {
		super.onCreate();  
		mApp=this;
		CrashHandler crashHandler = CrashHandler.getInstance();  
		// 注册crashHandler  
		crashHandler.init(getApplicationContext());
		
		//初始化AndroLua工作目录
		if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED))
		{
			String sdDir = Environment.getExternalStorageDirectory().getAbsolutePath();
			luaExtDir = sdDir + "/AndroLua";
		}
		else
		{
			File[] fs= new File("/storage").listFiles();
			for (File f:fs)
			{
				String[] ls=f.list();
				if (ls == null)
					continue;
				if (ls.length > 5)
					luaExtDir = f.getAbsolutePath() + "/AndroLua";
			}
			if (luaExtDir == null)
				luaExtDir = getDir("AndroLua", Context.MODE_PRIVATE).getAbsolutePath();
		}

		File destDir = new File(luaExtDir);
		if (!destDir.exists())
			destDir.mkdirs();

		//定义文件夹
		localDir = getFilesDir().getAbsolutePath();
		odexDir = getDir("odex", Context.MODE_PRIVATE).getAbsolutePath();
		libDir = getDir("lib", Context.MODE_PRIVATE).getAbsolutePath();
		luaMdDir = getDir("lua", Context.MODE_PRIVATE).getAbsolutePath();
		luaCpath = getApplicationInfo().nativeLibraryDir + "/lib?.so" + ";" + libDir + "/lib?.so";
		//luaDir = extDir;
		luaLpath = luaMdDir + "/?.lua;" + luaMdDir + "/lua/?.lua;" + luaMdDir + "/?/init.lua;";
		//checkInfo();
	}
	
	@Override
	public String getLuaDir()
	{
		// TODO: Implement this method
		return localDir;
	}

	static private HashMap<String,Object> data=new HashMap<String,Object>();

	@Override
	public void call(String name, Object[] args)
	{
		// TODO: Implement this method
	}

	@Override
	public void set(String name, Object object)
	{
		// TODO: Implement this method
		data.put(name,object);
	}

	@Override
	public Map getGlobalData(){
		return data;
	}


	public Object get(String name)
	{
		// TODO: Implement this method
		return data.get(name);
	}
	
	public String getLocalDir()
	{
		// TODO: Implement this method
		return localDir;
	}
	
	
	public String getMdDir()
	{
		// TODO: Implement this method
		return luaMdDir;
	}
	
	@Override
	public String getLuaExtDir()
	{
		// TODO: Implement this method
		return luaExtDir;
	}

	@Override
	public String getLuaLpath()
	{
		// TODO: Implement this method
		return luaLpath;
	}

	@Override
	public String getLuaCpath()
	{
		// TODO: Implement this method
		return luaCpath;
	}

	@Override
	public Context getContext()
	{
		// TODO: Implement this method
		return this;
	}

	@Override
	public LuaState getLuaState()
	{
		// TODO: Implement this method
		return null;
	}

	@Override
	public Object doFile(String path, Object[] arg)
	{
		// TODO: Implement this method
		return null;
	}

	@Override
	public void sendMsg(String msg)
	{
		// TODO: Implement this method
		Toast.makeText(this,msg,500).show();
		
	}

	@Override
	public void sendError(String title, Exception msg) {

	}


} 




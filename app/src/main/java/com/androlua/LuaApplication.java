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

/** 
 * 在开发应用时都会和Activity打交道，而Application使用的就相对较少了。 
 * Application是用来管理应用程序的全局状态的，比如载入资源文件。 
 * 在应用程序启动的时候Application会首先创建，然后才会根据情况(Intent)启动相应的Activity或者Service。 
 * 在本文将在Application中注册未捕获异常处理器。 
 */  
public class LuaApplication extends Application implements LuaContext
{

	
	private boolean isUpdata;

	private String localDir;

	private String odexDir;

	private String libDir;

	private String luaMdDir;

	private String luaCpath;

	private String luaLpath;
	
	private String luaExtDir;

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
		checkInfo();
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
	

	public void checkInfo()
	{
		try
		{
			PackageInfo packageInfo=getPackageManager().getPackageInfo(this.getPackageName(), 0);
			long lastTime=packageInfo.lastUpdateTime;
			String versionName=packageInfo.versionName;
			SharedPreferences info=getSharedPreferences("appInfo", 0);
			long oldLastTime=info.getLong("lastUpdateTime", 0);
			if (oldLastTime != lastTime)
			{
				SharedPreferences.Editor edit=info.edit();
				edit.putLong("lastUpdateTime", lastTime);
				edit.commit();
				onUpdata(lastTime, oldLastTime);
			}
			String oldVersionName=info.getString("versionName", "");
			if (!versionName.equals(oldVersionName))
			{
				SharedPreferences.Editor edit=info.edit();
				edit.putString("versionName", versionName);
				edit.commit();
				onVersionChanged(versionName, oldVersionName);
			}
		}
		catch (PackageManager.NameNotFoundException e)
		{

		}

	}


	private void onVersionChanged(String versionName, String oldVersionName)
	{
		// TODO: Implement this method
		//runFunc("onVersionChanged", versionName, oldVersionName);
	}

	private void onUpdata(long lastTime, long oldLastTime)
	{
		isUpdata = true;
		try
		{
			LuaUtil.rmDir(new File(localDir),".lua");
			LuaUtil.rmDir(new File(luaMdDir),".lua");

			unApk("assets", localDir);
			unApk("lua", luaMdDir);
			//unZipAssets("main.alp", extDir);
		}
		catch (IOException e)
		{
			sendMsg(e.getMessage());
		}
	}

	private void unApk(String dir, String extDir) throws IOException
	{
		int i=dir.length() + 1;
		ZipFile zip=new ZipFile(getApplicationInfo().publicSourceDir);
		Enumeration<? extends ZipEntry> entries=zip.entries();
		while (entries.hasMoreElements())
		{
			ZipEntry entry=entries.nextElement();
			String name=entry.getName();
			if (name.indexOf(dir) != 0)
				continue;
			String path=name.substring(i);
			if (entry.isDirectory())
			{
				File f=new File(extDir + File.separator + path);
				if (!f.exists())
					f.mkdirs();
			}
			else
			{
				File temp=new File(extDir + File.separator + path).getParentFile();
                if (!temp.exists())
				{
                    if (!temp.mkdirs())
					{
                        throw new RuntimeException("create file " + temp.getName() + " fail");
                    }
                }

				FileOutputStream out=new FileOutputStream(extDir + File.separator + path);
				InputStream in=zip.getInputStream(entry);
				byte[] buf=new byte[4096];
				int count=0;
				while ((count = in.read(buf)) != -1)
				{
					out.write(buf, 0, count);
				}
				out.close();
				in.close();
			}
		}
		zip.close();
	}
	
	
} 




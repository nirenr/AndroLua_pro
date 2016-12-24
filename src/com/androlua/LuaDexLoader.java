package com.androlua;

import android.content.*;
import android.content.res.*;
import com.luajava.*;
import dalvik.system.*;
import java.io.*;
import java.lang.reflect.*;
import java.util.*;

public class LuaDexLoader {
	private static HashMap<String,LuaDexClassLoader> dexCache=new HashMap<String,LuaDexClassLoader>();
	private ArrayList<ClassLoader> dexList=new ArrayList<ClassLoader>();
	private HashMap<String,String> libCache=new HashMap<String,String>();
	
	private LuaContext mContext;

	private String luaDir;

	private AssetManager mAssetManager;

	private Resources mResources;

	private Resources.Theme mTheme;

	private String odexDir;
	public LuaDexLoader(LuaContext context) {
		mContext = context;
		luaDir = context.getLuaDir();
		LuaApplication app=LuaApplication.getInstance();
		//localDir = app.getLocalDir();
		odexDir = app.getOdexDir();
	}

	public ArrayList<ClassLoader> getClassLoaders() {
		// TODO: Implement this method
		return dexList;
	}

	public void loadLibs() throws LuaException{
		File[] libs=new File(mContext.getLuaDir() + "/libs").listFiles();
		if(libs==null)
			return;
		for(File f:libs){
			if(f.getAbsolutePath().endsWith(".so"))
				loadLib(f.getName());
			else
				loadDex(f.getAbsolutePath());
		}
	}
	
	public void loadLib(String name) throws LuaException {
		String fn = name;
		int i=name.indexOf(".");
		if (i > 0)
			fn = name.substring(0, i);
		if(fn.startsWith("lib"))
			fn=fn.substring(3);
		String libDir=mContext.getContext().getDir(fn, Context.MODE_PRIVATE).getAbsolutePath();
		String libPath=libDir + "/lib" + fn + ".so";
		File f=new File(libPath);
		if (!f.exists()) {
			f = new File(luaDir + "/libs/lib" + fn + ".so");
			if (!f.exists())
				throw new LuaException("can not find lib " + name);
			LuaUtil.copyFile(luaDir + "/libs/lib" + fn + ".so", libPath);
			
		}
		libCache.put(fn,libPath);
	}

	public HashMap<String,String> getLibrarys(){
		return libCache;
	}
	
	
	public DexClassLoader loadDex(String path) throws LuaException {
		String name=path;
		LuaDexClassLoader dex=dexCache.get(name);
		if (dex == null) {
			if (path.charAt(0) != '/')
				path = luaDir + "/" + path;
			if (!new File(path).exists())
				if (new File(path + ".dex").exists())
					path += ".dex";
				else
				if (new File(path + ".jar").exists())
					path += ".jar";
				else
					throw new LuaException(path + " not found");
			dex = new LuaDexClassLoader(path, odexDir, LuaApplication.getInstance().getApplicationInfo().nativeLibraryDir, mContext.getContext().getClassLoader());
			dexCache.put(name, dex);
		}
		
			
		if(!dexList.contains(dex)){
			dexList.add(dex);
			path=dex.getDexPath();
			if (path.endsWith(".jar"))
				loadResources(path);
		}
		return dex;
	}

	public void loadResources(String path) {  
		try {
			AssetManager assetManager = AssetManager.class.newInstance();  
			Method addAssetPath = assetManager.getClass().getMethod("addAssetPath", String.class); 
			int ok=(int)addAssetPath.invoke(assetManager, path);
			if (ok == 0)
				return;
			mAssetManager = assetManager;  
			Resources superRes = mContext.getContext().getResources();  
			mResources = new Resources(mAssetManager, superRes.getDisplayMetrics(),  
									   superRes.getConfiguration());  
			mTheme = mResources.newTheme();  
			mTheme.setTo(mContext.getContext().getTheme());  
		}
		catch (Exception e) {  
			e.printStackTrace();  
		} 
	} 

	public AssetManager getAssets() {  
		return mAssetManager;  
	} 

	public Resources getResources() {  
		return mResources;  
	} 

}

package com.androlua;

import android.content.*;

import com.luajava.*;

import java.util.*;

public interface LuaContext {

    public ArrayList<ClassLoader> getClassLoaders();

    public void call(String func, Object... args);

    public void set(String name, Object value);

    public String getLuaPath();

    public String getLuaPath(String path);

    public String getLuaPath(String dir, String name);

    public String getLuaDir();

    public String getLuaDir(String dir);

    public String getLuaExtDir();

    public String getLuaExtDir(String dir);

    public void setLuaExtDir(String dir);

    public String getLuaExtPath(String path);

    public String getLuaExtPath(String dir, String name);

    public String getLuaLpath();

    public String getLuaCpath();

    public Context getContext();

    public LuaState getLuaState();

    public Object doFile(String path, Object... arg);

    public void sendMsg(String msg);

    public void sendError(String title, Exception msg);

    public int getWidth();

    public int getHeight();

    public Map getGlobalData();

    public Object getSharedData(String key);
    public Object getSharedData(String key,Object def);
    public boolean setSharedData(String key, Object value);

    public void regGc(LuaGcable obj);

}

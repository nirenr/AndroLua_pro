package com.androlua;

import android.annotation.SuppressLint;
import android.app.Notification;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.AssetManager;
import android.content.res.Resources;
import android.net.Uri;
import android.os.Binder;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.util.Log;
import android.widget.Toast;

import com.luajava.JavaFunction;
import com.luajava.LuaException;
import com.luajava.LuaObject;
import com.luajava.LuaState;
import com.luajava.LuaStateFactory;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import dalvik.system.DexClassLoader;

public class LuaService extends Service implements LuaContext,LuaBroadcastReceiver.OnReceiveListener {

	private static LuaService _this;
	public String luaDir;
	public String luaCpath;
	LuaBinder mBinder=new LuaBinder();
	private LuaDexLoader mLuaDexLoader;
	private ArrayList<LuaGcable> gclist=new ArrayList<LuaGcable>();
	private String luaLpath;
	private LuaService.MainHandler handler;
	private String luaMdDir;
	private LuaState L;
	private String luaPath;
	private String localDir;
	private String odexDir;
	private String libDir;
	private String luaExtDir;
	private BroadcastReceiver mReceiver;
	private StringBuilder output = new StringBuilder();
	private Toast toast;
	private StringBuilder toastbuilder = new StringBuilder();
	private long lastShow;
	private LuaResources mResources;

	private static byte[] readAll(InputStream input) throws IOException {
		ByteArrayOutputStream output = new ByteArrayOutputStream(4096);
		byte[] buffer = new byte[4096];
		int n = 0;
		while (-1 != (n = input.read(buffer))) {
			output.write(buffer, 0, n);
		}
		byte[] ret= output.toByteArray();
		output.close();
		return ret;
	}

	public static LuaService getService() {
		return _this;
	}

	@Override
	public ArrayList<ClassLoader> getClassLoaders() {
		// TODO: Implement this method
		return mLuaDexLoader.getClassLoaders();
	}

	public DexClassLoader loadDex(String path) throws LuaException {
		return mLuaDexLoader.loadDex(path);
	}

	public HashMap<String,String> getLibrarys(){
		return mLuaDexLoader.getLibrarys();
	}

	public void loadResources(String path) {
		mLuaDexLoader.loadResources(path);
	}

	@Override
	public AssetManager getAssets() {
		if (mLuaDexLoader != null && mLuaDexLoader.getAssets() != null)
			return mLuaDexLoader.getAssets();
		return super.getAssets();
	}

	public LuaResources getLuaResources() {
		Resources superRes = super.getResources();
		if (mLuaDexLoader != null && mLuaDexLoader.getResources() != null)
			superRes = mLuaDexLoader.getResources();
		mResources = new LuaResources(getAssets(), superRes.getDisplayMetrics(),
				superRes.getConfiguration());
		mResources.setSuperResources(superRes);
		return mResources;
	}

	public Resources getSuperResources() {
		return super.getResources();
	}

	@Override
	public Resources getResources() {
		if (mLuaDexLoader != null && mLuaDexLoader.getResources() != null)
			return mLuaDexLoader.getResources();
		if(mResources!=null)
			return mResources;
		return super.getResources();
	}

	public Intent registerReceiver(LuaBroadcastReceiver receiver, IntentFilter filter) {
		// TODO: Implement this method
		return super.registerReceiver(receiver, filter);
	}

	public Intent registerReceiver(LuaBroadcastReceiver.OnReceiveListener ltr, IntentFilter filter) {
		// TODO: Implement this method
		LuaBroadcastReceiver receiver=new LuaBroadcastReceiver(ltr);
		return super.registerReceiver(receiver, filter);
	}

	public Intent registerReceiver(IntentFilter filter) {
		// TODO: Implement this method
		if (mReceiver != null)
			unregisterReceiver(mReceiver);
		mReceiver = new LuaBroadcastReceiver(this);
		return super.registerReceiver(mReceiver, filter);
	}

	@Override
	public void onReceive(Context context, Intent intent) {
		// TODO: Implement this method
		runFunc("onReceive", context, intent);
	}

	@Override
	public void regGc(LuaGcable obj) {
		// TODO: Implement this method
		gclist.add(obj);
	}

	@Override
	public String getLuaPath() {
        // TODO: Implement this method
        return luaPath;
    }

	@Override
	public String getLuaPath(String path) {
        return new File(getLuaDir(),path).getAbsolutePath();
    }

	@Override
	public String getLuaPath(String dir, String name) {
        return new File(getLuaDir(dir),name).getAbsolutePath();
    }

	@Override
	public String getLuaExtPath(String path) {
        return new File(getLuaExtDir(),path).getAbsolutePath();
    }

	@Override
	public String getLuaExtPath(String dir, String name) {
        return new File(getLuaExtDir(dir),name).getAbsolutePath();
    }

	@Override
	public String getLuaDir(String name) {
		// TODO: Implement this method
		File dir=new File(luaDir + "/" + name);
		if (!dir.exists())
			if (!dir.mkdirs())
				return null;
		return dir.getAbsolutePath();
	}

	@Override
	public String getLuaExtDir(String name) {
		// TODO: Implement this method
		File dir=new File(luaExtDir + "/" + name);
		if (!dir.exists())
			if (!dir.mkdirs())
				return null;
		return dir.getAbsolutePath();
	}

	@Override
	public void setLuaExtDir(String dir) {
		if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
			String sdDir = Environment.getExternalStorageDirectory().getAbsolutePath();
			luaExtDir = new File(sdDir , dir).getAbsolutePath();
		} else {
			File[] fs = new File("/storage").listFiles();
			for (File f : fs) {
				String[] ls = f.list();
				if (ls == null)
					continue;
				if (ls.length > 5)
					luaExtDir = new File(f, dir).getAbsolutePath() ;
			}
			if (luaExtDir == null)
				luaExtDir = getDir(dir, Context.MODE_PRIVATE).getAbsolutePath();
		}
		File d = new File(luaExtDir);
		if (!d.exists())
			d.mkdirs();
	}

    @Override
	public String getLuaDir() {
		// TODO: Implement this method
		return luaDir;
	}

	@Override
	public String getLuaExtDir() {
		// TODO: Implement this method
		return luaExtDir;
	}

	@Override
	public String getLuaLpath() {
		// TODO: Implement this method
		return luaLpath;
	}

	@Override
	public String getLuaCpath() {
		// TODO: Implement this method
		return luaCpath;
	}

	@Override
	public Context getContext() {
		// TODO: Implement this method
		return this;
	}

	@Override
	public LuaState getLuaState() {
		// TODO: Implement this method
		return L;
	}

	public LuaBinder getBinder() {
		return mBinder;
	}

	public void setBinder(LuaBinder mBinder) {
		this.mBinder = mBinder;
	}

	@Override
	public IBinder onBind(Intent p1) {
		// TODO: Implement this method
		startForeground(1, new Notification());
		return new LuaBinder();
	}

	@Override
	public void onCreate() {
		// TODO: Implement this method
		super.onCreate();
		_this = LuaService.this;
		//定义文件夹
		LuaApplication app=(LuaApplication) getApplication();
		localDir = app.getLocalDir();
		odexDir = app.getOdexDir();
		libDir = app.getLibDir();
		luaMdDir = app.getMdDir();
		luaCpath = app.getLuaCpath();
		luaDir = localDir;
		luaLpath = app.getLuaLpath();
		luaExtDir = app.getLuaExtDir();

		handler = new MainHandler();
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		// TODO: Implement this method
		_this = LuaService.this;
		if (L == null) {
			startForeground(1, new Notification());
			luaPath = intent.getStringExtra("luaPath");
			luaDir = intent.getStringExtra("luaDir");
			luaLpath = (luaDir + "/?.lua;" + luaDir + "/lua/?.lua;" + luaDir + "/?/init.lua;") + luaLpath;

			Uri uri=intent.getData();
			try {
				initLua();
				mLuaDexLoader = new LuaDexLoader(this);
				mLuaDexLoader.loadLibs();

				if (uri != null)
					doFile(uri.getPath());
				else
					doFile("service.lua");

			}
			catch (Exception e) {
				sendMsg(e.getMessage());
			}


		}
		runFunc("onStartCommand", intent, flags, startId);
		runFunc("onStart", (Object[])intent.getSerializableExtra("arg"));
		return super.onStartCommand(intent, flags, startId);
	}

	@Override
	public boolean onUnbind(Intent intent) {
		// TODO: Implement this method
		return super.onUnbind(intent);
	}

	@Override
	public void onDestroy() {
		// TODO: Implement this method
		runFunc("onDestroy");
		if(mReceiver!=null)
		unregisterReceiver(mReceiver);
		super.onDestroy();
	}


	public LuaAsyncTask newTask(LuaObject func) throws LuaException {
		return newTask(func, null, null);
	}

	public LuaAsyncTask newTask(LuaObject func, LuaObject callback) throws LuaException {
		return newTask(func, null, callback);
	}

	public LuaAsyncTask newTask(LuaObject func, LuaObject update, LuaObject callback) throws LuaException {
		return new LuaAsyncTask(this, func, update, callback);
	}

	public LuaThread newThread(LuaObject func) throws LuaException {
		return newThread(func, null);
	}

	public LuaThread newThread(LuaObject func, Object[] arg) throws LuaException {
		LuaThread thread = new LuaThread(this, func, true, arg);
		return thread;
	}

	public LuaTimer newTimer(LuaObject func) throws LuaException {
		return newTimer(func, null);
	}

	public LuaTimer newTimer(LuaObject func, Object[] arg) throws LuaException {
		return new LuaTimer(this, func, arg);
	}

	public LuaAsyncTask task(long delay,LuaObject func) throws LuaException {
		return task(delay,null,null);
	}

	public LuaAsyncTask task(long delay, Object[] arg, LuaObject func) throws LuaException {
		LuaAsyncTask task = new LuaAsyncTask(this, delay, func);
		task.execute(arg);
		return task;
	}

	public LuaAsyncTask task(LuaObject func) throws LuaException {
		return task(func,null,null,null);
	}

	public LuaAsyncTask task(LuaObject func, Object[] arg) throws LuaException {
		return task(func, arg,null,null);
	}

	public LuaAsyncTask task(LuaObject func, Object[] arg, LuaObject callback) throws LuaException {
		return task(func,null,null,callback);
	}

	public LuaAsyncTask task(LuaObject func,LuaObject update, LuaObject callback) throws LuaException {
		return task(func,null,update,callback);
	}

	public LuaAsyncTask task(LuaObject func, Object[] arg, LuaObject update, LuaObject callback) throws LuaException {
		LuaAsyncTask task = new LuaAsyncTask(this, func, update, callback);
		task.execute(arg);
		return task;
	}

	public LuaThread thread(LuaObject func) throws LuaException {
		LuaThread thread = newThread(func, null);
		thread.start();
		return thread;
	}

	public LuaThread thread(LuaObject func, Object[] arg) throws LuaException {
		LuaThread thread = new LuaThread(this, func, true, arg);
		thread.start();
		return thread;
	}

	public LuaTimer timer(LuaObject func, long period) throws LuaException {
		return timer(func, 0, period, null);
	}

	public LuaTimer timer(LuaObject func, long period, Object[] arg) throws LuaException {
		return timer(func, 0, period, arg);
	}

	public LuaTimer timer(LuaObject func, long delay, long period) throws LuaException {
		return timer(func, delay, period, null);
	}

	public LuaTimer timer(LuaObject func, long delay, long period, Object[] arg) throws LuaException {
		LuaTimer timer = new LuaTimer(this, func, arg);
		timer.start(delay, period);
		return timer;
	}

	public Ticker ticker(final LuaObject func, long period) throws LuaException {
		Ticker timer = new Ticker();
		timer.setOnTickListener(new Ticker.OnTickListener() {
			@Override
			public void onTick() {
				try {
					func.call();
				} catch (LuaException e) {
					e.printStackTrace();
					sendError("onTick",e);
				}
			}
		});
		timer.setPeriod(period);
		timer.start();
		return timer;
	}

	public int getWidth() {
		return getResources().getDisplayMetrics().widthPixels;
	}

	public int getHeight() {
		return getResources().getDisplayMetrics().heightPixels;
	}

	@Override
	public Map getGlobalData() {
		return LuaApplication.getInstance().getGlobalData();
	}

	@Override
	public Object getSharedData(String key) {
		return LuaApplication.getInstance().getSharedData(key);
	}

	@Override
	public Object getSharedData(String key, Object def) {
		return LuaApplication.getInstance().getSharedData(key,def);
	}

    @Override
	public boolean setSharedData(String key, Object value) {
		return LuaApplication.getInstance().setSharedData(key,value);
	}


	//初始化lua使用的Java函数
	private void initLua() throws Exception {
		L = LuaStateFactory.newLuaState();
		L.openLibs();
		L.pushJavaObject(this);
		L.setGlobal("service");
		L.getGlobal("service");
		L.setGlobal("this");
		L.pushContext(this);


		L.getGlobal("luajava");
		L.pushString(luaExtDir);
		L.setField(-2, "luaextdir");
		L.pushString(luaDir);
		L.setField(-2, "luadir");
		L.pushString(luaPath);
		L.setField(-2, "luapath");
		L.pop(1);

		JavaFunction assetLoader = new LuaAssetLoader(this, L);

		L.getGlobal("package");
		L.pushString(luaLpath);
		L.setField(-2, "path");
		L.pushString(luaCpath);
		L.setField(-2, "cpath");
		L.pop(1);

		JavaFunction print=new JavaFunction(L) {

			@Override
			public int execute() throws LuaException {
				if (L.getTop() < 2) {
					sendMsg("");
					return 0;
				}
				for (int i = 2; i <= L.getTop(); i++) {
					int type = L.type(i);
					String val = null;
					String stype = L.typeName(type);
					switch (stype) {
						case "userdata":
							Object obj = L.toJavaObject(i);
							if (obj != null)
								val = obj.toString();
							break;
						case "boolean":
							val = L.toBoolean(i) ? "true" : "false";
							break;
						default:
							val = L.toString(i);
							break;
					}
					if (val == null)
						val = stype;
					output.append("\t");
					output.append(val);
					output.append("\t");
				}
				sendMsg(output.toString().substring(1, output.length() - 1));
				output.setLength(0);
				return 0;
			}


		};

		print.register("print");
		JavaFunction set = new JavaFunction(L) {
			@Override
			public int execute() throws LuaException {
				LuaThread thread = (LuaThread) L.toJavaObject(2);

				thread.set(L.toString(3), L.toJavaObject(4));
				return 0;
			}
		};
		set.register("set");

		JavaFunction call = new JavaFunction(L) {
			@Override
			public int execute() throws LuaException {
				LuaThread thread = (LuaThread) L.toJavaObject(2);

				int top=L.getTop();
				if (top > 3) {
					Object[] args = new Object[top - 3];
					for (int i=4;i <= top;i++) {
						args[i - 4] = L.toJavaObject(i);
					}
					thread.call(L.toString(3), args);
				}
				else if (top == 3) {
					thread.call(L.toString(3));
				}

				return 0;
			};
		};
		call.register("call");

	}

	//运行lua脚本
	public Object doFile(String filePath) {
		return doFile(filePath, new Object[0]);
	}

	public Object doFile(String filePath, Object[] args) {
		int ok = 0;
		try {
			if (filePath.charAt(0) != '/')
				filePath = luaDir + "/" + filePath;

			L.setTop(0);
			ok = L.LloadFile(filePath);

			if (ok == 0) {
				L.getGlobal("debug");
				L.getField(-1, "traceback");
				L.remove(-2);
				L.insert(-2);
				int l=0;
				if (args != null)
					l = args.length;
				for (int i=0;i < l;i++) {
					L.pushObjectValue(args[i]);
				}
				ok = L.pcall(l, 1, -2 - l);
				if (ok == 0) {
					return L.toJavaObject(-1);
				}
			}
			throw new LuaException(errorReason(ok) + ": " + L.toString(-1));
		}
		catch (LuaException e) {
			sendMsg(e.getMessage());
		}

		return null;
	}

	public Object doAsset(String name, Object...args) {
		int ok = 0;
		try {
			byte[] bytes = readAsset(name);
			L.setTop(0);
			ok = L.LloadBuffer(bytes, name);

			if (ok == 0) {
				L.getGlobal("debug");
				L.getField(-1, "traceback");
				L.remove(-2);
				L.insert(-2);
				int l=0;
				if (args != null)
					l = args.length;
				for (int i=0;i < l;i++) {
					L.pushObjectValue(args[i]);
				}
				ok = L.pcall(l, 0, -2 - l);
				if (ok == 0) {
					return L.toJavaObject(-1);
				}
			}
			throw new LuaException(errorReason(ok) + ": " + L.toString(-1));
		}
		catch (Exception e) {
			sendMsg(e.getMessage());
		}

		return null;
	}

//运行lua函数
	public Object runFunc(String funcName, Object...args) {
		if (L != null) {
			try {
				L.setTop(0);
				L.getGlobal(funcName);
				if (L.isFunction(-1)) {
					L.getGlobal("debug");
					L.getField(-1, "traceback");
					L.remove(-2);
					L.insert(-2);

					int l=0;
					if (args != null)
						l = args.length;
					for (int i=0;i < l;i++) {
						L.pushObjectValue(args[i]);
					}

					int ok = L.pcall(l, 1, -2 - l);
					if (ok == 0) {
						return L.toJavaObject(-1);
					}
					throw new LuaException(errorReason(ok) + ": " + L.toString(-1));
				}
			}
			catch (LuaException e) {
				sendMsg(funcName + " " + e.getMessage());
			}
		}
		return null;
	}

//读取asset文件

//运行lua代码
	public Object doString(String funcSrc, Object... args) {
		try {
			L.setTop(0);
			int ok = L.LloadString(funcSrc);

			if (ok == 0) {
				L.getGlobal("debug");
				L.getField(-1, "traceback");
				L.remove(-2);
				L.insert(-2);

				int l=0;
				if (args != null)
					l = args.length;
				for (int i=0;i < l;i++) {
					L.pushObjectValue(args[i]);
				}

				ok = L.pcall(l, 1, -2 - l);
				if (ok == 0) {
					return L.toJavaObject(-1);
				}
			}
			throw new LuaException(errorReason(ok) + ": " + L.toString(-1)) ;
		}
		catch (LuaException e) {
			sendMsg(e.getMessage());
		}
		return null;
	}

//生成错误信息
	private String errorReason(int error) {
		switch (error) {
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

	public byte[] readAsset(String name) throws IOException {
		AssetManager am = getAssets();
		InputStream is = am.open(name);
		byte[] ret= readAll(is);
		is.close();
		//am.close();
		return ret;
	}

	//显示信息
	public void sendMsg(String msg) {
		Message message = new Message();
		Bundle bundle = new Bundle();
		bundle.putString("data", msg);
		message.setData(bundle);
		message.what = 0;
		handler.sendMessage(message);
		Log.i("lua", msg);
	}

	@Override
	public void sendError(String title, Exception msg) {
		runFunc("onError", title, msg);
	}

	public Object loadLib(String name) throws LuaException {
		int i=name.indexOf(".");
		String fn = name;
		if (i > 0)
			fn = name.substring(0, i);
		File f=new File(libDir + "/lib" + fn + ".so");
		if (!f.exists()) {
			f = new File(luaDir + "/lib" + fn + ".so");
			if (!f.exists())
				throw new LuaException("can not find lib " + name);
			copyFile(luaDir + "/lib" + fn + ".so", libDir + "/lib" + fn + ".so");
		}
		LuaObject require=L.getLuaObject("require");
		return require.call(name);
	}

	private void copyFile(String oldPath, String newPath) {
		try {
			int bytesum = 0;
			int byteread = 0;
			File oldfile = new File(oldPath);
			if (oldfile.exists()) { //文件存在时
				InputStream inStream = new FileInputStream(oldPath); //读入原文件
				FileOutputStream fs = new FileOutputStream(newPath);
				byte[] buffer = new byte[4096];
				int length;
				while ((byteread = inStream.read(buffer)) != -1) {
					bytesum += byteread; //字节数 文件大小
					System.out.println(bytesum);
					fs.write(buffer, 0, byteread);
				}
				inStream.close();
			}
		}
		catch (Exception e) {
			System.out.println("复制文件操作出错");
			e.printStackTrace();

		}

	}

//显示toast
	@SuppressLint("ShowToast")
	public void showToast(String text) {
		try{
			long now=System.currentTimeMillis();
			if (toast == null || now - lastShow > 1000) {
				toastbuilder.setLength(0);
				toast = Toast.makeText(this, text, Toast.LENGTH_LONG);
				toastbuilder.append(text);
			}
			else {
				toastbuilder.append("\n");
				toastbuilder.append(text);
				toast.setText(toastbuilder.toString());
				toast.setDuration(Toast.LENGTH_LONG);
			}
			lastShow = now;
			toast.show();
		}catch (Exception e){
			e.printStackTrace();
		}

    }

	private void setField(String key, Object value) {
		try {
			L.pushObjectValue(value);
			L.setGlobal(key);
		}
		catch (LuaException e) {
			sendMsg(e.getMessage());
		}
	}

	public void call(String func) {
		push(2, func);

	}

	public void call(String func, Object[] args) {
		if (args.length == 0)
			push(2, func);
		else
			push(3, func, args);
	}

	public void set(String key, Object value) {
		push(1, key, new Object[]{ value});
	}

	public Object get(String key) throws LuaException {
		L.getGlobal(key);
		return L.toJavaObject(-1);
	}

	public void push(int what, String s) {
		Message message = new Message();
		Bundle bundle = new Bundle();
		bundle.putString("data", s);
		message.setData(bundle);
		message.what = what;

		handler.sendMessage(message);

	}

	public void push(int what, String s, Object[] args) {
		Message message = new Message();
		Bundle bundle = new Bundle();
		bundle.putString("data", s);
		bundle.putSerializable("args", args);
		message.setData(bundle);
		message.what = what;

		handler.sendMessage(message);

	}

	public class LuaBinder extends Binder {

		public LuaService getService() {
			return LuaService.this;
		}
	}

	public class MainHandler extends Handler {
		@Override 
		public void handleMessage(Message msg) { 
			super.handleMessage(msg); 
			switch (msg.what) {
				case 0:
					{

						String data = msg.getData().getString("data");
						showToast(data);
					}
					break;
				case 1:
					{
						Bundle data=msg.getData();
						setField(data.getString("data"), ((Object[])data.getSerializable("args"))[0]);
					}
					break;
				case 2:
					{
						String src=msg.getData().getString("data");
						runFunc(src);
					}
					break;
				case 3:
					{
						String src=msg.getData().getString("data");
						Serializable args=msg.getData().getSerializable("args");
						runFunc(src, (Object[])args);
					}
			}
		}
	}
}

package com.androlua;

import android.app.*;
import android.content.*;
import android.content.res.*;
import android.graphics.*;
import android.net.*;
import android.os.*;
import android.util.*;
import android.view.*;
import android.view.ViewGroup.*;
import android.widget.*;
import com.luajava.*;
import java.io.*;
import java.util.*;
import java.util.concurrent.*;
import java.util.regex.*;

public class Main extends Activity
{

	private LuaState L;
	private String luaPath;
	public String luaDir;

		private StringBuilder toastbuilder = new StringBuilder();
	private Boolean isCreate = false;

	public Handler handler;

	private Toast toast;
	public TextView status;
	private LinearLayout layout;

	private boolean isSetViewed;

	private long lastShow;

	private Menu optionsMenu;

	private LuaObject mOnKeyDown;

	private LuaObject mOnKeyUp;

	private LuaObject mOnKeyLongPress;

	private LuaObject mOnTouchEvent;

	private ArrayList<LuaThread> threadList=new ArrayList<LuaThread>();

	private ExecutorService mThreadPool;

	public String luaCpath;
	private boolean mInAsset=false;


	@Override
	public void onCreate(Bundle savedInstanceState)
	{//startForeground(1, new Notification()); 
		setTheme(android.R.style.Theme_Holo_Light_NoActionBar);
		//s=android.R.style.Theme_Holo_Wallpaper_NoTitleBar;
		//设置主题
//		Intent intent=getIntent();
//		int theme=intent.getIntExtra("theme", android.R.style.Theme_Holo_Light_NoActionBar);
//		setTheme(theme);
//
		//设置print界面
		super.onCreate(savedInstanceState);
		/*LayoutInflater inflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		 layout = (LinearLayout) inflater.inflate(R.layout.main, null);

		 status = (TextView) layout.findViewById(R.id.status);
		 */
		layout = new LinearLayout(this);
		layout.setBackgroundColor(Color.WHITE);
		ScrollView scroll=new ScrollView(this);
		scroll.setFillViewport(true);
		status = new TextView(this);
		status.setTextColor(Color.BLACK);
		scroll.addView(status, new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT));
		layout.addView(scroll, new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT));
//		status.setMovementMethod(ScrollingMovementMethod.getInstance());
		status.setText("");
		status.setTextIsSelectable(true);
		//status.setTypeface(Typeface.MONOSPACE);
//		status.setHorizontallyScrolling(true);
//		status.setEnabled(true)
		//初始化AndroLua工作目录
		String state = Environment.getExternalStorageState();
		if (state.equals(Environment.MEDIA_MOUNTED))
		{
			String sdDir = Environment.getExternalStorageDirectory().getAbsolutePath();
			luaDir = sdDir + "/AndroLua";
		}
		else
		{
//			luaDir = getFilesDir().getAbsolutePath();
			sendMsg("No such sdcard");
			setContentView(layout);
			return;
		}	
		mThreadPool = Executors.newCachedThreadPool();
		luaCpath=getApplicationInfo().nativeLibraryDir + "/lib?.so"+";"+getDir("lib",Context.MODE_PRIVATE).getAbsolutePath()+"/lib?.so";
		//luaCpath="./?.so;" + getApplicationInfo().nativeLibraryDir+ "/lib?.so";
		
		File destDir = new File(luaDir);
		if (!destDir.exists())
			destDir.mkdirs();

		handler = new MainHandler();

		try
		{
			status.setText("");
			Intent intent=getIntent();
			Uri uri=intent.getData(); 
			if (uri != null)
			{
				String path = uri.getPath();

				if (Pattern.matches("^/[\\w_]+$", path))
				{
					initLua();
					path = path.substring(1, path.length());
					//doString("require \"" + path + "\"");
					mInAsset=true;
					doAsset(path + ".lua");
				}
				else
				{
					luaPath = path;
					luaDir = luaPath.substring(0, luaPath.lastIndexOf("/"));
					initLua();
//					Jlua.init(L);				
					doFile(luaPath);
				}
			}
			else
			{
				initLua();
//				Jlua.init(L);
				mInAsset=true;
				doAsset("main");
				//doFile(luaDir+"/new.lua");
			}
			isCreate = true;
			runFunc("onCreate", savedInstanceState);
			if (!isSetViewed)
				setContentView(layout);
		} 
		catch (Exception e)
		{
			sendMsg(e.getMessage());
			setContentView(layout);
			return;
		}

		mOnKeyDown = L.getLuaObject("onKeyDown");
		if (mOnKeyDown.isNil())
			mOnKeyDown = null;
		mOnKeyUp = L.getLuaObject("onKeyUp");
		if (mOnKeyUp.isNil())
			mOnKeyUp = null;
		mOnKeyLongPress = L.getLuaObject("onKeyLongPress");
		if (mOnKeyLongPress.isNil())
			mOnKeyLongPress = null;
		mOnTouchEvent = L.getLuaObject("onTouchEvent");
		if (mOnTouchEvent.isNil())
			mOnTouchEvent = null;

	}

	public boolean isInAsset()
	{
		return mInAsset;
	}
	
	@Override
	public void onContentChanged()
	{
		// TODO: Implement this method
		super.onContentChanged();
		isSetViewed = true;
	}

	@Override
	protected void onStart()
	{
		super.onStart();
		runFunc("onStart");
	}

	@Override
	protected void onResume()
	{
		super.onResume();
		runFunc("onResume");
	}

	@Override
	protected void onPause()
	{
		super.onPause();
		runFunc("onPause");
	}

	@Override
	protected void onStop()
	{
		super.onStop();
		runFunc("onStop");
	}

	@Override
	protected void onDestroy()
	{
		for (LuaThread t:threadList)
		{
			sendMsg(t.toString() + t.isRun);
			if (t.isRun)
				t.quit();
		}
		runFunc("onDestroy");
		super.onDestroy();
		System.gc();
		L.gc(LuaState.LUA_GCCOLLECT, 1);
		//L.close();
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data)
	{
		// TODO: Implement this method
		runFunc("onActivityResult", requestCode, resultCode, data);
		super.onActivityResult(requestCode, resultCode, data);
	}


	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		if (mOnKeyDown != null)
		{
			try
			{
				Object ret=mOnKeyDown.call(keyCode, event);
				if (ret != null && ret.getClass() == Boolean.class && (boolean)ret)
					return true;
			}
			catch (LuaException e)
			{
				sendMsg("onKeyDown " + e.getMessage());
			}
		}
		return super.onKeyDown(keyCode, event);
	}

	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event)
	{
		if (mOnKeyUp != null)
		{
			try
			{
				Object ret=mOnKeyUp.call(keyCode, event);
				if (ret != null && ret.getClass() == Boolean.class && (boolean)ret)
					return true;
			}
			catch (LuaException e)
			{
				sendMsg("onKeyUp " + e.getMessage());
			}
		}
		return super.onKeyUp(keyCode, event);
	}

	@Override
	public boolean onKeyLongPress(int keyCode, KeyEvent event)
	{
		if (mOnKeyLongPress != null)
		{
			try
			{
				Object ret=mOnKeyLongPress.call(keyCode, event);
				if (ret != null && ret.getClass() == Boolean.class && (boolean)ret)
					return true;
			}
			catch (LuaException e)
			{
				sendMsg("onKeyLongPress " + e.getMessage());
			}
		}
		return super.onKeyLongPress(keyCode, event);
	}

	@Override
	public boolean onTouchEvent(MotionEvent event)
	{
		if (mOnTouchEvent != null)
		{
			try
			{
				Object ret=mOnTouchEvent.call(event);
				if (ret != null && ret.getClass() == Boolean.class && (boolean)ret)
					return true;
			}
			catch (LuaException e)
			{
				sendMsg("onTouchEvent " + e.getMessage());
			}
		}
		return super.onTouchEvent(event);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu)
	{
		// TODO: Implement this method
		optionsMenu = menu;
		runFunc("onCreateOptionsMenu", menu);
		return super.onCreateOptionsMenu(menu);
	}

	public Menu getOptionsMenu()
	{
		return optionsMenu;
	}

	@Override
	public boolean onMenuItemSelected(int featureId, MenuItem item)
	{
		// TODO: Implement this method
		runFunc("onMenuItemSelected", featureId, item);
		return super.onMenuItemSelected(featureId, item);
	}

	@Override
	public void onCreateContextMenu(ContextMenu menu, View v, ContextMenu.ContextMenuInfo menuInfo)
	{
		// TODO: Implement this method
		runFunc("onCreateContextMenu", menu, v, menuInfo);
		super.onCreateContextMenu(menu, v, menuInfo);
	}

	@Override
	public boolean onContextItemSelected(MenuItem item)
	{
		// TODO: Implement this method
		runFunc("onContextItemSelected", item);
		return super.onContextItemSelected(item);
	}



	public int getWidth()
	{
		return getWindowManager().getDefaultDisplay().getWidth();
	}
	public int getHeight()
	{
		return getWindowManager().getDefaultDisplay().getHeight();
	}

	public void newActivity(String path)
	{
		Intent intent = new Intent(this, Main.class);
		if (Pattern.matches("^\\w+$", path))
			if (isInAsset())
				intent.setData(Uri.parse("file:/" + path));
			else
				intent.setData(Uri.parse("file://" + luaDir + "/" + path + ".lua"));
		else
			intent.setData(Uri.parse("file://" + path));

		startActivityForResult(intent, 1);
		overridePendingTransition(android.R.anim.slide_in_left, android.R.anim.slide_out_right);
	}

	public void newActivity(int req, String path)
	{
		Intent intent = new Intent(this, Main.class);
		intent.setData(Uri.parse("file://" + path));
		startActivityForResult(intent, req);
		overridePendingTransition(android.R.anim.slide_in_left, android.R.anim.slide_out_right);
	}


	
//初始化lua使用的Java函数
	private void initLua() throws Exception
	{
		L = LuaStateFactory.newLuaState();
		L.openLibs();
		L.pushJavaObject(this);
		L.setGlobal("activity");

		L.getGlobal("luajava"); 
		L.pushString(luaDir);
		L.setField(-2, "luadir"); 
		L.pushString(luaPath);
		L.setField(-2, "luapath"); 
		L.pop(1);

		JavaFunction print = new LuaPrint(this,L);
		print.register("print");

		JavaFunction assetLoader = new LuaAssetLoader(this,L); 

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
		/*
		 JavaFunction task = new newLuaAsyncTask(L);
		 task.register("task");


		 JavaFunction thread = new newLuaThread(L);
		 thread.register("thread");
		 */
		JavaFunction set = new JavaFunction(L) {
			@Override
			public int execute() throws LuaException
			{
				LuaThread thread = (LuaThread) L.toJavaObject(2);

				thread.set(L.toString(3), L.toJavaObject(4));
				return 0;
			}
		};
		set.register("set");

		JavaFunction call = new JavaFunction(L) {
			@Override
			public int execute() throws LuaException
			{
				LuaThread thread = (LuaThread) L.toJavaObject(2);

				int top=L.getTop();
				if (top > 3)
				{
					Object[] args = new Object[top - 3];
					for (int i=4;i <= top;i++)
					{
						args[i - 4] = L.toJavaObject(i);
					}
					thread.call(L.toString(3), args);
				}
				else if (top == 3)
				{
					thread.call(L.toString(3));
				}

				return 0;
			};
		};
		call.register("call");

	}

//运行lua脚本
	private void doFile(String filePath) 
	{
		int ok = 0;
		try
		{
			L.setTop(0);
			ok = L.LloadFile(filePath);

			if (ok == 0)
			{
				L.getGlobal("debug");
				L.getField(-1, "traceback");
				L.remove(-2);
				L.insert(-2);
				ok = L.pcall(0, 0, -2);
				if (ok == 0)
				{				
//					setResult(ok);
					return;
				}
			}
			Intent res= new Intent();
			res.putExtra("data", L.toString(-1));
			setResult(ok, res);
			throw new LuaException(errorReason(ok) + ": " + L.toString(-1));
		} 
		catch (LuaException e)
		{			
//			Toast.makeText(Main.this, e.getMessage(), Toast.LENGTH_LONG).show();
			setTitle(errorReason(ok));
			setContentView(layout);
			sendMsg(e.getMessage());
		}

	}

	public void doAsset(String name) 
	{
		int ok = 0;
		try
		{
			byte[] bytes = readAsset(name);
			L.setTop(0);
			ok = L.LloadBuffer(bytes, name);

			if (ok == 0)
			{
				L.getGlobal("debug");
				L.getField(-1, "traceback");
				L.remove(-2);
				L.insert(-2);
				ok = L.pcall(0, 0, -2);
				if (ok == 0)
				{				
					return;
				}
			}
			throw new LuaException(errorReason(ok) + ": " + L.toString(-1));
		} 
		catch (Exception e)
		{			
			setTitle(errorReason(ok));
			setContentView(layout);
			sendMsg(e.getMessage());
		}

	}

//运行lua函数
	private Object runFunc(String funcName, Object...args)
	{
		if (isCreate)
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
						return L.toJavaObject(-1);
					}
					throw new LuaException(errorReason(ok) + ": " + L.toString(-1));
				}
			}
			catch (LuaException e)
			{
				sendMsg(funcName + " " + e.getMessage());
			}
		}	
		return false;
	}



//运行lua代码
	private void doString(String funcSrc, Object... args)
	{
		try
		{
			L.setTop(0);
			int ok = L.LloadString(funcSrc);

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

				ok = L.pcall(l, 1, -2 - l);
				if (ok == 0)
				{				
					return ;
				}
			}
			throw new LuaException(errorReason(ok) + ": " + L.toString(-1)) ;
		}
		catch (LuaException e)
		{
			sendMsg(e.getMessage());
		}
	}


//生成错误信息
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

//读取asset文件

	public byte[] readAsset(String name) throws IOException 
	{
		AssetManager am = getAssets();
		InputStream is = am.open(name);
		byte[] ret= readAll(is);
		is.close();
		//am.close();
		return ret;
	}

	private static byte[] readAll(InputStream input) throws IOException 
	{
		ByteArrayOutputStream output = new ByteArrayOutputStream(4096);
		byte[] buffer = new byte[4096];
		int n = 0;
		while (-1 != (n = input.read(buffer)))
		{
			output.write(buffer, 0, n);
		}
		byte[] ret= output.toByteArray();
		output.close();
		return ret;
	}

//复制asset文件到sd卡
	public void assetsToSD(String InFileName, String OutFileName) throws IOException 
	{  
		InputStream myInput;  
		OutputStream myOutput = new FileOutputStream(OutFileName);  
		myInput = this.getAssets().open(InFileName);  
		byte[] buffer = new byte[8192];  
		int length = myInput.read(buffer);
        while (length > 0)
        {
			myOutput.write(buffer, 0, length); 
			length = myInput.read(buffer);
		}

        myOutput.flush();  
		myInput.close();  
		myOutput.close();        
	}  

//显示信息
	public void sendMsg(String msg)
	{
		Message message = new Message();
		Bundle bundle = new Bundle();  
		bundle.putString("data", msg);  
		message.setData(bundle);  
		message.what = 0;
		handler.sendMessage(message);
		Log.d("lua", msg);
	}


//显示toast
	public void showToast(String text)
	{   
		long now=System.currentTimeMillis();
        if (toast == null || now - lastShow > 1000)
		{ 
			toastbuilder.setLength(0);
            toast = Toast.makeText(this, text, 1000);    
			toastbuilder.append(text);
		}
		else
		{    
			toastbuilder.append("\n");
			toastbuilder.append(text);
			toast.setText(toastbuilder.toString());      
            toast.setDuration(1000);           
		}    
		lastShow = now;
		toast.show();
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
			sendMsg(e.getMessage());
		}
	}

	public void call(String func)
	{
		push(2, func);

	}

	public void call(String func, Object[] args)
	{
		if (args.length == 0)
			push(2, func);
		else
			push(3, func, args);
	}

	public void set(String key, Object value)
	{
		push(1, key, new Object[]{ value});
	}

	public Object get(String key) throws LuaException
	{
		L.getGlobal(key);
		return L.toJavaObject(-1);
	}

	public void push(int what, String s)
	{
		Message message = new Message();
		Bundle bundle = new Bundle();
		bundle.putString("data", s);
		message.setData(bundle);  
		message.what = what;

		handler.sendMessage(message);

	}

	public void push(int what, String s, Object[] args)
	{
		Message message = new Message();
		Bundle bundle = new Bundle();
		bundle.putString("data", s);
		bundle.putSerializable("args", args);
		message.setData(bundle);  
		message.what = what;

		handler.sendMessage(message);

	}
	
	

	

	public class MainHandler extends Handler
	{
		@Override 
		public void handleMessage(Message msg)
		{ 
			super.handleMessage(msg); 
			switch (msg.what)
			{
				case 0:
					{

						String data = msg.getData().getString("data");
//							Toast.makeText(Main.this, data , Toast.LENGTH_SHORT).show();
						showToast(data);
//							msgbuilder.append(data);
//							msgbuilder.append("\n");
//							status.setText(msgbuilder.toString());
						status.append(data + "\n");
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

	

	

}}

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

import com.androlua.util.*;
import com.luajava.*;

import dalvik.system.*;

import java.io.*;
import java.util.*;
import java.util.zip.*;

public class LuaActivity extends Activity implements LuaBroadcastReceiver.OnReceiveListerer, LuaContext {

    private LuaDexLoader mLuaDexLoader;
    private static ArrayList<String> prjCache = new ArrayList<String>();

    private int mWidth;

    private int mHeight;

    private ListView list;

    private ArrayListAdapter<String> adapter;

    @Override
    public ArrayList<ClassLoader> getClassLoaders() {
        // TODO: Implement this method
        return mLuaDexLoader.getClassLoaders();
    }

    public HashMap<String, String> getLibrarys() {
        return mLuaDexLoader.getLibrarys();
    }

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

    public String luaCpath;

    private String localDir;

    private String odexDir;

    private String libDir;

    private String luaExtDir;

    private LuaBroadcastReceiver mReceiver;

    private String luaLpath;

    private String luaMdDir;

    private boolean isUpdata;

    private boolean mDebug = true;

    private final static String DATA = "data";

    private final static String NAME = "name";

    private AssetManager mAssetManager;

    private Resources mResources;

    private Resources.Theme mTheme;

    private ArrayList<LuaGcable> gclist = new ArrayList<LuaGcable>();

    @Override
    public void onCreate(Bundle savedInstanceState) {
        setTheme(android.R.style.Theme_Holo_Light_NoActionBar);
        //设置主题
//		Intent intent=getIntent();
//		int theme=intent.getIntExtra("theme", android.R.style.Theme_Holo_Light_NoActionBar);

        StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder().permitAll().build();
        StrictMode.setThreadPolicy(policy);
        //设置print界面
        super.onCreate(null);

        WindowManager wm = (WindowManager) getSystemService(Context.WINDOW_SERVICE);
        DisplayMetrics outMetrics = new DisplayMetrics();
        wm.getDefaultDisplay().getMetrics(outMetrics);
        mWidth = outMetrics.widthPixels;
        mHeight = outMetrics.heightPixels;

        layout = new LinearLayout(this);
        //layout.setBackgroundColor(Color.WHITE);
        ScrollView scroll = new ScrollView(this);
        scroll.setFillViewport(true);
        status = new TextView(this);

        status.setTextColor(Color.BLACK);
        scroll.addView(status, new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT));
        //layout.addView(scroll, new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT));
        status.setText("");
        status.setTextIsSelectable(true);
        list = new ListView(this);
        list.setFastScrollEnabled(true);
        adapter = new ArrayListAdapter<String>(this, android.R.layout.simple_list_item_1) {
            @Override
            public View getView(int position, View convertView, ViewGroup parent) {
                TextView view = (TextView) super.getView(position, convertView, parent);
                if (convertView == null)
                    view.setTextIsSelectable(true);
                return view;
            }
        };
        list.setAdapter(adapter);
        layout.addView(list, new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT));


        //定义文件夹
        LuaApplication app = (LuaApplication) getApplication();
        localDir = app.getLocalDir();
        odexDir = app.getOdexDir();
        libDir = app.getLibDir();
        luaMdDir = app.getMdDir();
        luaCpath = app.getLuaCpath();
        luaDir = localDir;
        luaLpath = app.getLuaLpath();
        luaExtDir = app.getLuaExtDir();


        handler = new MainHandler();

        try {
            status.setText("");
            adapter.clear();
            Intent intent = getIntent();
            Object[] arg = (Object[]) intent.getSerializableExtra("arg");
            if (arg == null)
                arg = new Object[0];

            luaPath = getLuaPath();

            luaLpath = (luaDir + "/?.lua;" + luaDir + "/lua/?.lua;" + luaDir + "/?/init.lua;") + luaLpath;
            initLua();
            mLuaDexLoader = new LuaDexLoader(this);
            mLuaDexLoader.loadLibs();
            //MultiDex.installLibs(this);

            doFile(luaPath, arg);
            isCreate = true;
            runFunc("main", arg);
            runFunc("onCreate", savedInstanceState);
            if (!isSetViewed) {
                TypedArray array = getTheme().obtainStyledAttributes(new int[]{
                        android.R.attr.colorBackground,
                        android.R.attr.textColorPrimary,
                        android.R.attr.textColorHighlightInverse,
                });
                int backgroundColor = array.getColor(0, 0xFF00FF);
                int textColor = array.getColor(1, 0xFF00FF);
                array.recycle();
                status.setTextColor(textColor);
                layout.setBackgroundColor(backgroundColor);
                setContentView(layout);
            }
        } catch (Exception e) {
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


    @Override
    public void regGc(LuaGcable obj) {
        // TODO: Implement this method
        gclist.add(obj);
    }

    public String getLuaPath() {
        // TODO: Implement this method
        Intent intent = getIntent();
        Uri uri = intent.getData();
        String path = null;
        if (uri == null)
            return null;

        path = uri.getPath();
        luaPath = path;
        File f = new File(path);

        luaDir = new File(luaPath).getParent();
        if (f.getName().equals("main.lua") && !new File(luaDir + "/init.lua").exists()) {
            if (!prjCache.contains(luaDir))
                prjCache.add(luaDir);
        } else {
            for (String p : prjCache)
                if (luaDir.indexOf(p) == 0) {
                    luaDir = p;
                    break;
                }
        }
        return path;
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
        return L;
    }

    public View getDecorView() {
        return getWindow().getDecorView();
    }


    public String getLocalDir() {
        return localDir;
    }

    @Override
    public String getLuaExtDir() {
        return luaExtDir;
    }

    @Override
    public String getLuaExtDir(String name) {
        File dir = new File(luaExtDir + "/" + name);
        if (!dir.exists())
            if (!dir.mkdirs())
                return null;
        return dir.getAbsolutePath();
    }

    @Override
    public String getLuaDir() {
        return luaDir;
    }

    @Override
    public String getLuaDir(String name) {
        File dir = new File(luaDir + "/" + name);
        if (!dir.exists())
            if (!dir.mkdirs())
                return null;
        return dir.getAbsolutePath();
    }


    /**
     * 解压Assets中的文件
     *
     * @param assetName       压缩包文件名
     * @param outputDirectory 输出目录
     * @throws IOException
     */
    public void unZipAssets(String assetName, String outputDirectory) throws IOException {
        //创建解压目标目录
        File file = new File(outputDirectory);
        //如果目标目录不存在，则创建
        if (!file.exists()) {
            file.mkdirs();
        }
        InputStream inputStream = null;
        //打开压缩文件
        try {
            inputStream = this.getAssets().open(assetName);
        } catch (IOException e) {
            return;
        }


        ZipInputStream zipInputStream = new ZipInputStream(inputStream);
        //读取一个进入点
        ZipEntry zipEntry = zipInputStream.getNextEntry();
        //使用1Mbuffer
        byte[] buffer = new byte[1024 * 32];
        //解压时字节计数
        int count = 0;
        //如果进入点为空说明已经遍历完所有压缩包中文件和目录
        while (zipEntry != null) {
            //如果是一个目录
            if (zipEntry.isDirectory()) {
                //String name = zipEntry.getName();
                //name = name.substring(0, name.length() - 1);
                file = new File(outputDirectory + File.separator + zipEntry.getName());
                file.mkdir();
            } else {
                //如果是文件
                file = new File(outputDirectory + File.separator
                        + zipEntry.getName());
                //创建该文件
                file.createNewFile();
                FileOutputStream fileOutputStream = new FileOutputStream(file);
                while ((count = zipInputStream.read(buffer)) > 0) {
                    fileOutputStream.write(buffer, 0, count);
                }
                fileOutputStream.close();
            }
            //定位到下一个文件入口
            zipEntry = zipInputStream.getNextEntry();
        }
        zipInputStream.close();
    }

    public DexClassLoader loadDex(String path) throws LuaException {
        return mLuaDexLoader.loadDex(path);
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

    @Override
    public Resources getResources() {
        if (mLuaDexLoader != null && mLuaDexLoader.getResources() != null)
            return mLuaDexLoader.getResources();
        return super.getResources();
    }

    public Object loadLib(String name) throws LuaException {
        int i = name.indexOf(".");
        String fn = name;
        if (i > 0)
            fn = name.substring(0, i);
        File f = new File(libDir + "/lib" + fn + ".so");
        if (!f.exists()) {
            f = new File(luaDir + "/lib" + fn + ".so");
            if (!f.exists())
                throw new LuaException("can not find lib " + name);
            LuaUtil.copyFile(luaDir + "/lib" + fn + ".so", libDir + "/lib" + fn + ".so");
        }
        LuaObject require = L.getLuaObject("require");
        return require.call(name);
    }


    public Intent registerReceiver(LuaBroadcastReceiver receiver, IntentFilter filter) {
        // TODO: Implement this method
        return super.registerReceiver(receiver, filter);
    }

    public Intent registerReceiver(LuaBroadcastReceiver.OnReceiveListerer ltr, IntentFilter filter) {
        // TODO: Implement this method
        LuaBroadcastReceiver receiver = new LuaBroadcastReceiver(ltr);
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
    public void onContentChanged() {
        // TODO: Implement this method
        super.onContentChanged();
        isSetViewed = true;
    }

    @Override
    protected void onStart() {
        super.onStart();
        runFunc("onStart");
    }

    @Override
    protected void onResume() {
        super.onResume();
        runFunc("onResume");
    }

    @Override
    protected void onPause() {
        super.onPause();
        runFunc("onPause");
    }

    @Override
    protected void onStop() {
        super.onStop();
        runFunc("onStop");
    }


    @Override
    protected void onDestroy() {
        if (mReceiver != null)
            unregisterReceiver(mReceiver);

        for (LuaGcable obj : gclist) {
            obj.gc();
        }

        runFunc("onDestroy");
        super.onDestroy();
        System.gc();
        L.gc(LuaState.LUA_GCCOLLECT, 1);
        //L.close();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        // TODO: Implement this method
        if (data != null) {
            String name = data.getStringExtra(NAME);
            if (name != null) {
                Object[] res = (Object[]) data.getSerializableExtra(DATA);
                if (res == null) {
                    runFunc("onResult", name);
                } else {
                    Object[] arg = new Object[res.length + 1];
                    arg[0] = name;
                    for (int i = 0; i < res.length; i++)
                        arg[i + 1] = res[i];
                    Object ret = runFunc("onResult", arg);
                    if (ret != null && ret.getClass() == Boolean.class && (Boolean) ret)
                        return;
                }
            }
        }
        runFunc("onActivityResult", requestCode, resultCode, data);
        super.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (mOnKeyDown != null) {
            try {
                Object ret = mOnKeyDown.call(keyCode, event);
                if (ret != null && ret.getClass() == Boolean.class && (Boolean) ret)
                    return true;
            } catch (LuaException e) {
                sendError("onKeyDown", e);
            }
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (mOnKeyUp != null) {
            try {
                Object ret = mOnKeyUp.call(keyCode, event);
                if (ret != null && ret.getClass() == Boolean.class && (Boolean) ret)
                    return true;
            } catch (LuaException e) {
                sendError("onKeyUp", e);
            }
        }
        return super.onKeyUp(keyCode, event);
    }

    @Override
    public boolean onKeyLongPress(int keyCode, KeyEvent event) {
        if (mOnKeyLongPress != null) {
            try {
                Object ret = mOnKeyLongPress.call(keyCode, event);
                if (ret != null && ret.getClass() == Boolean.class && (Boolean) ret)
                    return true;
            } catch (LuaException e) {
                sendError("onKeyLongPress", e);
            }
        }
        return super.onKeyLongPress(keyCode, event);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (mOnTouchEvent != null) {
            try {
                Object ret = mOnTouchEvent.call(event);
                if (ret != null && ret.getClass() == Boolean.class && (Boolean) ret)
                    return true;
            } catch (LuaException e) {
                sendError("onTouchEvent", e);
            }
        }
        return super.onTouchEvent(event);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // TODO: Implement this method
        optionsMenu = menu;
        runFunc("onCreateOptionsMenu", menu);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // TODO: Implement this method
        Object ret = null;
        if (!item.hasSubMenu())
            ret = runFunc("onOptionsItemSelected", item);
        if (ret != null && ret.getClass() == Boolean.class && (Boolean) ret)
            return true;
        return super.onOptionsItemSelected(item);
    }

    public Menu getOptionsMenu() {
        return optionsMenu;
    }

    @Override
    public boolean onMenuItemSelected(int featureId, MenuItem item) {
        // TODO: Implement this method
        if (!item.hasSubMenu())
            runFunc("onMenuItemSelected", featureId, item);
        return super.onMenuItemSelected(featureId, item);
    }


    @Override
    public void onCreateContextMenu(ContextMenu menu, View v, ContextMenu.ContextMenuInfo menuInfo) {
        // TODO: Implement this method
        runFunc("onCreateContextMenu", menu, v, menuInfo);
        super.onCreateContextMenu(menu, v, menuInfo);
    }

    @Override
    public boolean onContextItemSelected(MenuItem item) {
        // TODO: Implement this method
        runFunc("onContextItemSelected", item);
        return super.onContextItemSelected(item);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        // TODO: Implement this method
        super.onConfigurationChanged(newConfig);
        WindowManager wm = (WindowManager) getSystemService(Context.WINDOW_SERVICE);
        DisplayMetrics outMetrics = new DisplayMetrics();
        wm.getDefaultDisplay().getMetrics(outMetrics);
        mWidth = outMetrics.widthPixels;
        mHeight = outMetrics.heightPixels;

        runFunc("onConfigurationChanged", newConfig);
    }


    public int getWidth() {
        return mWidth;
    }

    public int getHeight() {
        return mHeight;
    }

    @Override
    public Map getGlobalData() {
        return ((LuaApplication) getApplication()).getGlobalData();
    }


    public boolean bindService(int flag) {
        ServiceConnection conn = new ServiceConnection() {

            @Override
            public void onServiceConnected(ComponentName comp, IBinder binder) {
                // TODO: Implement this method
                runFunc("onServiceConnected", comp, ((LuaService.LuaBinder) binder).getService());
            }

            @Override
            public void onServiceDisconnected(ComponentName comp) {
                // TODO: Implement this method
                runFunc("onServiceDisconnected", comp);
            }
        };
        return bindService(conn, flag);
    }

    public boolean bindService(ServiceConnection conn, int flag) {
        // TODO: Implement this method
        Intent service = new Intent(this, LuaService.class);
        service.putExtra("luaDir", luaDir);
        service.putExtra("luaPath", luaPath);
        return super.bindService(service, conn, flag);
    }

    public ComponentName startService() {
        return startService(null, null);
    }

    public ComponentName startService(Object[] arg) {
        return startService(null, arg);
    }

    public ComponentName startService(String path) {
        return startService(path, null);
    }

    public ComponentName startService(String path, Object[] arg) {
        // TODO: Implement this method
        Intent intent = new Intent(this, LuaService.class);
        intent.putExtra("luaDir", luaDir);
        intent.putExtra("luaPath", luaPath);
        if (path != null) {
            if (path.charAt(0) != '/')
                intent.setData(Uri.parse("file://" + luaDir + "/" + path + ".lua"));
            else
                intent.setData(Uri.parse("file://" + path));
        }
        if (arg != null)
            intent.putExtra("arg", arg);

        return super.startService(intent);
    }


    public void newActivity(String path) throws FileNotFoundException {
        newActivity(1, path, null);
    }

    public void newActivity(String path, Object[] arg) throws FileNotFoundException {
        newActivity(1, path, arg);
    }

    public void newActivity(int req, String path) throws FileNotFoundException {
        newActivity(req, path, null);
    }

    public void newActivity(int req, String path, Object[] arg) throws FileNotFoundException {
        Intent intent = new Intent(this, LuaActivity.class);
        intent.putExtra(NAME, path);
        if (path.charAt(0) != '/')
            path = luaDir + "/" + path;
        File f = new File(path);
        if (f.isDirectory() && new File(path + "/main.lua").exists())
            path += "/main.lua";
        else if ((f.isDirectory() || !f.exists()) && !path.endsWith(".lua"))
            path += ".lua";
        if (!new File(path).exists())
            throw new FileNotFoundException(path);

        intent.setData(Uri.parse("file://" + path));

        if (arg != null)
            intent.putExtra("arg", arg);
        startActivityForResult(intent, req);
        //overridePendingTransition(android.R.anim.slide_in_left, android.R.anim.slide_out_right);
    }


    public void newActivity(String path, int in, int out) throws FileNotFoundException {
        newActivity(1, path, in, out, null);
    }

    public void newActivity(String path, int in, int out, Object[] arg) throws FileNotFoundException {
        newActivity(1, path, in, out, arg);
    }

    public void newActivity(int req, String path, int in, int out) throws FileNotFoundException {
        newActivity(req, path, in, out, null);
    }


    public void newActivity(int req, String path, int in, int out, Object[] arg) throws FileNotFoundException {
        Intent intent = new Intent(this, LuaActivity.class);
        intent.putExtra(NAME, path);
        if (path.charAt(0) != '/')
            path = luaDir + "/" + path;
        File f = new File(path);
        if (f.isDirectory() && new File(path + "/main.lua").exists())
            path += "/main.lua";
        else if ((f.isDirectory() || !f.exists()) && !path.endsWith(".lua"))
            path += ".lua";
        if (!new File(path).exists())
            throw new FileNotFoundException(path);
        intent.setData(Uri.parse("file://" + path));

        if (arg != null)
            intent.putExtra("arg", arg);
        startActivityForResult(intent, req);
        overridePendingTransition(in, out);
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


    public Bitmap loadBitmap(String path) throws IOException {
        return LuaBitmap.getBitmap(this, path);
    }

    public void setContentView(String layout) throws LuaException {
        setContentView(layout, null);
    }

    public void setContentView(String layout, LuaObject env) throws LuaException {
        // TODO: Implement this method
        LuaObject loadlayout = L.getLuaObject("loadlayout");
        View view = (View) loadlayout.call(layout, env);
        super.setContentView(view);
    }


    public void setContentView(LuaObject layout) throws LuaException {
        setContentView(layout, null);
    }

    public void setContentView(LuaObject layout, LuaObject env) throws LuaException {
        // TODO: Implement this method
        LuaObject loadlayout = L.getLuaObject("loadlayout");
        View view = null;
        if (layout.isString())
            view = (View) loadlayout.call(layout.getString(), env);
        else if (layout.isTable())
            view = (View) loadlayout.call(layout, env);
        else
            throw new LuaException("layout may be table or string.");
        super.setContentView(view);
    }

    public void result(Object[] data) {
        Intent res = new Intent();
        res.putExtra(NAME, getIntent().getStringExtra(NAME));
        res.putExtra(DATA, data);
        setResult(0, res);
        finish();
    }

    //初始化lua使用的Java函数
    private void initLua() throws Exception {
        L = LuaStateFactory.newLuaState();
        L.openLibs();
        L.pushJavaObject(this);
        L.setGlobal("activity");
        L.getGlobal("activity");
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
        initENV();

        JavaFunction print = new LuaPrint(this, L);
        print.register("print");

        L.getGlobal("package");
        L.pushString(luaLpath);
        L.setField(-2, "path");
        L.pushString(luaCpath);
        L.setField(-2, "cpath");
        L.pop(1);

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

                int top = L.getTop();
                if (top > 3) {
                    Object[] args = new Object[top - 3];
                    for (int i = 4; i <= top; i++) {
                        args[i - 4] = L.toJavaObject(i);
                    }
                    thread.call(L.toString(3), args);
                } else if (top == 3) {
                    thread.call(L.toString(3));
                }

                return 0;
            }

            ;
        };
        call.register("call");

    }

    public void setDebug(boolean isDebug) {
        mDebug = isDebug;
    }


    private void initENV() throws LuaException {
        if (!new File(luaDir + "/init.lua").exists())
            return;

        try {
            int ok = L.LloadFile(luaDir + "/init.lua");
            if (ok == 0) {
                L.newTable();
                LuaObject env = L.getLuaObject(-1);
                L.setUpValue(-2, 1);
                ok = L.pcall(0, 0, 0);
                if (ok == 0) {
                    LuaObject title = env.getField("appname");
                    if (title.isString())
                        setTitle(title.getString());

                    LuaObject debug = env.getField("debugmode");
                    if (debug.isBoolean())
                        mDebug = debug.getBoolean();

                    LuaObject theme = env.getField("theme");

                    if (theme.isNumber())
                        setTheme((int) theme.getInteger());
                    else if (theme.isString())
                        setTheme(android.R.style.class.getField(theme.getString()).getInt(null));

                    return;
                }
            }
            throw new LuaException(errorReason(ok) + ": " + L.toString(-1));
        } catch (Exception e) {
            sendMsg(e.getMessage());
        }
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
                int l = args.length;
                for (int i = 0; i < l; i++) {
                    L.pushObjectValue(args[i]);
                }
                ok = L.pcall(l, 1, -2 - l);
                if (ok == 0) {
                    return L.toJavaObject(-1);
                }
            }
            Intent res = new Intent();
            res.putExtra(DATA, L.toString(-1));
            setResult(ok, res);
            throw new LuaException(errorReason(ok) + ": " + L.toString(-1));
        } catch (LuaException e) {
            setTitle(errorReason(ok));
            setContentView(layout);
            sendMsg(e.getMessage());
            String s = e.getMessage();
            String p = "android.permission.";
            int i = s.indexOf(p);
            if (i > 0) {
                i = i + p.length();
                int n = s.indexOf(".", i);
                if (n > i) {
                    String m = s.substring(i, n);
                    L.getGlobal("require");
                    L.pushString("permission");
                    L.pcall(1, 0, 0);
                    L.getGlobal("permission_info");
                    L.getField(-1, m);
                    if (L.isString(-1))
                        m = m + " (" + L.toString(-1) + ")";
                    sendMsg("权限错误: " + m);
                    return null;
                }
            }
            if (isUpdata) {
                /*LuaUtil.rmDir(new File(extDir));
				 LuaUtil.rmDir(new File(luaMdDir));
				 SharedPreferences info=getSharedPreferences("appInfo", 0);
				 SharedPreferences.Editor edit=info.edit();
				 edit.putLong("lastUpdateTime", 0);
				 edit.commit();*/
                //sendMsg("初始化错误，请清除数据后重新启动程序。。。");
            }

        }

        return null;
    }

    public Object doAsset(String name, Object... args) {
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
                int l = args.length;
                for (int i = 0; i < l; i++) {
                    L.pushObjectValue(args[i]);
                }
                ok = L.pcall(l, 0, -2 - l);
                if (ok == 0) {
                    return L.toJavaObject(-1);
                }
            }
            throw new LuaException(errorReason(ok) + ": " + L.toString(-1));
        } catch (Exception e) {
            setTitle(errorReason(ok));
            setContentView(layout);
            sendMsg(e.getMessage());
        }

        return null;
    }

    //运行lua函数
    public Object runFunc(String funcName, Object... args) {
        if (L != null) {
            synchronized (L) {
                try {
                    L.setTop(0);
                    L.pushGlobalTable();
                    L.pushString(funcName);
                    L.rawGet(-2);
                    if (L.isFunction(-1)) {
                        L.getGlobal("debug");
                        L.getField(-1, "traceback");
                        L.remove(-2);
                        L.insert(-2);

                        int l = args.length;
                        for (int i = 0; i < l; i++) {
                            L.pushObjectValue(args[i]);
                        }

                        int ok = L.pcall(l, 1, -2 - l);
                        if (ok == 0) {
                            return L.toJavaObject(-1);
                        }
                        throw new LuaException(errorReason(ok) + ": " + L.toString(-1));
                    }
                } catch (LuaException e) {
                    sendError(funcName, e);
                }
            }
        }
        return null;
    }


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

                int l = args.length;
                for (int i = 0; i < l; i++) {
                    L.pushObjectValue(args[i]);
                }

                ok = L.pcall(l, 1, -2 - l);
                if (ok == 0) {
                    return L.toJavaObject(-1);
                }
            }
            throw new LuaException(errorReason(ok) + ": " + L.toString(-1));
        } catch (LuaException e) {
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

//读取asset文件

    public byte[] readAsset(String name) throws IOException {
        AssetManager am = getAssets();
        InputStream is = am.open(name);
        byte[] ret = readAll(is);
        is.close();
        //am.close();
        return ret;
    }

    private static byte[] readAll(InputStream input) throws IOException {
        ByteArrayOutputStream output = new ByteArrayOutputStream(4096);
        byte[] buffer = new byte[4096];
        int n = 0;
        while (-1 != (n = input.read(buffer))) {
            output.write(buffer, 0, n);
        }
        byte[] ret = output.toByteArray();
        output.close();
        return ret;
    }

    //复制asset文件到sd卡
    public void assetsToSD(String InFileName, String OutFileName) throws IOException {
        InputStream myInput;
        OutputStream myOutput = new FileOutputStream(OutFileName);
        myInput = this.getAssets().open(InFileName);
        byte[] buffer = new byte[8192];
        int length = myInput.read(buffer);
        while (length > 0) {
            myOutput.write(buffer, 0, length);
            length = myInput.read(buffer);
        }

        myOutput.flush();
        myInput.close();
        myOutput.close();
    }

    //显示信息
    public void sendMsg(String msg) {
        Message message = new Message();
        Bundle bundle = new Bundle();
        bundle.putString(DATA, msg);
        message.setData(bundle);
        message.what = 0;
        handler.sendMessage(message);
        Log.d("lua", msg);
    }

    @Override
    public void sendError(String title, Exception msg) {
        Object ret = runFunc("onError", title, msg);
        if (ret != null && ret.getClass() == Boolean.class && (Boolean) ret)
            return;
        else
            sendMsg(title + ": " + msg.getMessage());
    }

    //显示toast
    public void showToast(String text) {
        long now = System.currentTimeMillis();
        if (toast == null || now - lastShow > 1000) {
            toastbuilder.setLength(0);
            toast = Toast.makeText(this, text, Toast.LENGTH_LONG);
            toastbuilder.append(text);
        } else {
            toastbuilder.append("\n");
            toastbuilder.append(text);
            toast.setText(toastbuilder.toString());
            toast.setDuration(Toast.LENGTH_LONG);
        }
        lastShow = now;
        toast.show();
    }

    private void setField(String key, Object value) {
        synchronized (L) {
            try {
                L.pushObjectValue(value);
                L.setGlobal(key);
            } catch (LuaException e) {
                sendError("setField", e);
            }
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
        push(1, key, new Object[]{value});
    }

    public Object get(String key) throws LuaException {
        synchronized (L) {
            L.getGlobal(key);
            return L.toJavaObject(-1);
        }
    }

    public void push(int what, String s) {
        Message message = new Message();
        Bundle bundle = new Bundle();
        bundle.putString(DATA, s);
        message.setData(bundle);
        message.what = what;

        handler.sendMessage(message);

    }

    public void push(int what, String s, Object[] args) {
        Message message = new Message();
        Bundle bundle = new Bundle();
        bundle.putString(DATA, s);
        bundle.putSerializable("args", args);
        message.setData(bundle);
        message.what = what;

        handler.sendMessage(message);

    }


    public class MainHandler extends Handler {

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case 0: {

                    String data = msg.getData().getString(DATA);
                    if (mDebug)
                        showToast(data);
                    status.append(data + "\n");
                    adapter.add(data);
                }
                break;
                case 1: {
                    Bundle data = msg.getData();
                    setField(data.getString(DATA), ((Object[]) data.getSerializable("args"))[0]);
                }
                break;
                case 2: {
                    String src = msg.getData().getString(DATA);
                    runFunc(src);
                }
                break;
                case 3: {
                    String src = msg.getData().getString(DATA);
                    Serializable args = msg.getData().getSerializable("args");
                    runFunc(src, (Object[]) args);
                }
            }
        }
    }
}

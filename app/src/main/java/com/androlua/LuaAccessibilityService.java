package com.androlua;

import android.accessibilityservice.AccessibilityService;
import android.annotation.SuppressLint;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.Path;
import android.hardware.display.VirtualDisplay;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.KeyEvent;
import android.view.WindowManager;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.androlua.util.ClickRunnable;
import com.androlua.util.GlobalActionAutomator;
import com.nirenr.screencapture.ScreenCaptureListener;
import com.nirenr.screencapture.ScreenShot;
import com.luajava.LuaException;
import com.luajava.LuaFunction;
import com.luajava.LuaTable;
import com.nirenr.Point;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import static android.content.Intent.FLAG_ACTIVITY_NEW_TASK;
import static android.view.accessibility.AccessibilityNodeInfo.ACTION_ACCESSIBILITY_FOCUS;
import static android.view.accessibility.AccessibilityNodeInfo.ACTION_CLICK;
import static android.view.accessibility.AccessibilityNodeInfo.ACTION_FOCUS;
import static android.view.accessibility.AccessibilityNodeInfo.ACTION_LONG_CLICK;
import static android.view.accessibility.AccessibilityNodeInfo.ACTION_SCROLL_BACKWARD;

@SuppressLint("NewApi")
public class LuaAccessibilityService extends AccessibilityService {

    private static AccessibilityServiceCallbacks sAccessibilityServiceCallbacks;
    private LuaApplication mApplication;
    public static LuaFunction onAccessibilityEvent;
    private Map mData;
    private static LuaAccessibilityService mLuaAccessibilityService;
    private HashMap<String, ComponentName> appMap = new HashMap<String, ComponentName>();
    private boolean mOk;
    private Handler handler;
    private GlobalActionAutomator mGlobalActionAutomator;
    private ScreenShot mScreenShot;
    private int mScreenDensity;
    private int mScreenWidth;
    private int mScreenHeight;

    private void init() {
        WindowManager mWindowManager = (WindowManager) getSystemService(Context.WINDOW_SERVICE);
        DisplayMetrics metrics = new DisplayMetrics();
        if (mWindowManager == null) {
            return;
        }
        mWindowManager.getDefaultDisplay().getRealMetrics(metrics);
        mScreenDensity = metrics.densityDpi;
        mScreenWidth = metrics.widthPixels;
        mScreenHeight = metrics.heightPixels;
    }

    public int getDensity() {
        if(mScreenDensity==0)
            init();
        return mScreenDensity;
    }

    public int getWidth() {
        if(mScreenWidth==0)
            init();
        return mScreenWidth;
    }

    public int getHeight() {
        if(mScreenHeight==0)
            init();
        return mScreenHeight;
    }


    public static interface AccessibilityServiceCallbacks {
        public void onAccessibilityEvent(LuaAccessibilityService service,AccessibilityEvent event);

        public void onInterrupt(LuaAccessibilityService service);

        public void onServiceConnected(LuaAccessibilityService service);

        public void onCreate(LuaAccessibilityService service);

        public boolean onKeyEvent(LuaAccessibilityService service,KeyEvent event);

        public void onDestroy(LuaAccessibilityService service);

        public void onConfigurationChanged(LuaAccessibilityService service,Configuration newConfig) ;
    }

    public static void setCallback(AccessibilityServiceCallbacks callback){
        sAccessibilityServiceCallbacks=callback;
    }

    @Override
    public void onCreate() {
        // TODO: Implement this method
        Log.i("lua", "onCreate");
        super.onCreate();
        handler = new Handler();
        mLuaAccessibilityService = this;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N){
            mGlobalActionAutomator = new GlobalActionAutomator(this, new Handler());
            mGlobalActionAutomator.setService(this);
        }
        if(sAccessibilityServiceCallbacks!=null)
            sAccessibilityServiceCallbacks.onCreate(this);
        asyncGetAllApp();

        mApplication = (LuaApplication) getApplication();
        mData = mApplication.getGlobalData();
        if (!mData.containsKey("LuaAccessibilityService")) {
            return;
        }
        LuaTable as = (LuaTable) mData.get("LuaAccessibilityService");
        if (as == null)
            return;
        try {
            LuaFunction func = (LuaFunction) as.get("onCreate");
            func.call(this);
        } catch (LuaException e) {
            LuaFunction func = (LuaFunction) as.get("onError");
            if (func == null) {
                Log.i("onCreate", e.getMessage());
                return;
            }

            try {
                func.call(e);
            } catch (LuaException e2) {
                Log.i("onCreate", e.getMessage());
            }
        }
     }

    public boolean click(Point p) {
        return click(p.x, p.y);
    }

    public boolean click(int x, int y) {
        if (mGlobalActionAutomator != null) {
            return mGlobalActionAutomator.click(x, y);
        }
        return false;
    }

    public boolean longClick(Point p) {
        return longClick(p.x, p.y);
    }

    public boolean longClick(int x, int y) {
        if (mGlobalActionAutomator != null) {
            return mGlobalActionAutomator.longClick(x, y);
        }
        return false;
    }

    public boolean press(Point p, int delay) {
        return press(p.x, p.y, delay);
    }

    public boolean press(int x, int y, int delay) {
        if (mGlobalActionAutomator != null) {
            return mGlobalActionAutomator.press(x, y, delay);
        }
        return false;
    }

    public boolean swipe(Point p1, Point p2, int delay) {
        return swipe(p1.x, p1.y, p2.x, p2.y, delay);
    }

    public boolean swipe(int x1, int y1, int x2, int y2, int delay) {
        if (mGlobalActionAutomator != null) {
            return mGlobalActionAutomator.swipe(x1, y1, x2, y2, delay);
        }
        return false;
    }

    public boolean swipe(Path path, int delay) {
        if (mGlobalActionAutomator != null) {
            return mGlobalActionAutomator.gesture(0, delay,path);
        }
        return false;
    }

    @Override
    protected void onServiceConnected() {
        // TODO: Implement this method
        Log.i("lua", "onServiceConnected");
        super.onServiceConnected();
        if(sAccessibilityServiceCallbacks!=null)
            sAccessibilityServiceCallbacks.onServiceConnected(this);

        if (!mData.containsKey("LuaAccessibilityService")) {
            return;
        }
        LuaTable as = (LuaTable) mData.get("LuaAccessibilityService");
        if (as == null) {

            return;
        }
        try {
            LuaFunction func = (LuaFunction) as.get("onServiceConnected");
            func.call(this);
        } catch (LuaException e) {
            LuaFunction func = (LuaFunction) as.get("onError");
            if (func == null) {
                Log.i("onServiceConnected", e.getMessage());
                return;
            }

            try {
                func.call(e);
            } catch (LuaException e2) {
                Log.i("onServiceConnected", e.getMessage());
            }
        }

    }

    public static LuaAccessibilityService getInstance() {
        return mLuaAccessibilityService;
    }


    @Override
    public void onAccessibilityEvent(AccessibilityEvent p1) {
        // TODO: Implement this method
        //Log.i("lua", p1.toString());

        if(sAccessibilityServiceCallbacks!=null)
            sAccessibilityServiceCallbacks.onAccessibilityEvent(this,p1);

        if (onAccessibilityEvent != null) {
            try {
                onAccessibilityEvent.call(p1);
            } catch (LuaException e) {
                Log.i("lua", "onAccessibilityEvent: " + e.toString());
            }
            return;
        }
        if (!mData.containsKey("LuaAccessibilityService")) {
            return;
        }
        LuaTable as = (LuaTable) mData.get("LuaAccessibilityService");
        if (as == null) {
            return;
        }
        try {
            LuaFunction func = (LuaFunction) as.get("onAccessibilityEvent");
            func.call(p1);
        } catch (LuaException e) {
            LuaFunction func = (LuaFunction) as.get("onError");
            if (func == null) {
                Log.i("onAccessibilityEvent", e.getMessage());
                return;
            }
            try {
                func.call(e);
            } catch (LuaException e2) {
                Log.i("onAccessibilityEvent", e.getMessage());
            }
        }
    }

    @Override
    public void onInterrupt() {
        // TODO: Implement this method
        if(sAccessibilityServiceCallbacks!=null)
            sAccessibilityServiceCallbacks.onInterrupt(this);

    }

    @Override
    public void onDestroy() {
        if(sAccessibilityServiceCallbacks!=null)
            sAccessibilityServiceCallbacks.onDestroy(this);
        stopScreenshot();
        super.onDestroy();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        if(sAccessibilityServiceCallbacks!=null)
            sAccessibilityServiceCallbacks.onConfigurationChanged(this,newConfig);
    }

    @Override
    protected boolean onKeyEvent(KeyEvent event) {
        if(sAccessibilityServiceCallbacks!=null)
            if(sAccessibilityServiceCallbacks.onKeyEvent(this,event))
                return true;

        return super.onKeyEvent(event);
    }

    public void startScreenshot() {
        mScreenShot = new ScreenShot(this,null);
    }

    public void startScreenshot(VirtualDisplay.Callback callback) {
        mScreenShot = new ScreenShot(this,callback);
    }

    public Bitmap getScreenshot() {
        if (mScreenShot != null)
            return mScreenShot.getScreenShot();
        return null;
    }

    public void getScreenshot(final LuaFunction listener) {
        ScreenShot.getScreenCaptureBitmap(this, new ScreenCaptureListener() {
            @Override
            public void onScreenCaptureDone(Bitmap bitmap) {
                try {
                    listener.call(bitmap);
                } catch (LuaException e) {
                    e.printStackTrace();
                }
            }

            @Override
            public void onScreenCaptureError(String msg) {
                try {
                    listener.call(null,msg);
                } catch (LuaException e) {
                    e.printStackTrace();
                }

            }
        });
    }

    public void stopScreenshot() {
        if (mScreenShot != null)
            mScreenShot.release();
        mScreenShot = null;
    }

    public boolean scrollForward(AccessibilityNodeInfo node) {
        if (node == null)
            return false;

        if (Build.VERSION.SDK_INT < 21) {
            //noinspection deprecation
            if ((node.getActions() & AccessibilityNodeInfo.ACTION_SCROLL_FORWARD) == 0)
                return false;
        } else {
            if (!node.getActionList().contains(AccessibilityNodeInfo.AccessibilityAction.ACTION_SCROLL_FORWARD))
                return false;
        }
        return node.performAction(AccessibilityNodeInfo.ACTION_SCROLL_FORWARD);
    }

    public boolean scrollBackward(AccessibilityNodeInfo node) {
        if (node == null)
            return false;

        if (Build.VERSION.SDK_INT < 21) {
            //noinspection deprecation
            if ((node.getActions() & ACTION_SCROLL_BACKWARD) == 0)
                return false;
        } else {
            if (!node.getActionList().contains(AccessibilityNodeInfo.AccessibilityAction.ACTION_SCROLL_BACKWARD))
                return false;
        }
        return node.performAction(ACTION_SCROLL_BACKWARD);
    }

    public void postExecute(long time, final String menu, final AccessibilityNodeInfo node, final LuaFunction callback) {
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                try {
                    callback.call(execute(menu, node), menu, node);
                } catch (LuaException e) {
                    e.printStackTrace();
                    sendError("postExecute", e);
                }
            }
        }, time);
    }


    private void sendError(String postClick, LuaException e) {
    }

    public void postExecute(long time, final String menu, final AccessibilityNodeInfo node) {
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                execute(menu, node);
            }
        }, time);
    }

    public void postClick(long time, final LuaTable buttons) {
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                click(buttons);
            }
        }, time);
    }

    public void postClick(long time, final LuaTable buttons, final LuaFunction callback) {
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                click(buttons, callback);
            }
        }, time);
    }

    private AccessibilityNodeInfo checkParent(AccessibilityNodeInfo node) {
        return node;
    }

    public boolean click(LuaTable buttons) {
        return new ClickRunnable(this, buttons).canClick();
    }

    public boolean click(LuaTable buttons, final LuaFunction callback) {
        return new ClickRunnable(this, buttons).canClick(new ClickRunnable.ClickCallback() {
            @Override
            public void onDone(boolean bool, LuaTable bs, String name, int idx) {
                try {
                    callback.call(bool, bs, name, idx);
                } catch (LuaException e) {
                    e.printStackTrace();
                    sendError("click", e);
                }
            }
        });
    }

    public ClickRunnable loopClick(final LuaTable buttons) {
        ClickRunnable click = new ClickRunnable(this, buttons);
        click.canClick(new ClickRunnable.ClickCallback() {
            @Override
            public void onDone(boolean bool, LuaTable bs, String name, int idx) {
                loopClick(buttons);
            }
        });
        return click;
    }


    public boolean findClick(String[] buttons) {
        for (String name : buttons) {
            AccessibilityNodeInfo button = findAccessibilityNodeInfoByText(name, 0);
            if (button != null) {
                button = checkClick(button);
                boolean ret = button.performAction(ACTION_CLICK);
                if (ret)
                    return true;
            }
        }
        return false;
    }

    public boolean click(String appName, String lable, int[] buttons) {
        if (appName == null || lable == null)
            return false;
        //print("click",getAppName(getFocusView()));
        if (!appName.equals(getAppName(getFocusView())))
            return false;
        AccessibilityNodeInfo root = getRootInActiveWindow();
        if (root == null)
            return false;
        //print("click",root.findAccessibilityNodeInfosByText(lable));
        if (root.findAccessibilityNodeInfosByText(lable).isEmpty())
            return false;
        for (int i : buttons) {
            if (root.getChildCount() <= i)
                return false;
            root = root.getChild(i);
            if (root == null)
                return false;
        }
        //print("click",root);
        return toClick(root);
    }

    private void findNodeInfoByText(List<AccessibilityNodeInfo> ret, AccessibilityNodeInfo node, String keyword) {
        if (node == null)
            return;
        CharSequence[] t = new CharSequence[2];
        String[] names = keyword.split("\\|");
        t[0] = node.getContentDescription();
        t[1] = node.getText();
        for (String name : names) {
            boolean start = !name.startsWith("*");
            boolean end = !name.endsWith("*");
            if (!start)
                name = name.substring(1);
            if (!end)
                name = name.substring(0, name.length() - 1);

            for (CharSequence d : t) {
                if (d == null)
                    continue;

                String text = d.toString().trim();
                if (start && end) {
                    if (name.equals(text)){
                        ret.add(node);
                        break;
                    }
                } else if (start) {
                    if (text.startsWith(name)){
                        ret.add(node);
                        break;
                    }
                } else if (end) {
                    if (text.endsWith(name)){
                        ret.add(node);
                        break;
                    }
                } else {
                    if (text.contains(name)){
                        ret.add(node);
                        break;
                    }
                }

            }
        }
        int c = node.getChildCount();
        for (int i = 0; i < c; i++) {
            findNodeInfoByText(ret, node.getChild(i), keyword);
        }
    }

    public List<AccessibilityNodeInfo> findAccessibilityNodeInfoByText(String keyword) {
        AccessibilityNodeInfo root = getRootInActiveWindow();
        List<AccessibilityNodeInfo> ret = new ArrayList<>();
        if (root == null)
            return ret;
        String[] names = keyword.split("\\|");
        for (String name : names) {
            if (name.isEmpty())
                continue;
            char c = name.charAt(0);
            switch (c) {
                case '%':
                    execute(name.substring(1), getFocusView());
                    return ret;
            }

            int idx = name.lastIndexOf('&');
            if (idx > 0) {
                AccessibilityNodeInfo node = findAccessibilityNodeInfo(name.substring(idx + 1));
                if (node == null)
                    continue;
                name = name.substring(0, idx);
            }

            boolean start = !name.startsWith("*");
            boolean end = !name.endsWith("*");
            if (!start)
                name = name.substring(1);
            if (!end)
                name = name.substring(0, name.length() - 1);
            List<AccessibilityNodeInfo> list = root.findAccessibilityNodeInfosByText(name);
            for (AccessibilityNodeInfo node : list) {
                String text = (node.getText() + "").trim();
                String des = (node.getContentDescription() + "").trim();
                if (start && end) {
                    if (name.equals(text) || name.equals(des))
                        ret.add(node);
                } else if (start) {
                    if (text.startsWith(name) || des.startsWith(name))
                        ret.add(node);
                } else if (end) {
                    if (text.endsWith(name) || des.endsWith(name))
                        ret.add(node);
                } else {
                    if (text.contains(name) || des.contains(name))
                        ret.add(node);
                }
            }
        }
        if (ret.isEmpty())
            findNodeInfoByText(ret, root, keyword);
        return ret;
    }

    public AccessibilityNodeInfo findAccessibilityNodeInfoByText(String name, int i) {
        List<AccessibilityNodeInfo> ret = findAccessibilityNodeInfoByText(name);
        if (ret.isEmpty())
            return null;
        int size = ret.size();

        if (i + 1 > size || 0 - i > size)
            return null;
        if (i < 0)
            return ret.get(ret.size() + i);
        else
            return ret.get(i);
    }

    public List<AccessibilityNodeInfo> findAccessibilityNodeInfoById(String name) {
        AccessibilityNodeInfo root = getRootInActiveWindow();
        if (root == null)
            return new ArrayList<>();
        return root.findAccessibilityNodeInfosByText(name);
    }

    public AccessibilityNodeInfo findAccessibilityNodeInfoById(String name, int i) {
        List<AccessibilityNodeInfo> ret = findAccessibilityNodeInfoById(name);
        if (ret.isEmpty())
            return null;
        int size = ret.size();

        if (i + 1 > size || 0 - i > size)
            return null;
        if (i < 0)
            return ret.get(ret.size() + i);
        else
            return ret.get(i);
    }

    public AccessibilityNodeInfo findAccessibilityNodeInfoByIndex(String name) {
        AccessibilityNodeInfo root = getRootInActiveWindow();
        if (root == null)
            return null;
        String[] buttons = name.split("-");
        for (String s : buttons) {
            try {
                int i = Integer.valueOf(s);
                if (root.getChildCount() <= i)
                    return null;
                root = root.getChild(i);
                if (root == null)
                    return null;
            } catch (Exception e) {
                e.printStackTrace();
                return null;
            }
        }
        return root;
    }


    public AccessibilityNodeInfo findAccessibilityNodeInfo(String name) {
        int idx = name.lastIndexOf("@");
        if (idx > 0) {
            String app = name.substring(idx + 1);
            if (!app.equals(getAppName(getFocusView())))
                return null;
            name = name.substring(0, idx);
        }
        idx = name.lastIndexOf("#");
        int i = -1;
        if (idx > 0) {
            try {
                i = Integer.valueOf(name.substring(idx + 1));
            } catch (Exception e) {
                i = -1;
            }
            name = name.substring(0, idx);
        }
        switch (name.charAt(0)) {
            case '%':
                if (execute(name.substring(1), getFocusView()))
                    return AccessibilityNodeInfo.obtain();
                else
                    return null;
            case '>':
                if (startApp(name.substring(1)))
                    return AccessibilityNodeInfo.obtain();
                else
                    return null;
            case '@':
                return findAccessibilityNodeInfoById(name.substring(1), i);
            case '$':
                return findAccessibilityNodeInfoByIndex(name.substring(1));
            default:
                return findAccessibilityNodeInfoByText(name, i);
        }
    }

    public boolean execute(String menu, final AccessibilityNodeInfo node) {
        boolean ret = false;
        switch (menu) {
            case "向上翻页":
                AccessibilityNodeInfo list = findListView(getRootInActiveWindow());
                if (list == null)
                    return false;
                ret = scrollBackward(list);
                return ret;
            case "向下翻页":
                AccessibilityNodeInfo list2 = findListView(getRootInActiveWindow());
                if (list2 == null)
                    return false;
                ret = scrollForward(list2);
                return ret;
            case "减少进度":
                return scrollBackward(node);
            case "增加进度":
                return scrollForward(node);
            case "粘贴":
                paste(node);
                break;
            case "最近任务":
                toRecents();
                break;
            case "清空":
                if (Build.VERSION.SDK_INT >= 21) {
                    return node.performAction(AccessibilityNodeInfo.ACTION_SET_TEXT);
                } else {
                    return false;
                }
            case "复制":
                copy(getText(node));
                break;
            case "追加复制":
                appendCopy(getText(node));
                break;
            case "主屏幕":
                toHome();
                break;
            case "返回":
                toBack();
                break;
            case "点击":
                toClick(node);
                break;
            case "长按":
                toLongClick(node);
                break;
            case "通知栏":
            case "打开通知栏":
                toNotifications();
                break;
            default:
                return false;
        }

        return true;
    }

    public boolean isListView2(AccessibilityNodeInfo source) {
        if (source == null)
            return false;
        CharSequence className = source.getClassName();
        if (className != null) {
            String name = className.toString();
            switch (name) {
                case "android.widget.AdapterView":
                case "android.widget.ListView":
                case "android.widget.GridView":
                case "android.widget.AbsListView":
                case "android.widget.ExpandableListView":
                case "android.support.v7.widget.RecyclerView":
                case "flyme.support.v7.widget.RecyclerView":
                case "android.widget.ScrollView":
                case "android.widget.HorizontalScrollView":
                case "com.tencent.widget.GridView":
                    return true;
                default:
                    if (name.endsWith("ScrollView"))
                        return true;
                    else if (name.endsWith("GridView"))
                        return true;
                    else if (name.endsWith("RecyclerView"))
                        return true;
                    else if (name.endsWith("ListView"))
                        return true;
            }
        }
        return false;
    }

    public String getText(AccessibilityNodeInfo source) {
        return getNodeInfoText(source);
    }

    private AccessibilityNodeInfo findListView(AccessibilityNodeInfo node) {
        if (node == null)
            return null;
        if (isListView2(node))
            return node;
        int count = node.getChildCount();
        for (int i = 0; i < count; i++) {
            AccessibilityNodeInfo list = findListView(node.getChild(i));
            if (list != null)
                return list;
        }
        return null;
    }

    public boolean insert(AccessibilityNodeInfo mEditView, CharSequence text) {
        if (mEditView == null)
            return false;

        if (text == null)
            return false;
        if (mEditView.isEditable()) {
            if (!mEditView.isFocused())
                mEditView.performAction(ACTION_FOCUS);

            ClipboardManager cm = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);
            cm.setPrimaryClip(ClipData.newPlainText("label", text));
            cm.setText(text);
            if (mEditView.performAction(AccessibilityNodeInfo.ACTION_PASTE)) {
                return true;
            }
        }
        return false;
    }

    public boolean paste(AccessibilityNodeInfo mEditView, CharSequence text) {
        if (mEditView == null)
            return false;

        if (text == null)
            return false;
        if (mEditView.isEditable()) {
            if (!mEditView.isFocused())
                mEditView.performAction(ACTION_FOCUS);

            ClipboardManager cm = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);
            cm.setPrimaryClip(ClipData.newPlainText("label", text));
            cm.setText(text);
            if (mEditView.performAction(AccessibilityNodeInfo.ACTION_PASTE)) {
                return true;
            }
        }
        return paste(text);
    }

    public boolean paste(AccessibilityNodeInfo mEditView) {
        ClipboardManager cm = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);
        return paste(mEditView, cm.getText());
    }

    public boolean paste() {
        ClipboardManager cm = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);
        return paste(getFocusView(), cm.getText());
    }

    public boolean paste(CharSequence text) {
        if (text == null)
            return false;
        AccessibilityNodeInfo mEditView = getEditText();
        if (mEditView == null) {
            return false;
        }
        if (getFocusView().isEditable() && getFocusView().getText() != null)
            text = getFocusView().getText().toString() + text;
        if (Build.VERSION.SDK_INT >= 21) {
            Bundle arguments = new Bundle();
            arguments.putCharSequence(AccessibilityNodeInfo.ACTION_ARGUMENT_SET_TEXT_CHARSEQUENCE, text);
            return mEditView.performAction(AccessibilityNodeInfo.ACTION_SET_TEXT, arguments);
        } else {
            return false;
        }
    }


    public boolean copy() {
        return copy(getText(getFocusView()));
    }

    public boolean copy(CharSequence t) {
        if (t == null) {
            return false;
        }
        String text = t.toString();
        ClipboardManager cm = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);
        cm.setPrimaryClip(ClipData.newPlainText("label", text));
        return true;
    }

    public boolean appendCopy() {
        return appendCopy(getText(getFocusView()));
    }

    public boolean appendCopy(CharSequence t) {
        if (t == null) {
            return false;
        }
        ClipboardManager cm = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);
        CharSequence c = cm.getText();
        String text = "";
        if (c != null)
            text = c.toString();
        if (text.length() > 1)
            text = text + "\n";
        cm.setPrimaryClip(ClipData.newPlainText("label", text + t));
        return true;
    }

    public boolean setText(String s) {
        return setText(getEditText(), s);
    }

    public boolean setText(AccessibilityNodeInfo mEditView, String s) {
        if (mEditView == null || !mEditView.isEditable()) {
            return false;
        }
        if (Build.VERSION.SDK_INT >= 21) {
            Bundle arguments = new Bundle();
            arguments.putCharSequence(AccessibilityNodeInfo.ACTION_ARGUMENT_SET_TEXT_CHARSEQUENCE, s);
            return mEditView.performAction(AccessibilityNodeInfo.ACTION_SET_TEXT, arguments);
        } else {
            return paste(mEditView, s);
        }
    }

    public AccessibilityNodeInfo getEditText() {
        ArrayList<AccessibilityNodeInfo> editList = getAllEditTextList();
        if (editList.isEmpty()) {
            return null;
        }
        AccessibilityNodeInfo mHoverView = editList.get(0);
        if (mHoverView != null)
            mHoverView.performAction(ACTION_ACCESSIBILITY_FOCUS);
        return mHoverView;
    }

    public ArrayList<AccessibilityNodeInfo> getAllEditTextList() {
        ArrayList<AccessibilityNodeInfo> textList = new ArrayList<AccessibilityNodeInfo>();
        AccessibilityNodeInfo node = getRootInActiveWindow();
        getEditText(node, textList);
        return textList;
    }

    public void getEditText(AccessibilityNodeInfo node, ArrayList<AccessibilityNodeInfo> textList) {
        if (node == null)
            return;
        if (node.isEditable())
            textList.add(node);
        int count = node.getChildCount();
        if (count > 0) {
            for (int i = 0; i < count; i++)
                getEditText(node.getChild(i), textList);
        }
    }

    public String getAllText(int minLength) {
        ArrayList<String> list = getAllTextList();
        StringBuilder buf = new StringBuilder();
        for (String text : list) {
            if (text.length() > minLength)
                buf.append(text).append("\n");
        }
        return buf.toString();
    }


    public ArrayList<String> getAllTextList(AccessibilityNodeInfo focus) {
        ArrayList<String> textList = new ArrayList<String>();
        AccessibilityNodeInfo node = getRootInActiveWindow();
        mOk = !focus.isVisibleToUser();
        getText(node, textList, focus);
        return textList;
    }

    public void toBack() {
        performGlobalAction(AccessibilityService.GLOBAL_ACTION_BACK);
    }

    public void toHome() {
        performGlobalAction(AccessibilityService.GLOBAL_ACTION_HOME);
    }

    public void toRecents() {
        performGlobalAction(AccessibilityService.GLOBAL_ACTION_RECENTS);
    }

    public void toNotifications() {
        performGlobalAction(AccessibilityService.GLOBAL_ACTION_NOTIFICATIONS);
    }

    public boolean toClick(AccessibilityNodeInfo nodeInfo) {
        if (nodeInfo != null) {
            try {
                return nodeInfo.performAction(ACTION_CLICK);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        return false;
    }

    public boolean toLongClick(AccessibilityNodeInfo node) {
        if (node != null) {
            try {
                return node.performAction(ACTION_LONG_CLICK);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        return false;
    }

    private void getText(AccessibilityNodeInfo node, ArrayList<String> textList, AccessibilityNodeInfo focus) {
        if (node == null)
            return;
        /*if (!node.isVisibleToUser()&&isInListView(node))
            return;*/
        if (!mOk)
            mOk = node.equals(focus);
        CharSequence text = getNodeInfoText(node);
        if (mOk) {
            if (text != null)
                textList.add(text.toString());
        }
        int count = node.getChildCount();
        if (count > 0) {
            for (int i = 0; i < count; i++) {
                AccessibilityNodeInfo child = node.getChild(i);
                if (child == null)
                    continue;
                if (!mOk)
                    mOk = child.equals(focus);
                getText(child, textList, focus);
            }
        }
    }

    public String getNodeInfoText(AccessibilityNodeInfo source) {
        if (source == null)
            return null;
        CharSequence ct = source.getContentDescription();
        CharSequence text = source.getText();
        String contentDescription = null;
        if (ct != null)
            contentDescription = ct.toString();
        if (contentDescription != null && contentDescription.trim().length() > 0 && (!source.isEditable() || text == null)) {
            return contentDescription;
        } else if (text != null && text.length() > 0) {
            return text.toString();
        }
        return null;
    }


    public ArrayList<String> getAllTextList() {
        ArrayList<String> textList = new ArrayList<String>();
        AccessibilityNodeInfo node = getRootInActiveWindow();
        getText(node, textList);
        return textList;
    }

    private void getText(AccessibilityNodeInfo node, ArrayList<String> textList) {
        if (node == null)
            return;
        /*if (!node.isVisibleToUser()&&isInListView(node))
            return;*/
        CharSequence text = getNodeInfoText(node);
        int count = node.getChildCount();
        if (count > 0) {
            for (int i = 0; i < count; i++)
                getText(node.getChild(i), textList);
        }
    }


    private void asyncGetAllApp() {

        new AsyncTask<String, String, HashMap<String, ComponentName>>() {

            @Override
            protected HashMap<String, ComponentName> doInBackground(String... params) {
                HashMap<String, ComponentName> appMap = new HashMap<>();
                PackageManager manager = getPackageManager();
                Intent mainIntent = new Intent(Intent.ACTION_MAIN, null); //取出Intent 为Action_Main的程序
                mainIntent.addCategory(Intent.CATEGORY_DEFAULT); //分辨出位默认Laucher启动的程序

                List<ResolveInfo> apps = manager.queryIntentActivities(mainIntent, 0); //利用包管理器将起取出来
                Collections.sort(apps, new ResolveInfo.DisplayNameComparator(manager));

                int count = apps.size();
                for (int i = 0; i < count; i++) {
                    //ApplicationInfo application = new ApplicationInfo();
                    ResolveInfo info = apps.get(i);

                    CharSequence title = info.loadLabel(manager);
                    ComponentName componentName = new ComponentName(
                            info.activityInfo.applicationInfo.packageName,
                            info.activityInfo.name);
                    appMap.put(title.toString().toLowerCase(), componentName);
                }

                mainIntent = new Intent(Intent.ACTION_MAIN, null); //取出Intent 为Action_Main的程序
                mainIntent.addCategory(Intent.CATEGORY_LAUNCHER); //分辨出位默认Laucher启动的程序

                apps = manager.queryIntentActivities(mainIntent, 0); //利用包管理器将起取出来
                Collections.sort(apps, new ResolveInfo.DisplayNameComparator(manager));

                count = apps.size();
                for (int i = 0; i < count; i++) {
                    //ApplicationInfo application = new ApplicationInfo();
                    ResolveInfo info = apps.get(i);

                    CharSequence title = info.loadLabel(manager);
                    ComponentName componentName = new ComponentName(
                            info.activityInfo.applicationInfo.packageName,
                            info.activityInfo.name);
                    appMap.put(title.toString().toLowerCase(), componentName);
                }
                return appMap;
            }

            @Override
            protected void onPostExecute(HashMap<String, ComponentName> stringComponentNameHashMap) {
                super.onPostExecute(stringComponentNameHashMap);
                if (stringComponentNameHashMap != null && !stringComponentNameHashMap.isEmpty())
                    appMap = stringComponentNameHashMap;
            }
        }.execute("");
    }


    private void getAllApp() {
        PackageManager manager = getPackageManager();
        Intent mainIntent = new Intent(Intent.ACTION_MAIN, null); //取出Intent 为Action_Main的程序
        mainIntent.addCategory(Intent.CATEGORY_DEFAULT); //分辨出位默认Laucher启动的程序

        final List<ResolveInfo> apps = manager.queryIntentActivities(mainIntent, 0); //利用包管理器将起取出来
        Collections.sort(apps, new ResolveInfo.DisplayNameComparator(manager));

        if (apps != null) {
            final int count = apps.size();
            appMap.clear();
            for (int i = 0; i < count; i++) {
                //ApplicationInfo application = new ApplicationInfo();
                ResolveInfo info = apps.get(i);

                CharSequence title = info.loadLabel(manager);
                ComponentName componentName = new ComponentName(
                        info.activityInfo.applicationInfo.packageName,
                        info.activityInfo.name);
                appMap.put(title.toString().toLowerCase(), componentName);
            }
        }
        getAllApp2();
    }

    private void getAllApp2() {
        PackageManager manager = getPackageManager();
        Intent mainIntent = new Intent(Intent.ACTION_MAIN, null); //取出Intent 为Action_Main的程序
        mainIntent.addCategory(Intent.CATEGORY_LAUNCHER); //分辨出位默认Laucher启动的程序

        final List<ResolveInfo> apps = manager.queryIntentActivities(mainIntent, 0); //利用包管理器将起取出来
        Collections.sort(apps, new ResolveInfo.DisplayNameComparator(manager));

        if (apps != null) {
            final int count = apps.size();
            for (int i = 0; i < count; i++) {
                //ApplicationInfo application = new ApplicationInfo();
                ResolveInfo info = apps.get(i);

                CharSequence title = info.loadLabel(manager);
                ComponentName componentName = new ComponentName(
                        info.activityInfo.applicationInfo.packageName,
                        info.activityInfo.name);
                appMap.put(title.toString().toLowerCase(), componentName);
            }
        }
    }

    public boolean startApp(String appName) {
        asyncGetAllApp();
        appName = appName.toLowerCase();
        ComponentName className = appMap.get(appName);
        if (className == null)
            return false;
        Intent intent = new Intent(Intent.ACTION_MAIN);
        intent.addCategory(Intent.CATEGORY_DEFAULT);
        intent.setComponent(className);
        intent.setFlags(FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED);
        try {
            startActivity(intent);
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
        return true;
    }

    public boolean deleteApp(String appName) {
        appMap.clear();
        getAllApp2();
        appName = appName.toLowerCase();
        ComponentName className = appMap.get(appName);
        if (className == null)
            return false;
        Intent intent = new Intent(Intent.ACTION_DELETE, Uri.parse("package:" + className.getPackageName()));
        intent.setFlags(FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED);
        startActivity(intent);
        return true;
    }

    public boolean installApp(String appName) {
        if (appName == null)
            return false;
        Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse("market://search?q=" + appName));
        intent.setFlags(FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED);
        try {
            startActivity(intent);
        } catch (Exception e) {
            return false;
        }
        return true;
    }

    public AccessibilityNodeInfo getFocusView() {
        return getRootInActiveWindow();
    }

    public String getAppName(AccessibilityNodeInfo node) {
        if (node == null)
            return "";
        CharSequence pkn = node.getPackageName();
        if (pkn == null)
            return "";
        String pkg = pkn.toString();
        PackageManager pm = getPackageManager();
        try {
            ApplicationInfo info = pm.getApplicationInfo(pkg, 0);
            return pm.getApplicationLabel(info).toString();
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
            return "";
        }
    }

    public Handler getHandler() {
        return handler;
    }

    public boolean isClickable(AccessibilityNodeInfo source) {
        if (source == null)
            return false;
        if (source.isClickable())
            return true;
        if (source.isCheckable())
            return true;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            if (source.getActionList().contains(AccessibilityNodeInfo.AccessibilityAction.ACTION_CLICK))
                return true;
        } else {
            if ((source.getActions() & AccessibilityNodeInfo.ACTION_CLICK) != 0)
                return true;
        }
        return false;
    }

    private AccessibilityNodeInfo checkClick(AccessibilityNodeInfo node) {
        try {
            AccessibilityNodeInfo parent = node;
            while (parent != null) {
                if (isClickable(parent))
                    return parent;
                parent = parent.getParent();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return node;
    }


    public void toClick2(AccessibilityNodeInfo node) {
        toClick(checkClick(node));
    }
}

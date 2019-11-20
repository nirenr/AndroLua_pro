package com.androlua;

import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.widget.AbsListView;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.luajava.LuaException;
import com.luajava.LuaFunction;
import com.luajava.LuaJavaAPI;
import com.luajava.LuaObject;
import com.luajava.LuaState;
import com.luajava.LuaTable;

import java.io.IOException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

/**
 * Created by Administrator on 2017/02/27 0027.
 */

public class LuaMultiAdapter extends BaseAdapter {

    private BitmapDrawable mDraw;
    private Resources mRes;
    private LuaState L;
    private LuaContext mContext;
    private LuaTable<Integer, LuaTable> mLayout;
    private LuaTable<Integer, LuaTable<String, Object>> mData;
    private LuaTable<String, Object> mTheme;
    private LuaFunction<View> loadLayout;
    private LuaFunction insert;
    private LuaFunction remove;
    private LuaTable<Integer, LuaFunction<Animation>> mAnimationUtil;
    private HashMap<View, Animation> mAnimCache = new HashMap<View, Animation>();
    private HashMap<View, Boolean> mStyleCache = new HashMap<View, Boolean>();

    private boolean mNotifyOnChange = true;
    private boolean updateing;
    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            notifyDataSetChanged();
        }

    };
    private HashMap<String, Boolean> loaded = new HashMap<String, Boolean>();

    public LuaMultiAdapter(LuaContext context, LuaTable layout) throws LuaException {
        this(context, null, layout);
    }

    public LuaMultiAdapter(LuaContext context, LuaTable<Integer, LuaTable<String, Object>> data, LuaTable<Integer, LuaTable> layout) throws LuaException {
        mContext = context;
        mLayout = layout;
        mRes = mContext.getContext().getResources();

        L = context.getLuaState();
        if (data == null)
            data = new LuaTable<Integer, LuaTable<String, Object>>(L);
        mData = data;
        loadLayout = (LuaFunction<View>) L.getLuaObject("loadlayout").getFunction();
        insert = L.getLuaObject("table").getField("insert").getFunction();
        remove = L.getLuaObject("table").getField("remove").getFunction();
        int len = mLayout.length();
        for (int i = 1; i <= len; i++) {
            L.newTable();
            loadLayout.call(mLayout.get(i), L.getLuaObject(-1), AbsListView.class);
            L.pop(1);
        }
    }

    @Override
    public int getViewTypeCount() {
        return mLayout.length();
    }

    @Override
    public int getItemViewType(int position) {
        try{
            int t = mData.get(position + 1).get("__type",Integer.class) - 1;
            return t < 0 ? 0 : t;
        } catch (Exception e) {
            e.printStackTrace();
            return 0;
        }
     }


    public void setAnimation(LuaTable<Integer, LuaFunction<Animation>> animation) {
        setAnimationUtil(animation);
    }

    public void setAnimationUtil(LuaTable<Integer, LuaFunction<Animation>> animation) {
        mAnimCache.clear();
        mAnimationUtil = animation;
    }

    @Override
    public int getCount() {
        // TODO: Implement this method
        return mData.length();
    }

    @Override
    public Object getItem(int position) {
        // TODO: Implement this method
        return mData.get(position + 1);
    }

    @Override
    public long getItemId(int position) {
        // TODO: Implement this method
        return position + 1;
    }

    public LuaTable<Integer, LuaTable<String, Object>> getData() {
        return mData;
    }

    public void add(LuaTable<String, Object> item) throws Exception {
        insert.call(mData, item);
        if (mNotifyOnChange) notifyDataSetChanged();
    }

    public void addAll(LuaTable<Integer, LuaTable<String, Object>> items) throws Exception {
        int len = items.length();
        for (int i = 1; i <= len; i++)
            insert.call(mData, items.get(i));
        if (mNotifyOnChange) notifyDataSetChanged();
    }


    public void insert(int position, LuaTable<String, Object> item) throws Exception {
        insert.call(mData, position + 1, item);
        if (mNotifyOnChange) notifyDataSetChanged();
    }

    public void remove(int position) throws Exception {
        remove.call(mData, position + 1);
        if (mNotifyOnChange) notifyDataSetChanged();
    }

    public void clear() {
        mData.clear();
        if (mNotifyOnChange) notifyDataSetChanged();
    }

    public void setNotifyOnChange(boolean notifyOnChange) {
        mNotifyOnChange = notifyOnChange;
        if (mNotifyOnChange) notifyDataSetChanged();
    }

    @Override
    public void notifyDataSetChanged() {
        // TODO: Implement this method
        super.notifyDataSetChanged();
        if (!updateing) {
            updateing = true;
            new Handler().postDelayed(new Runnable() {
                @Override
                public void run() {
                    // TODO: Implement this method
                    updateing = false;
                }
            }, 500);
        }
    }

    @Override
    public View getDropDownView(int position, View convertView, ViewGroup parent) {
        // TODO: Implement this method
        return getView(position, convertView, parent);
    }

    public void setStyle(LuaTable<String, Object> theme) {
        mStyleCache.clear();
        mTheme = theme;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        // TODO: Implement this method
        View view = null;
        LuaObject holder = null;
        int t = mData.get(position + 1).get("__type", Integer.class);
        t = t < 1 ? 1 : t;
        if (convertView == null) {
            try {
                LuaTable layout = mLayout.get(t);
                L.newTable();
                holder = L.getLuaObject(-1);
                L.pop(1);
                view = loadLayout.call(layout, holder, AbsListView.class);
                view.setTag(holder);
                //mHolderCache.put(view,holder);
            } catch (LuaException e) {
                return new View(mContext.getContext());
            }
        } else {
            view = convertView;
            holder = (LuaObject) view.getTag();
            //holder = mHolderCache.get(view);
        }

        LuaTable<String, Object> hm = mData.get(position + 1);

        if (hm == null) {
            Log.i("lua", position + " is null");
            return view;
        }

        boolean bool = mStyleCache.get(view) == null;
        if (bool)
            mStyleCache.put(view, true);

        Set<Map.Entry<String, Object>> sets = hm.entrySet();
        for (Map.Entry<String, Object> entry : sets) {
            try {
                String key = entry.getKey();
                if (key.equals("type"))
                    continue;
                Object value = entry.getValue();
                LuaObject obj = holder.getField(key);
                if (obj.isJavaObject()) {
                    if (mTheme != null && bool) {
                        setHelper((View) obj.getObject(), mTheme.get(key));
                    }
                    setHelper((View) obj.getObject(), value);
                }
            } catch (Exception e) {
                Log.i("lua", e.getMessage());
            }
        }

        if (updateing) {
            return view;
        }

        if (mAnimationUtil != null && convertView != null) {
            Animation anim = mAnimCache.get(convertView);
            if (anim == null) {
                try {
                    anim = mAnimationUtil.get(t).call();
                    mAnimCache.put(convertView, anim);
                } catch (Exception e) {
                    mContext.sendError("setAnimation", e);
                }
            }
            if (anim != null) {
                view.clearAnimation();
                view.startAnimation(anim);
            }
        }
        return view;
    }

    private void setFields(View view, LuaTable<String, Object> fields) throws LuaException {
        Set<Map.Entry<String, Object>> sets = fields.entrySet();
        for (Map.Entry<String, Object> entry2 : sets) {
            String key2 = entry2.getKey();
            Object value2 = entry2.getValue();
            if (key2.toLowerCase().equals("src"))
                setHelper(view, value2);
            else
                javaSetter(view, key2, value2);

        }
    }

    private void setHelper(View view, Object value) {
        try {
            if (value instanceof LuaTable) {
                setFields(view, (LuaTable<String, Object>) value);
            } else if (view instanceof TextView) {
                if (value instanceof CharSequence)
                    ((TextView) view).setText((CharSequence) value);
                else
                    ((TextView) view).setText(value.toString());
            } else if (view instanceof ImageView) {
                if (value instanceof Bitmap)
                    ((ImageView) view).setImageBitmap((Bitmap) value);
                else if (value instanceof String)
                    ((ImageView) view).setImageDrawable(new LuaMultiAdapter.AsyncLoader().getBitmap(mContext, (String) value));
                else if (value instanceof Drawable)
                    ((ImageView) view).setImageDrawable((Drawable) value);
                else if (value instanceof Number)
                    ((ImageView) view).setImageResource(((Number) value).intValue());
            }
        } catch (Exception e) {
            mContext.sendError("setHelper", e);
        }
    }

    private int javaSetter(Object obj, String methodName, Object value) throws LuaException {

        if (methodName.length() > 2 && methodName.substring(0, 2).equals("on") && value instanceof LuaFunction)
            return javaSetListener(obj, methodName, value);

        return javaSetMethod(obj, methodName, value);
    }

    private int javaSetListener(Object obj, String methodName, Object value) throws LuaException {
        String name = "setOn" + methodName.substring(2) + "Listener";
        ArrayList<Method> methods = LuaJavaAPI.getMethod(obj.getClass(), name, false);
        for (Method m : methods) {

            Class<?>[] tp = m.getParameterTypes();
            if (tp.length == 1 && tp[0].isInterface()) {
                L.newTable();
                L.pushObjectValue(value);
                L.setField(-2, methodName);
                try {
                    Object listener = L.getLuaObject(-1).createProxy(tp[0]);
                    m.invoke(obj, listener);
                    return 1;
                } catch (Exception e) {
                    throw new LuaException(e);
                }
            }
        }
        return 0;
    }

    private int javaSetMethod(Object obj, String methodName, Object value) throws LuaException {
        if (Character.isLowerCase(methodName.charAt(0))) {
            methodName = Character.toUpperCase(methodName.charAt(0)) + methodName.substring(1);
        }
        String name = "set" + methodName;
        Class<?> type = value.getClass();
        StringBuilder buf = new StringBuilder();


        ArrayList<Method> methods = LuaJavaAPI.getMethod(obj.getClass(), name, false);

        for (Method m : methods) {
            Class<?>[] tp = m.getParameterTypes();
            if (tp.length != 1)
                continue;

            if (tp[0].isPrimitive()) {
                try {
                    if (value instanceof Double || value instanceof Float) {
                        m.invoke(obj, LuaState.convertLuaNumber(((Number) value).doubleValue(), tp[0]));
                    } else if (value instanceof Long || value instanceof Integer) {
                        m.invoke(obj, LuaState.convertLuaNumber(((Number) value).longValue(), tp[0]));
                    } else if (value instanceof Boolean) {
                        m.invoke(obj, (Boolean) value);
                    } else {
                        continue;
                    }
                    return 1;
                } catch (Exception e) {
                    buf.append(e.getMessage());
                    buf.append("\n");
                    continue;
                }

            }

            if (!tp[0].isAssignableFrom(type))
                continue;

            try {
                m.invoke(obj, value);
                return 1;
            } catch (Exception e) {
                buf.append(e.getMessage());
                buf.append("\n");
                continue;
            }
        }
        if (buf.length() > 0)
            throw new LuaException("Invalid setter " + methodName + ". Invalid Parameters.\n" + buf.toString() + type.toString());
        else
            throw new LuaException("Invalid setter " + methodName + " is not a method.\n");

    }

    private class AsyncLoader extends Thread {

        private String mPath;

        private LuaContext mContext;

        public Drawable getBitmap(LuaContext context, String path) throws IOException {
            // TODO: Implement this method
            mContext = context;
            mPath = path;
            if (!path.toLowerCase().startsWith("http://")&&!path.toLowerCase().startsWith("https://"))
                return new BitmapDrawable(mRes, LuaBitmap.getBitmap(context, path));
            if (LuaBitmap.checkCache(context, path))
                return new BitmapDrawable(mRes, LuaBitmap.getBitmap(context, path));
            if (!loaded.containsKey(mPath)) {
                start();
                loaded.put(mPath, true);
            }

            return new LoadingDrawable(mContext.getContext());
        }

        @Override
        public void run() {
            // TODO: Implement this method
            try {
                LuaBitmap.getBitmap(mContext, mPath);
                mHandler.sendEmptyMessage(0);
            } catch (IOException e) {
                mContext.sendError("AsyncLoader", e);
            }

        }
    }


}

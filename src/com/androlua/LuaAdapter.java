package com.androlua;
import android.graphics.*;
import android.graphics.drawable.*;
import android.os.*;
import android.view.*;
import android.widget.*;
import com.luajava.*;
import java.lang.ref.*;
import java.util.*;
import java.io.*;
import android.util.*;
import android.view.animation.*;
import android.content.*;
import java.lang.reflect.*;
import com.androlua.LuaAdapter.*;
import com.luajava.LuaTable.*;

public class LuaAdapter extends BaseAdapter {

	private LuaState L;
	private LuaContext mContext;


	private LuaTable mLayout;
	private LuaTable<Integer,LuaTable<String,Object>> mData;
	private LuaTable<String,Object> mTheme;


	private LuaFunction<View> loadlayout;

	private LuaFunction insert;

	private LuaFunction remove;

	private LuaFunction<Animation> mAnimationUtil;

	private HashMap<View,Animation> mAnimCache = new HashMap<View,Animation>();

	private HashMap<View,Boolean> mStyleCache = new HashMap<View,Boolean>();
	
	private boolean mNotifyOnChange=true;

	private boolean updateing;


	public LuaAdapter(LuaContext context, LuaTable layout) throws LuaException {
		this(context, null, layout);
	}

	public LuaAdapter(LuaContext context, LuaTable<Integer,LuaTable<String,Object>> data, LuaTable layout) throws LuaException {
		mContext = context;
		mLayout = layout;
		L = context.getLuaState();
		if (data == null)
			data = new LuaTable<Integer,LuaTable<String,Object>>(L);
		mData = data;
		loadlayout = (LuaFunction<View>)L.getLuaObject("loadlayout").getFunction();
		insert = L.getLuaObject("table").getField("insert").getFunction();
		remove = L.getLuaObject("table").getField("remove").getFunction();
		L.newTable();
		loadlayout.call(mLayout, L.getLuaObject(-1) , AbsListView.class);
		L.pop(1);
		
	}

	public void setAnimationUtil(LuaFunction<Animation> animation) {
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

	public LuaTable<Integer,LuaTable<String,Object>> getData(){
		return mData;
	}
	
	public void add(LuaTable<String,Object> item) throws Exception {
		mData.put(mData.length() + 1, item);
		if (mNotifyOnChange) notifyDataSetChanged();
	}

	public void insert(int position, LuaTable<String,Object> item) throws Exception {
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
		if (updateing == false) {
			updateing = true;
			new Handler().postDelayed(new Runnable(){
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

	public void setStyle(LuaTable<String,Object> theme) {
		mStyleCache.clear();
		mTheme = theme;
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		// TODO: Implement this method
		View view = null;
		LuaObject holder = null;
		if (convertView == null) {
			try {
				L.newTable();
				holder = L.getLuaObject(-1);
				L.pop(1);
				view = loadlayout.call(mLayout, holder, AbsListView.class);
				view.setTag(holder);
			}
			catch (LuaException e) {
				return new View(mContext.getContext());
			}
		}
		else {
			view = convertView;
			holder = (LuaObject)view.getTag();
		}

		LuaTable<String, Object> hm=mData.get(position + 1);
		
		if (hm == null) {
			Log.d("lua", position + " is null");
			return view;
		}

		boolean bool=mStyleCache.get(view) == null;
		if(bool)
			mStyleCache.put(view,true);
			
		Set<Map.Entry<String, Object>> sets= hm.entrySet();
		for (Map.Entry<String, Object> entry: sets) {
			try {
				String key=entry.getKey();
				Object value = entry.getValue();
				LuaObject obj=holder.getField(key);
				if (obj.isJavaObject()) {
					if(mTheme!=null && bool){
						setHelper((View)obj.getObject(),mTheme.get(key));
					}
					setHelper((View)obj.getObject(), value);
				}
			}
			catch (Exception e) {
				Log.d("lua", e.getMessage());
			}
		}

		if (updateing) {
			return view;
		}

		if (mAnimationUtil != null && convertView != null) {
			Animation anim=mAnimCache.get(convertView);
			if (anim == null) {
				try {
					anim = mAnimationUtil.call();
					mAnimCache.put(convertView, anim);
				}
				catch (Exception e) {
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

	private void setFilds(View view, LuaTable<String, Object> fields) throws LuaException {
		Set<Map.Entry<String, Object>> sets= fields.entrySet();
		for (Map.Entry<String, Object> entry2: sets) {
				String key2=entry2.getKey();
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
			setFilds(view, (LuaTable<String, Object>)value);
		}
		else if (view instanceof TextView) {
			if (value instanceof CharSequence)
				((TextView)view).setText((CharSequence)value);
			else
				((TextView)view).setText(value.toString());
		}
		else if (view instanceof ImageView) {
					if (value instanceof Bitmap)
					((ImageView)view).setImageBitmap((Bitmap)value);
				else if (value instanceof String)
					((ImageView)view).setImageBitmap(new AsyncLoader().getBitmap(mContext, (String)value));
				else if (value instanceof Drawable)
					((ImageView)view).setImageDrawable((Drawable)value);
				else if (value instanceof Number)
					((ImageView)view).setImageResource(((Number)value).intValue());
		}
		}
		catch (Exception e) {
			mContext.sendError("setHelper", e);
		}
	}


	private int javaSetter(Object obj, String methodName, Object value) throws LuaException {
		Class clazz = obj.getClass();

		String className=clazz.getName();
		Method[] methods = LuaJavaAPI.methodsMap.get(className);
		if (methods == null) {
			methods = clazz.getMethods();
			LuaJavaAPI.methodsMap.put(className, methods);
		}
		
		if (methodName.length() > 2 && methodName.substring(0, 2).equals("on") && value instanceof LuaFunction)		
			return javaSetListener(obj, methodName, methods, value);
		
		return javaSetMethod(obj, methodName, methods, value);
	}

	private int javaSetListener(Object obj, String methodName, Method[] methods, Object value) throws LuaException {
		String name="setOn" + methodName.substring(2) + "Listener";
			for (Method m:methods) {
				if (!m.getName().equals(name))
					continue;
				
				Class<?>[] tp=m.getParameterTypes();
				if (tp.length == 1 && tp[0].isInterface()) {
					L.newTable();
					L.pushObjectValue(value);
					L.setField(-2, methodName);
					try {
						Object listener = L.getLuaObject(-1).createProxy(tp[0]);
						m.invoke(obj, new Object[]{listener});
						return 1;
					}
					catch (Exception e) {
						throw new LuaException(e);
					}					
				}
			}			
		return 0;
	}

	
	private int javaSetMethod(Object obj, String methodName, Method[] methods , Object value) throws LuaException {
		String name="set" + methodName;
		Class<?> type = value.getClass();
		StringBuilder buf=new StringBuilder();
		for (Method m:methods) {
			if (!m.getName().equals(name))
				continue;

			Class<?>[] tp=m.getParameterTypes();
			if (tp.length != 1)
				continue;

			if (tp[0].isPrimitive()) {
				try {
					if (value instanceof Double || value instanceof Float) {
						m.invoke(obj, new Object[]{LuaState.convertLuaNumber(((Number)value).doubleValue(), tp[0])});
					}
					else if (value instanceof Long || value instanceof Integer) {
						m.invoke(obj, new Object[]{LuaState.convertLuaNumber(((Number)value).longValue(), tp[0])});
					}
					else if (value instanceof Boolean ){
						m.invoke(obj, new Object[]{(Boolean)value});
					}
					else{
						continue;
					}
					return 1;
				}
				catch (Exception e) {
					buf.append(e.getMessage());
					buf.append("\n");
					continue;
				}

			}

			if (!tp[0].isAssignableFrom(type))
				continue;

			try {
				m.invoke(obj, new Object[]{value});
				return 1;
			}
			catch (Exception e) {
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


	private Handler mHandler=new Handler(){
		@Override
		public void handleMessage(Message msg) {
			notifyDataSetChanged();
		}

	};


	private HashMap<String,Boolean> loaded=new HashMap<String,Boolean>();

	private class AsyncLoader extends Thread {

		private String mPath;

		private LuaContext mContext;

		public Bitmap getBitmap(LuaContext context, String path) throws IOException {
			// TODO: Implement this method
			mContext = context;
			mPath = path;
			if (!path.startsWith("http://"))
				return LuaBitmap.getBitmap(context, path);
			if (LuaBitmap.checkCache(context, path))
				return LuaBitmap.getBitmap(context, path);
			if (!loaded.containsKey(mPath)) {
				start();
				loaded.put(mPath, true);
			}			
			return Bitmap.createBitmap(0, 0, Bitmap.Config.RGB_565);
		}

		@Override
		public void run() {
			// TODO: Implement this method
			try {
				LuaBitmap.getBitmap(mContext, mPath);
				mHandler.sendEmptyMessage(0);
			}
			catch (IOException e) {
				mContext.sendError("getBitmap", e);
			}

		}

	}


}

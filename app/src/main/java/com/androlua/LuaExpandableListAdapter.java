package com.androlua;

import android.content.res.Resources;
import android.graphics.*;
import android.graphics.drawable.*;
import android.os.*;
import android.util.*;
import android.view.*;
import android.view.animation.*;
import android.widget.*;
import com.luajava.*;
import java.io.*;
import java.lang.reflect.*;
import java.util.*;

public class LuaExpandableListAdapter extends BaseExpandableListAdapter {

	private BitmapDrawable mDraw;
	private Resources mRes;
	private LuaState L;
	private LuaContext mContext;
	
	private LuaTable<Integer,LuaTable<String,Object>> mGroupData;
	private LuaTable<Integer,LuaTable<Integer,LuaTable<String,Object>>> mChildData;

	private HashMap<View,Animation> mAnimCache = new HashMap<View,Animation>();
	
	private LuaTable mGroupLayout;
	private LuaTable mChildLayout;
	
	private LuaFunction<View> loadlayout;

	private LuaFunction<?> insert;
	
	private LuaFunction<?> remove;

	private boolean updateing;

	private LuaFunction<Animation> mAnimationUtil;

	private boolean mNotifyOnChange;
	private Handler mHandler=new Handler(){
		@Override
		public void handleMessage(Message msg) {
			notifyDataSetChanged();
		}

	};
	private HashMap<String,Boolean> loaded=new HashMap<String,Boolean>();

	public LuaExpandableListAdapter(LuaContext context,  LuaTable groupLayout, LuaTable childLayout) throws LuaException {
		this(context,null,null,groupLayout,childLayout);
	}
	
	
	public LuaExpandableListAdapter(LuaContext context, LuaTable<Integer,LuaTable<String,Object>> groupData, LuaTable<Integer,LuaTable<Integer,LuaTable<String,Object>>> childData, LuaTable groupLayout, LuaTable childLayout) throws LuaException {
		mContext = context;
		L = context.getLuaState();
		mRes=mContext.getContext().getResources();

		mDraw=new BitmapDrawable(mRes, getClass().getResourceAsStream("/res/drawable/icon.png"));
		mDraw.setColorFilter(0x88ffffff, PorterDuff.Mode.SRC_ATOP);

		mGroupLayout = groupLayout;
		mChildLayout = childLayout;

		if (groupData == null)
			groupData = new LuaTable<Integer,LuaTable<String,Object>>(L);
		if (childData == null)
			childData = new LuaTable<Integer,LuaTable<Integer,LuaTable<String,Object>>>(L);
		mGroupData = groupData;
		mChildData = childData;

		loadlayout = (LuaFunction<View>)L.getLuaObject("loadlayout").getFunction();
		insert = L.getLuaObject("table").getField("insert").getFunction();
		remove = L.getLuaObject("table").getField("remove").getFunction();

		L.newTable();
		loadlayout.call(mGroupLayout, L.getLuaObject(-1) , AbsListView.class);
		loadlayout.call(mChildLayout, L.getLuaObject(-1) , AbsListView.class);
		L.pop(1);

	}

	public void setAnimationUtil(LuaFunction<Animation> animation) {
		mAnimCache.clear();
		mAnimationUtil = animation;
	}

	@Override
	public int getGroupCount() {
		// TODO: Implement this method
		return mGroupData.length();
	}

	@Override
	public int getChildrenCount(int groupPosition) {
		// TODO: Implement this method
		return mChildData.get(groupPosition + 1).length();
	}

	@Override
	public Object getGroup(int groupPosition) {
		// TODO: Implement this method
		return mGroupData.get(groupPosition + 1);
	}

	@Override
	public Object getChild(int groupPosition, int childPosition) {
		// TODO: Implement this method
		return mChildData.get(groupPosition + 1).get(childPosition + 1);
	}

	@Override
	public long getGroupId(int groupPosition) {
		// TODO: Implement this method
		return groupPosition + 1;
	}

	@Override
	public long getChildId(int groupPosition, int childPosition) {
		// TODO: Implement this method
		return childPosition + 1;
	}
	
	@Override
	public boolean hasStableIds() {
		// TODO: Implement this method
		return false;
	}

	public GroupItem getGroupItem(int groupPosition) {
		// TODO: Implement this method
		return new GroupItem(mChildData.get(groupPosition + 1));
	}
	
	public LuaTable<Integer,LuaTable<String,Object>> getGroupData(){
		return mGroupData;
	}

	public LuaTable<Integer,LuaTable<Integer,LuaTable<String,Object>>> getChildData(){
		return mChildData;
	}
	
	public GroupItem add(LuaTable<String,Object> groupItem) throws Exception {
		mGroupData.put(mGroupData.length() + 1, groupItem);
		LuaTable<Integer, LuaTable<String, Object>> childItem=new LuaTable<Integer,LuaTable<String,Object>>(L);
		mChildData.put(mGroupData.length(),childItem);
		if (mNotifyOnChange) notifyDataSetChanged();
		return new GroupItem(childItem);
	}

	public GroupItem add(LuaTable<String,Object> groupItem, LuaTable<Integer, LuaTable<String, Object>> childItem) throws Exception {
		mGroupData.put(mGroupData.length() + 1, groupItem);
		mChildData.put(mGroupData.length(),childItem);
		if (mNotifyOnChange) notifyDataSetChanged();
		return new GroupItem(childItem);
	}

	public GroupItem insert(int position, LuaTable<String,Object> groupItem, LuaTable<Integer, LuaTable<String, Object>> childItem) throws Exception {
		insert.call(mGroupData, position + 1, groupItem);
		insert.call(mChildData, position + 1, childItem);
		if (mNotifyOnChange) notifyDataSetChanged();
		return new GroupItem(childItem);
	}

	public void remove(int idx) throws Exception {
		remove.call(mGroupData, idx + 1);
		if (mNotifyOnChange) notifyDataSetChanged();
	}
	
	public void clear() {
		mGroupData.clear();
		mChildData.clear();
		if (mNotifyOnChange) notifyDataSetChanged();
	}

	public void setNotifyOnChange(boolean notifyOnChange) {
        mNotifyOnChange = notifyOnChange;
		if (mNotifyOnChange) notifyDataSetChanged();
	}

	@Override
	public View getGroupView(int groupPosition, boolean isExpanded, View convertView, ViewGroup parent) {
		// TODO: Implement this method
		View view = null;
		LuaTable<String,View> holder = null;
		if (convertView == null) {
			try {
				holder = new LuaTable<String,View>(L);
				view = loadlayout.call(mGroupLayout, holder, AbsListView.class);
				view.setTag(holder);
			}
			catch (LuaException e) {
				return new View(mContext.getContext());
			}
		}
		else {
			view = convertView;
			holder = (LuaTable<String, View>) view.getTag();
		}

		LuaTable<String, Object> hm=mGroupData.get(groupPosition + 1);

		if (hm == null) {
			Log.i("lua", groupPosition + " is null");
			return view;
		}


		Set<Map.Entry<String, Object>> sets= hm.entrySet();
		for (Map.Entry<String, Object> entry: sets) {
			try {
				String key=entry.getKey();
				Object value = entry.getValue();
				View obj=holder.get(key);
				if (obj!=null) {
					setHelper(obj, value);
				}
			}
			catch (Exception e) {
				Log.i("lua", e.getMessage());
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
					Log.i("lua", e.getMessage());
				}
			}
			if (anim != null) {
				view.clearAnimation();
				view.startAnimation(anim);
			}
		}
		return view;

	}

	@Override
	public View getChildView(int groupPosition, int childPosition, boolean isLastChild, View convertView, ViewGroup parent) {
		// TODO: Implement this method
		View view = null;
		LuaTable<String,View> holder = null;
		if (convertView == null) {
			try {
				holder = new LuaTable<String,View>(L);
				view = loadlayout.call(mChildLayout, holder, AbsListView.class);
				view.setTag(holder);
			}
			catch (LuaException e) {
				return new View(mContext.getContext());
			}
		}
		else {
			view = convertView;
			holder = (LuaTable<String, View>) view.getTag();
		}

		LuaTable<String, Object> hm=mChildData.get(groupPosition + 1).get(childPosition + 1);

		if (hm == null) {
			Log.i("lua", childPosition + " is null");
			return view;
		}

		Set<Map.Entry<String, Object>> sets= hm.entrySet();
		for (Map.Entry<String, Object> entry: sets) {
			try {
				String key=entry.getKey();
				Object value = entry.getValue();
				View obj=holder.get(key);
				if (obj != null) {
					setHelper(obj, value);
				}
			}
			catch (Exception e) {
				Log.i("lua", e.getMessage());
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
					Log.i("lua", e.getMessage());
				}
			}
			if (anim != null) {
				view.clearAnimation();
				view.startAnimation(anim);
			}
		}
		return view;
	}

	@Override
	public boolean isChildSelectable(int groupPosition, int childPosition) {
		// TODO: Implement this method
		return false;
	}

	private void setFields(View view, LuaTable<String, Object> fields) {
		Set<Map.Entry<String, Object>> sets= fields.entrySet();
		for (Map.Entry<String, Object> entry2: sets) {
			try {
				String key2=entry2.getKey();
				Object value2 = entry2.getValue();
				if (key2.toLowerCase().equals("src"))
					setHelper(view, value2);
				else
					javaSetter(view, key2, value2);
			}
			catch (Exception e2) {
				Log.i("lua", e2.getMessage());
			}
		}
	}

	private void setHelper(View view, Object value) {
		if (value instanceof LuaTable) {
			setFields(view, (LuaTable<String, Object>)value);
		}
		else if (view instanceof TextView) {
			if (value instanceof CharSequence)
				((TextView)view).setText((CharSequence)value);
			else
				((TextView)view).setText(value.toString());
		}
		else if (view instanceof ImageView) {
			try {
				if (value instanceof Bitmap)
					((ImageView)view).setImageBitmap((Bitmap)value);
				else if (value instanceof String)
					((ImageView)view).setImageDrawable(new AsyncLoader().getBitmap(mContext, (String)value));
				else if (value instanceof Drawable)
					((ImageView)view).setImageDrawable((Drawable)value);
				else if (value instanceof Number)
					((ImageView)view).setImageResource(((Number)value).intValue());
			}
			catch (Exception e) {
				Log.i("lua", e.getMessage());
			}

		}
	}

	private int javaSetter(Object obj, String methodName, Object value) throws LuaException {

		if (methodName.length() > 2 && methodName.substring(0, 2).equals("on") && value instanceof LuaFunction)
			return javaSetListener(obj, methodName, value);

		return javaSetMethod(obj, methodName, value);
	}

	private int javaSetListener(Object obj, String methodName, Object value) throws LuaException {
		String name="setOn" + methodName.substring(2) + "Listener";
		ArrayList<Method> methods = LuaJavaAPI.getMethod(obj.getClass(), name, false);
		for (Method m:methods) {

			Class<?>[] tp=m.getParameterTypes();
			if (tp.length == 1 && tp[0].isInterface()) {
				L.newTable();
				L.pushObjectValue(value);
				L.setField(-2, methodName);
				try {
					Object listener = L.getLuaObject(-1).createProxy(tp[0]);
					m.invoke(obj, listener);
					return 1;
				}
				catch (Exception e) {
					throw new LuaException(e);
				}
			}
		}
		return 0;
	}

	private int javaSetMethod(Object obj, String methodName, Object value) throws LuaException {
		if(Character.isLowerCase(methodName.charAt(0))){
			methodName=Character.toUpperCase(methodName.charAt(0))+methodName.substring(1);
		}
		String name="set" + methodName;
		Class<?> type = value.getClass();
		StringBuilder buf=new StringBuilder();

		ArrayList<Method> methods = LuaJavaAPI.getMethod(obj.getClass(), name, false);

		for (Method m:methods) {
			Class<?>[] tp=m.getParameterTypes();
			if (tp.length != 1)
				continue;

			if (tp[0].isPrimitive()) {
				try {
					if (value instanceof Double || value instanceof Float) {
						m.invoke(obj, LuaState.convertLuaNumber(((Number)value).doubleValue(), tp[0]));
					}
					else if (value instanceof Long || value instanceof Integer) {
						m.invoke(obj, LuaState.convertLuaNumber(((Number)value).longValue(), tp[0]));
					}
					else if (value instanceof Boolean ){
						m.invoke(obj, (Boolean)value);
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
				m.invoke(obj, value);
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

	private class GroupItem {
		private LuaTable<Integer,LuaTable<String,Object>> mData;
		public GroupItem(LuaTable<Integer,LuaTable<String,Object>> item){
			mData=item;
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

			return mDraw;
		}

		@Override
		public void run() {
			// TODO: Implement this method
			try {
				LuaBitmap.getBitmap(mContext, mPath);
				mHandler.sendEmptyMessage(0);
			}
			catch (IOException e) {
				mContext.sendError("AsyncLoader",e);
			}

		}

	}
	
}

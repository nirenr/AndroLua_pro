package com.androlua;

import android.graphics.*;
import android.graphics.drawable.*;
import android.os.*;
import android.util.*;
import android.view.*;
import android.widget.*;
import com.luajava.*;
import java.io.*;
import java.util.*;
import android.view.animation.*;

public class LuaArrayAdapter extends ArrayListAdapter {

	private LuaContext mContext;

	private LuaState L;

	private LuaObject mResource;

	private LuaObject loadlayout;

	private Animation mAnimation;


	public LuaArrayAdapter(LuaContext context, LuaObject resource) throws LuaException {
		this(context, resource, new String[0]);
	}

	public LuaArrayAdapter(LuaContext context, LuaObject resource, Object[] objects) throws LuaException {
		super(context.getContext(), 0, objects);
		mContext = context;
		mResource = resource;
		L = context.getLuaState();
		loadlayout = L.getLuaObject("loadlayout");
		L.newTable();
		loadlayout.call(mResource, L.getLuaObject(-1) , AbsListView.class);
		L.pop(1);
	}

	@Override
	public View getDropDownView(int position, View convertView, ViewGroup parent) {
		// TODO: Implement this method
		return getView(position, convertView, parent);
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		// TODO: Implement this method
		View view = null;
		LuaObject holder = null;
		if (convertView == null) {
			L.newTable();
			holder = L.getLuaObject(-1);
			L.pop(1);
			try {
				view = (View)loadlayout.call(mResource, holder, AbsListView.class);
			}
			catch (LuaException e) {
				return new View(mContext.getContext());
			}
		}
		else {
			view = convertView;
		}
		setHelper(view, getItem(position));
		if (mAnimation != null)
			view.startAnimation(mAnimation);
		return view;
	}

	public void setAnimation(Animation animation) {
		this.mAnimation = animation;
	}

	public Animation getAnimation() {
		return mAnimation;
	}


	private void setHelper(View view, Object value) {
		if (view instanceof TextView){
			if (value instanceof CharSequence)
				((TextView)view).setText((CharSequence)value);
			else
				((TextView)view).setText(value.toString());
		}
		else if (view instanceof ImageView) {
			try {
				ImageView img=(ImageView) view;
				Drawable drawable=null;
				if (value instanceof Bitmap)
					drawable = new BitmapDrawable((Bitmap)value);
				else if (value instanceof String)
					drawable = new BitmapDrawable(new AsyncLoader().getBitmap(mContext, (String)value));
				else if (value instanceof Drawable)
					drawable = (Drawable)value;
				else if (value instanceof Number)
					drawable = mContext.getContext().getDrawable(((Number)value).intValue());

				img.setImageDrawable(drawable);
				if (drawable instanceof BitmapDrawable) {
					Bitmap bmp=((BitmapDrawable)drawable).getBitmap();
					int w=bmp.getWidth();
					int h = bmp.getHeight();
					if (w > 200) {
						h = (int)(mContext.getHeight() * ((float)mContext.getWidth()) / ((float)w));
						w = mContext.getWidth();
						img.setLayoutParams(new ViewGroup.LayoutParams(w, h));
					}
				}
			}
			catch (Exception e) {
				Log.d("lua", e.getMessage());
			}

		}
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
				mContext.sendError("AsyncLoader",e);
			}

		}

	}

}

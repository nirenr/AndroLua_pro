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

public class LuaArrayAdapter extends ArrayListAdapter
{

	private LuaActivity mContext;

	private LuaState L;

	private LuaObject mResource;

	private LuaObject loadlayout;

	private Animation mAnimation;

	
	public LuaArrayAdapter(LuaActivity context,LuaObject resource) throws LuaException
	{
		this(context,resource,new String[0]);
	}
	
	public LuaArrayAdapter(LuaActivity context,LuaObject resource,String[] objects) throws LuaException
	{
		super(context,0,objects);
		mContext=context;
		mResource = resource;
		L = context.getLuaState();
		loadlayout = L.getLuaObject("loadlayout");
		L.newTable();
		loadlayout.call(mResource, L.getLuaObject(-1) , AbsListView.class);
		L.pop(1);
	}

	@Override
	public View getDropDownView(int position, View convertView, ViewGroup parent)
	{
		// TODO: Implement this method
		return getView(position, convertView, parent);
	}
	
	@Override
	public View getView(int position, View convertView, ViewGroup parent)
	{
		// TODO: Implement this method
		View view = null;
		LuaObject holder = null;
		if (convertView==null)
		{
			L.newTable();
			holder = L.getLuaObject(-1);
			L.pop(1);
			try
			{
				view = (View)loadlayout.call(mResource, holder, AbsListView.class);
			}
			catch (LuaException e)
			{
				return new View(mContext);
			}
		}
		else
		{
			view=convertView;
		}
		setHelper(view,getItem(position));
		if(mAnimation!=null)
			view.startAnimation(mAnimation);
		return view;
	}
	
	public void setAnimation(Animation animation)
	{
		this.mAnimation = animation;
	}

	public Animation getAnimation()
	{
		return mAnimation;
	}
	
	
	private void setHelper(View view, Object value)
	{
		if (view instanceof TextView)
			((TextView)view).setText(value.toString());
		else if (view instanceof ImageView)
		{
			try
			{
				ImageView img=(ImageView) view;
				Drawable drawable=null;
				if (value instanceof Bitmap)
					drawable=new BitmapDrawable((Bitmap)value);
				else if (value instanceof String)
					drawable=new BitmapDrawable(new AsyncLoader().getBitmap(mContext, (String)value));
				else if (value instanceof Drawable)
					drawable=(Drawable)value;
				else if (value instanceof Number)
					drawable=mContext.getDrawable(0);
				
				img.setImageDrawable(drawable);
			}
			catch (Exception e)
			{
				Log.d("lua",e.getMessage());
			}

		}
	}
	
	
	private class AsyncLoader extends AsyncTask
	{
		public Bitmap getBitmap(LuaActivity mContext, String path) throws IOException
		{
			// TODO: Implement this method
			if(path.indexOf("http://")!=0)
				return LuaBitmap.getBitmap(mContext,path);
			if(LuaBitmap.checkCache(mContext,path))
				return LuaBitmap.getBitmap(mContext,path);
			execute(mContext,path);
			return Bitmap.createBitmap(0,0,Bitmap.Config.RGB_565);
		}

		@Override
		protected Object doInBackground(Object[] p1)
		{
			// TODO: Implement this method
			try
			{
				LuaBitmap.getBitmap((LuaContext)p1[0], (String)p1[1]);
				return true;
			}
			catch (IOException e)
			{
				return null;
			}

		}

		@Override
		protected void onPostExecute(Object result)
		{
			// TODO: Implement this method
			super.onPostExecute(result);
			if(result!=null)
				notifyDataSetChanged();
		}
	}
	
}

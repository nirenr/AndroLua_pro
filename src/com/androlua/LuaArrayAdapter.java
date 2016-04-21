package com.androlua;

import android.widget.*;
import android.content.*;
import android.view.*;
import com.luajava.*;
import android.util.*;
import android.graphics.*;
import android.graphics.drawable.*;
import java.util.*;

public class LuaArrayAdapter extends ArrayListAdapter
{

	private LuaActivity mContext;

	private LuaState L;

	private LuaObject mResource;

	private LuaObject loadlayout;

	private String[] mObjects;
	
	
	public LuaArrayAdapter(LuaActivity context,LuaObject resource,String[] objects) throws LuaException
	{
		super(context,0,objects);
		mObjects=objects;
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
		return view;
	}
	
	private void setHelper(View view, Object value)
	{
		if (view instanceof TextView)
			((TextView)view).setText(value.toString());
		else if (view instanceof ImageView)
		{
			try
			{
				if (value instanceof Bitmap)
					((ImageView)view).setImageBitmap((Bitmap)value);
				else if (value instanceof String)
					((ImageView)view).setImageBitmap(LuaBitmap.getBitmap(mContext, (String)value));
				else if (value instanceof Drawable)
					((ImageView)view).setImageDrawable((Drawable)value);
				else if (value instanceof Number)
					((ImageView)view).setImageResource(((Number)value).intValue());
			}
			catch (Exception e)
			{
				Log.d("lua",e.getMessage());
			}

		}
	}
	
}

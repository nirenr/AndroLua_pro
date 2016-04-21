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

public class LuaAdapter extends BaseAdapter
{

	private LuaActivity mContext;

	private LuaObject mResource;

	private LuaState L;

	private LuaObject loadlayout;

	private ArrayList<? extends Map<String, ?>> mData;

	private String[] mFrom;

	private String[] mField;

	private String[] mTo;

	private boolean mNotifyOnChange=true;

	@Override
	public int getCount()
	{
		// TODO: Implement this method
		return mData.size();
	}

	@Override
	public Object getItem(int p1)
	{
		// TODO: Implement this method
		return mData.get(p1);
	}

	@Override
	public long getItemId(int id)
	{
		// TODO: Implement this method
		return id;
	}

	public void add(HashMap<java.lang.String, ?> map) throws Exception
	{
		if (mData instanceof ArrayList)
			((ArrayList<Map<String, ?>>)mData).add(map);
		else 
			throw new Exception("Con not add items");
		if (mNotifyOnChange) notifyDataSetChanged();
	}

	public void insert(int idx,HashMap<java.lang.String, ?> map) throws Exception
	{
		if (mData instanceof ArrayList)
			((ArrayList<Map<String, ?>>)mData).add(idx,map);
		else 
			throw new Exception("Con not add items");
		if (mNotifyOnChange) notifyDataSetChanged();
	}
	
	public void remove(int idx) throws Exception
	{
		if (mData instanceof ArrayList)
			((ArrayList<Map<String, ?>>)mData).remove(idx);
		else 
			throw new Exception("Con not add items");
		if (mNotifyOnChange) notifyDataSetChanged();
	}
	
	public void clear()
	{
		mData.clear();
		if (mNotifyOnChange) notifyDataSetChanged();
	}
	
	public void setNotifyOnChange(boolean notifyOnChange) {
        mNotifyOnChange = notifyOnChange;
		if (mNotifyOnChange) notifyDataSetChanged();
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
		if (convertView == null)
		{
			try
			{
				L.newTable();
				holder = L.getLuaObject(-1);
				L.pop(1);
				view = (View)loadlayout.call(mResource, holder, AbsListView.class);
				view.setTag(holder);
			}
			catch (LuaException e)
			{
				return new View(mContext);
			}
		}
		else
		{
			view = convertView;
			holder = (LuaObject)view.getTag();
		}
		if (mField != null)
		{

			for (int i=0;i < mFrom.length;i++)
			{
				try
				{
					String[] to = null;
					if (mTo != null)
						to = mTo;
					else
						to = mFrom;

					LuaObject obj=holder.getField(to[i]);
					if (obj.isJavaObject())
						obj.setField(mField[i], mData.get(position).get(mFrom[i]));
				}
				catch (LuaException e)
				{
					Log.d("lua","",e);
				}
			}
		}
		else
		{
			Map hm=mData.get(position);
			//Set<Map.Entry> sets =hm.entrySet(); 
			for (Map.Entry entry : hm.entrySet())
			{ 

				try
				{
					String key=(String)entry.getKey();
					Object value = entry.getValue();
					LuaObject obj=holder.getField(key);
					if (obj.isJavaObject())
						setHelper((View)obj.getObject(), value);

				}
				catch (Exception e)
				{
					Log.d("lua",e.getMessage());
				}
			}

		}
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

	public LuaAdapter(LuaActivity context, LuaObject resource) throws LuaException
	{
		mContext = context;
		mData = new ArrayList<Map<String, ?>>();
		mResource = resource;
		L = context.getLuaState();
		loadlayout = L.getLuaObject("loadlayout");
		L.newTable();
		loadlayout.call(mResource, L.getLuaObject(-1) , AbsListView.class);
		L.pop(1);
	}


	public LuaAdapter(LuaActivity context, java.util.List<? extends java.util.Map<java.lang.String, ?>> data, LuaObject resource) throws LuaException
	{
		mContext = context;
		mData = new ArrayList<Map<String, ?>>(data);
		mResource = resource;
		L = context.getLuaState();
		loadlayout = L.getLuaObject("loadlayout");
		L.newTable();
		loadlayout.call(mResource, L.getLuaObject(-1) , AbsListView.class);
		L.pop(1);
	}

	public LuaAdapter(LuaActivity context, java.util.List<? extends java.util.Map<java.lang.String, ?>> data, LuaObject resource, java.lang.String[] from, String[] field) throws LuaException
	{
		mContext = context;
		mData = new ArrayList<Map<String, ?>>(data);
		mFrom = from;
		mField = field;
		mResource = resource;
		L = context.getLuaState();
		loadlayout = L.getLuaObject("loadlayout");
		L.newTable();
		loadlayout.call(mResource, L.getLuaObject(-1) , AbsListView.class);
		L.pop(1);
	}

	public LuaAdapter(LuaActivity context, java.util.List<? extends java.util.Map<java.lang.String, ?>> data, LuaObject resource, java.lang.String[] from, String[] to, String[] field) throws LuaException
	{
		mContext = context;
		mData = new ArrayList<Map<String, ?>>(data);
		mFrom = from;
		mField = field;
		mTo = to;
		mResource = resource;
		L = context.getLuaState();
		loadlayout = L.getLuaObject("loadlayout");
		L.newTable();
		loadlayout.call(mResource, L.getLuaObject(-1) , AbsListView.class);
		L.pop(1);
	}


	public class AsyncImageLoader
	{

		private HashMap<String, SoftReference<Drawable>> imageCache;

		public AsyncImageLoader()
		{
			imageCache = new HashMap<String, SoftReference<Drawable>>();
		}

		public Drawable loadDrawable(final String imageUrl, final ImageCallback imageCallback)
		{
			if (imageCache.containsKey(imageUrl))
			{
				SoftReference<Drawable> softReference = imageCache.get(imageUrl);
				Drawable drawable = softReference.get();
				if (drawable != null)
				{
					return drawable;
				}
			}
			final Handler handler = new Handler() {
				@Override
				public void handleMessage(Message message)
				{
					imageCallback.imageLoaded((Drawable) message.obj, imageUrl);
				}
			};
			new Thread() {
				@Override
				public void run()
				{
					try
					{
						Drawable drawable = new BitmapDrawable(LuaBitmap.getHttpBitmap(imageUrl));
						imageCache.put(imageUrl, new SoftReference<Drawable>(drawable));
						Message message = handler.obtainMessage(0, drawable);
						handler.sendMessage(message);
					}
					catch (IOException e)
					{}
				}
			}.start();
			return null;
		}

	}
	public interface ImageCallback
	{
        public void imageLoaded(Drawable imageDrawable, String imageUrl);
    }
}

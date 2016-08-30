package com.androlua;

import android.app.*;
import android.view.*;
import android.os.*;
import com.luajava.*;

public class LuaFragment extends Fragment
{

	private LuaTable mLayout;

	private LuaObject mLoadLayout;
	
	public LuaFragment(LuaContext context,LuaTable layout){
		mLoadLayout=context.getLuaState().getLuaObject("loadlayout");
		mLayout=layout;
	}

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState)
	{
		// TODO: Implement this method
		//return super.onCreateView(inflater, container, savedInstanceState);
		try {
			return (View)mLoadLayout.call(mLayout);
		}
		catch (LuaException e) {
			throw new IllegalArgumentException(e.getMessage());
		}
	}
}

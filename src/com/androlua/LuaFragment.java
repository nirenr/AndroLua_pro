package com.androlua;

import android.app.*;
import android.view.*;
import android.os.*;
import com.luajava.*;

public class LuaFragment extends Fragment
{

	private LuaTable mLayout=null;

	private LuaObject mLoadLayout=null;
	public LuaFragment(){

	}
/*
	public LuaFragment(LuaTable layout){
		mLoadLayout=layout.getLuaState().getLuaObject("loadlayout");
		mLayout=layout;
	}*/
	public void setLayout(LuaTable layout){
		mLayout=layout;
	}
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState)
	{
		try {
			return (View)mLoadLayout.call(mLayout);
		}
		catch (LuaException e) {
			throw new IllegalArgumentException(e.getMessage());
		}
	}
}

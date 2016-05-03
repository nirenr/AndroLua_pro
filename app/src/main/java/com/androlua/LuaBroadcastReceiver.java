package com.androlua;

import android.content.*;
import com.androlua.LuaBroadcastReceiver.*;

public class LuaBroadcastReceiver extends BroadcastReceiver
{

	private LuaBroadcastReceiver.OnReceiveListerer mRlt;
	
	public LuaBroadcastReceiver(OnReceiveListerer rlt)
	{
		mRlt=rlt;
	}

	@Override
	public void onReceive(Context context, Intent intent)
	{
		// TODO: Implement this method
		mRlt.onReceive(context,intent);
	}
	
	public interface OnReceiveListerer
	{
		public abstract void onReceive(android.content.Context context, android.content.Intent intent);
	}
}

package com.androlua;

import android.os.*;

public class Ticker
{
	private Handler mHandler;

	private Ticker.OnTickListener mOnTickListener;

	private Thread mThread;

	private long mPeriod=1000;

	private boolean mEnabled=true;

	private boolean isRun=false;

	private long mLast;

	private long mOffset;


	public Ticker()
	{
		init();
	}

	private void init()
	{
		mHandler = new Handler()
		{
			public void handleMessage(Message msg)
			{
				if (mOnTickListener != null)
					mOnTickListener.onTick();
			}
		};
		mThread = new Thread(){
			@Override
			public void run()
			{
				isRun = true;
				while (isRun)
				{
					long now = System.currentTimeMillis();
					if (!mEnabled)
						mLast = now - mOffset;
					if (now - mLast >= mPeriod)
					{
						mLast = now;
						mHandler.sendEmptyMessage(0);
					}

					try
					{
						sleep(1);
					}
					catch (InterruptedException e)
					{
					}
				}
			}
		};
	}

	public void setPeriod(long period)
	{
		mLast=System.currentTimeMillis();
		mPeriod = period;
	}

	public long getPeriod()
	{
		return mPeriod;
	}
	
	
	public void setInterval(long period)
	{
		mLast=System.currentTimeMillis();
		mPeriod = period;
	}

	public long getInterval()
	{
		return mPeriod;
	}

	public void setEnabled(boolean enabled)
	{
		mEnabled = enabled;
		if (!enabled)
			mOffset = System.currentTimeMillis() - mLast;
	}

	public boolean getEnabled()
	{
		return mEnabled;
	}

	public void setOnTickListener(OnTickListener ltr)
	{
		mOnTickListener = ltr;
	}

	public void start()
	{
		mThread.start();
	}

	public void stop()
	{
		isRun = false;
	}

	public boolean isRun()
	{
		return isRun;
	}


	public interface OnTickListener
	{
		public void onTick();
	}
}

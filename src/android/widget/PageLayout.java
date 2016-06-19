package android.widget;

import android.content.Context;
import android.util.AttributeSet;
import android.util.TypedValue;
import android.view.MotionEvent;
import android.view.ViewGroup;
import android.widget.HorizontalScrollView;
import android.widget.LinearLayout;
import android.view.*;
import android.view.ViewGroup.*;
import android.widget.PageLayout.*;


public class PageLayout extends HorizontalScrollView
{
	/**
	 * 屏幕宽度
	 */
	private int mScreenWidth;
	
	private int mTouchScale = 0;
	
	private boolean once;

	private LinearLayout wrapper;

	private PageLayout.OnPageChangeListener mOnPageChangeListener;

	private int mIdx;

	public PageLayout(Context context)
	{
		super(context);
		init(context);
	}

	public PageLayout(Context context, AttributeSet attrs)
	{
		super(context, attrs);
		init(context);
	}

	public void setTouchScale(int touchScale)
	{
		this.mTouchScale = touchScale;
	}

	public int getTouchScale()
	{
		return mTouchScale;
	}
	
	private void init(Context context)
	{
		setHorizontalScrollBarEnabled(false);
		mScreenWidth = context.getResources().getDisplayMetrics().widthPixels;
		mTouchScale = mScreenWidth/10;
		wrapper = new LinearLayout(context);
		super.addView(wrapper);
	}

	@Override
	protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec)
	{
		/**
		 * 显示的设置一个宽度
		 */
		if (!once)
		{
			int n=wrapper.getChildCount();
			for (int i=0;i < n;i++)
				wrapper.getChildAt(i).getLayoutParams().width = mScreenWidth;
		}
		super.onMeasure(widthMeasureSpec, heightMeasureSpec);

	}

	@Override
	protected void onLayout(boolean changed, int l, int t, int r, int b)
	{
		super.onLayout(changed, l, t, r, b);
		if (changed)
		{
			if(!once)
				scrollTo(0,0);
			once = true;
		}
	}

	@Override
	public void addView(View child, ViewGroup.LayoutParams params)
	{
		// TODO: Implement this method
		wrapper.addView(child, params);
	}

	@Override
	public void addView(View child)
	{
		// TODO: Implement this method
		wrapper.addView(child);
	}

	@Override
	public boolean onInterceptTouchEvent(MotionEvent ev)
	{
		if (ev.getX() < mTouchScale || ev.getX() > mScreenWidth - mTouchScale)
		{
			return super.onInterceptTouchEvent(ev);
		}
		else
		{
			return false;
		}
	}


	@Override
	public boolean onTouchEvent(MotionEvent ev)
	{
		int action = ev.getAction();
		switch (action)
		{
				// Up时，进行判断，如果显示区域大于页面宽度一半则完全显示
			case MotionEvent.ACTION_UP:
				int scrollX = getScrollX();
				int x=scrollX % mScreenWidth;
				if (x < mScreenWidth / 2)
					showPage(scrollX / mScreenWidth);
				else
					showPage(scrollX / mScreenWidth+1);
				return true;
		}
		return super.onTouchEvent(ev);
	}

	public void showPage(int idx)
	{
		//wrapper.getChildAt(idx);
		smoothScrollTo(mScreenWidth*idx,0);
		if(mOnPageChangeListener!=null && mIdx!=idx)
			mOnPageChangeListener.onPageChange(this,idx);
		mIdx=idx;
	}
	
	public void showPage(View v)
	{
		int n=wrapper.getChildCount();
		for (int i=0;i < n;i++)
			if(wrapper.getChildAt(i).equals(v))
				showPage(i);
	}
	
	public View getPage(int idx)
	{
		return wrapper.getChildAt(idx);
	}
	
	public void setOnPageChangeListener(OnPageChangeListener ltr)
	{
		mOnPageChangeListener=ltr;
	}
	
	
	public static interface OnPageChangeListener
	{
		public void onPageChange(View v, int idx);
	}
}

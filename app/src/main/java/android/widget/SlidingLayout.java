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


public class SlidingLayout extends HorizontalScrollView
{
	/**
	 * 屏幕宽度
	 */
	private int mScreenWidth;
	/**
	 * 可以打开菜单的滑动范围
	 */
	private int mTouchScale = 0;
	/**
	 * 菜单的宽度
	 */
	private int mMenuWidth = 0;
	private int mHalfMenuWidth = 0;

	private boolean once;

	private boolean isOpen;

	private LinearLayout wrapper;

	private OnMenuOpenedListener mOnMenuOpenedListener;

	private OnMenuClosedListener mOnMenuClosedListener;

	private OnMenuStateChangeListener mOnMenuStateChangeListener;

	private boolean isSlidinged;

	public SlidingLayout(Context context)
	{
		super(context);
		init(context);
	}

	public SlidingLayout(Context context, AttributeSet attrs)
	{
		super(context, attrs);
		init(context);
	}

	public void setOnMenuStateChangeListener(OnMenuStateChangeListener mOnMenuStateChangeListener)
	{
		this.mOnMenuStateChangeListener = mOnMenuStateChangeListener;
	}

	public void setOnMenuClosedListener(OnMenuClosedListener onMenuClosedListener)
	{
		mOnMenuClosedListener = onMenuClosedListener;
	}

	public void setOnMenuOpenedListener(OnMenuOpenedListener onMenuOpenedListener)
	{
		mOnMenuOpenedListener = onMenuOpenedListener;
	}

	public void setTouchScale(int touchScale)
	{
		this.mTouchScale = touchScale;
	}

	public int getTouchScale()
	{
		return mTouchScale;
	}

	public void setMenuWidth(int menuWidth)
	{
		this.mMenuWidth = menuWidth;
	}

	public int getMenuWidth()
	{
		return mMenuWidth;
	}

	private void init(Context context)
	{
		setHorizontalScrollBarEnabled(false);
		mScreenWidth = context.getResources().getDisplayMetrics().widthPixels;

		//默认左侧滑动范围为屏幕宽度的10%
		mTouchScale = mScreenWidth / 10;
		wrapper = new LinearLayout(context);
		super.addView(wrapper);
	}

	@Override
	protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec)
	{
		/**
		 * 设置菜单宽度
		 */
		if (!once)
		{
			View menu = wrapper.getChildAt(0);
			View content = wrapper.getChildAt(1);

			ViewGroup.LayoutParams lp=menu.getLayoutParams();

			//如果没有指定菜单宽度，盲人使用屏幕宽度的80%
			if (mMenuWidth == 0 && lp.width < 0)
				lp.width = (int)(mScreenWidth * 0.8);
			mMenuWidth = lp.width;
			mHalfMenuWidth = lp.width / 2;
			content.getLayoutParams().width = mScreenWidth;
			wrapper.getLayoutParams().width=mScreenWidth+mMenuWidth;
		}
		if(isOpen)
			openMenu();
		else
			closeMenu();
		super.onMeasure(widthMeasureSpec, heightMeasureSpec);

	}

	@Override
	protected void onLayout(boolean changed, int l, int t, int r, int b)
	{
		super.onLayout(changed, l, t, r, b);
		if (changed)
		{
			// 将菜单隐藏
			if(!once)
				this.scrollTo(mMenuWidth, 0);
			else
				closeMenu();
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
		//菜单显示或者触摸在范围则拦截触摸操作
		if (isOpen || ev.getX() < mTouchScale)
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
			case MotionEvent.ACTION_MOVE:
				if (!isSlidinged && mOnMenuStateChangeListener != null)
					mOnMenuStateChangeListener.onMenuStateChange(this, isOpen);
				isSlidinged = true;
				break;
				// Up时，进行判断，如果显示区域大于菜单宽度一半则完全显示，否则隐藏
			case MotionEvent.ACTION_UP:
				int scrollX = getScrollX();
				if (isOpen)
				{
					if (scrollX > mHalfMenuWidth / 2)
						closeMenu();
					else
						openMenu();
				}
				else
				{
					if (scrollX > mHalfMenuWidth * 1.5)
						closeMenu();
					else
						openMenu();
				}
				isSlidinged = false;
				return true;
		}
		return super.onTouchEvent(ev);
	}

	/**
	 * 打开菜单
	 */
	public void openMenu()
	{
		this.smoothScrollTo(0, 0);
		if (!isOpen && mOnMenuOpenedListener != null)
			mOnMenuOpenedListener.onMenuOpened(this);
		isOpen = true;
	}

	/**
	 * 关闭菜单
	 */
	public void closeMenu()
	{
		this.smoothScrollTo(mMenuWidth, 0);
		if (isOpen && mOnMenuClosedListener != null)
			mOnMenuClosedListener.onMenuClosed(this);
		isOpen = false;
	}

	/**
	 * 切换菜单状态
	 */
	public void toggle()
	{
		if (isOpen)
		{
			closeMenu();
		}
		else
		{
			openMenu();
		}
	}

	public static interface OnMenuOpenedListener
	{
		public void onMenuOpened(View v);
	}

	public static interface OnMenuClosedListener
	{
		public void onMenuClosed(View v);
	}

	public static interface OnMenuStateChangeListener
	{
		public void onMenuStateChange(View v, boolean isOpen);
	}
}

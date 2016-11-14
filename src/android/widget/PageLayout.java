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
import android.util.*;


public class PageLayout extends HorizontalScrollView {

	private int mTouchScale = 0;

	private LinearLayout wrapper;

	private PageLayout.OnPageChangeListener mOnPageChangeListener;

	private int mIdx;

	private Scroller mScroller;
	private int mTouchSlop;
	private int mMinimumVelocity;
	private int mMaximumVelocity;
	private VelocityTracker mVelocityTracker;

	private int mCount;
	private int mWidth;


	public PageLayout(Context context) {
		super(context);
		init(context);
	}

	public PageLayout(Context context, AttributeSet attrs) {
		super(context, attrs);
		init(context);
	}

	public void setTouchScale(int touchScale) {
		this.mTouchScale = touchScale;
	}

	public int getTouchScale() {
		return mTouchScale;
	}

	private void init(Context context) {
		setHorizontalScrollBarEnabled(false);
		mWidth = context.getResources().getDisplayMetrics().widthPixels;
		mTouchScale = mWidth / 2;
		wrapper = new LinearLayout(context);
		super.addView(wrapper);

		mScroller = new Scroller(getContext());
		setFocusable(true);
		setWillNotDraw(false);
		final ViewConfiguration configuration = ViewConfiguration.get(context);
		mTouchSlop = configuration.getScaledTouchSlop();
		mMinimumVelocity = configuration.getScaledMinimumFlingVelocity();
		mMaximumVelocity = configuration.getScaledMaximumFlingVelocity();

	}

	@Override
	protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
		int w=getMeasuredWidth();
		int count=wrapper.getChildCount();
		if (mCount != count || mWidth != w) {
			mCount = count;
			mWidth = w;
			for (int i=0;i < count;i++) {
				ViewGroup chid=(ViewGroup) wrapper.getChildAt(i);
				ViewGroup.LayoutParams lp=chid.getLayoutParams();
				lp.width = mWidth;
				chid.setLayoutParams(lp);
				chid.requestLayout();
			}
			ViewGroup.LayoutParams lp=wrapper.getLayoutParams();
			lp.width = mWidth * count;
			wrapper.setLayoutParams(lp);
			wrapper.requestLayout();
			requestLayout();
		}
		super.onMeasure(widthMeasureSpec, heightMeasureSpec);
		showPage(mIdx);

	}

	@Override
	protected void onLayout(boolean changed, int l, int t, int r, int b) {
		super.onLayout(changed, l, t, r, b);
		if (changed) {
			showPage(mIdx);
		}
	}

	@Override
	public void addView(View child, ViewGroup.LayoutParams params) {
		// TODO: Implement this method
		FrameLayout l=new FrameLayout(getContext());

		l.addView(child, params);
		wrapper.addView(l);
	}

	@Override
	public void addView(View child) {
		// TODO: Implement this method
		FrameLayout l=new FrameLayout(getContext());
		l.addView(child);
		wrapper.addView(l);
	}

	@Override
	public boolean onInterceptTouchEvent(MotionEvent ev) {
		if (ev.getX() < mTouchScale || ev.getX() > mWidth - mTouchScale) {
			return super.onInterceptTouchEvent(ev);
		}
		else {
			return false;
		}
	}

	private void obtainVelocityTracker(MotionEvent event) {
		if (mVelocityTracker == null) {
			mVelocityTracker = VelocityTracker.obtain();
		}
		mVelocityTracker.addMovement(event);
	}

	private void releaseVelocityTracker() {
		if (mVelocityTracker != null) {
			mVelocityTracker.recycle();
			mVelocityTracker = null;
		}
	}

	@Override
	public boolean onTouchEvent(MotionEvent ev) {
		int action = ev.getAction();
		obtainVelocityTracker(ev);
		switch (action) {
				// Up时，进行判断，如果显示区域大于页面宽度一半则完全显示
			case MotionEvent.ACTION_UP:
				final VelocityTracker velocityTracker = mVelocityTracker;
				velocityTracker.computeCurrentVelocity(1000, mMaximumVelocity);
				int initialVelocityY = (int) velocityTracker.getYVelocity();
				int initialVelocityX = (int) velocityTracker.getXVelocity();
				releaseVelocityTracker();
				int absX=Math.abs(initialVelocityX);
				int absY=Math.abs(initialVelocityY);
				if (absX > mMinimumVelocity && absX > absY) {
					if (initialVelocityX > 0)
						showPage(Math.max(0, mIdx - 1));
					else
						showPage(Math.min(wrapper.getChildCount() - 1, mIdx + 1));
					return true;
				}

				int scrollX = getScrollX();
				int x=scrollX % mWidth;
				if (x < mWidth / 2)
					showPage(scrollX / mWidth);
				else
					showPage(scrollX / mWidth + 1);
				return true;
		}
		return super.onTouchEvent(ev);
	}

	public void showPage(int idx) {
		//wrapper.getChildAt(idx);
		smoothScrollTo(mWidth * idx, 0);
		if (mOnPageChangeListener != null && mIdx != idx)
			mOnPageChangeListener.onPageChange(this, idx);
		mIdx = idx;
	}

	public void showPage(View v) {
		int n=wrapper.getChildCount();
		for (int i=0;i < n;i++)
			if (wrapper.getChildAt(i).equals(v))
				showPage(i);
	}

	public View getPage(int idx) {
		return wrapper.getChildAt(idx);
	}

	public void setOnPageChangeListener(OnPageChangeListener ltr) {
		mOnPageChangeListener = ltr;
	}


	public static interface OnPageChangeListener {
		public void onPageChange(View v, int idx);
	}
}

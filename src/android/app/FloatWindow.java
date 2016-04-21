package android.app;

import android.content.*;
import android.content.res.*;
import android.graphics.*;
import android.graphics.drawable.*;
import android.view.*;
import android.widget.*;

public class FloatWindow
{

	private Context mContext;

	private FloatWindow.TitleBar mTitlebar;

	private WindowManager mWindowManager;

	private WindowManager.LayoutParams mLayoutParams;

	private LinearLayout mLayout;

	private FrameLayout content;
	public FloatWindow(Context context)
	{
		mContext = context;
		init(context);
	}

	private void init(Context context)
	{
		// TODO: Implement this method

		mWindowManager = (WindowManager) context.getApplicationContext().getSystemService(Context.WINDOW_SERVICE);
		mLayoutParams = new WindowManager.LayoutParams();
		mLayoutParams.format = PixelFormat.RGBA_8888;
		mLayoutParams.flags = WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL|WindowManager.LayoutParams.FLAG_WATCH_OUTSIDE_TOUCH;
		//mLayoutParams.flags = WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE|WindowManager.LayoutParams.FLAG_ALT_FOCUSABLE_IM;
		mLayoutParams.type = WindowManager.LayoutParams.TYPE_PHONE ;
		mLayoutParams.width = WindowManager.LayoutParams.WRAP_CONTENT;
		mLayoutParams.height = WindowManager.LayoutParams.WRAP_CONTENT;
		mLayout = new ContentView(context);
		mLayout.setPadding(10,10,10,10);
		mLayout.setOrientation(LinearLayout.VERTICAL);
		
		TypedArray array = context.getTheme().obtainStyledAttributes(new int[] {  
																		  android.R.attr.colorBackground, 
																		  android.R.attr.textColorPrimary, 
																	  }); 
		int backgroundColor = array.getColor(0, 0xFF00FF); 
		int textColor = array.getColor(1, 0xFF00FF); 
		array.recycle();

		GradientDrawable gd=new GradientDrawable();
		gd.setColor(backgroundColor);
		gd.setCornerRadius(4);
		gd.setStroke(1,textColor);
		gd.setAlpha(0x88);
		mLayout.setBackground(gd);
		mWindowManager.addView(mLayout, mLayoutParams);
		mLayout.setVisibility(View.GONE);
		mTitlebar = new TitleBar(context);
		content = new FrameLayout(context);
		mLayout.addView(mTitlebar);
		mLayout.addView(content);
	}

	public void setBackground(Drawable bg)
	{
		mLayout.setBackground(bg);
	}
	
	public Drawable getBackground()
	{
		return mLayout.getBackground();
	}
	
	public void setTitle(CharSequence title)
	{
		mTitlebar.setTitle(title);
	}

	public void setContentView(View v)
	{
		content.removeAllViews();
		content.addView(v);
	}

	public void show()
	{
		mLayout.setVisibility(View.VISIBLE);
	}

	public void hide()
	{
		mLayout.setVisibility(View.GONE);
	}

	public void dismiss()
	{
		mWindowManager.removeView(mLayout);
	}

	public void setFlags(int flags)
	{
		mLayoutParams.flags=flags;
		mWindowManager.updateViewLayout(mLayout, mLayoutParams);
	}
	
	public void setFormat(int format)
	{
		mLayoutParams.format=format;
		mWindowManager.updateViewLayout(mLayout, mLayoutParams);
	}
	
	public void setType(int type)
	{
		mLayoutParams.type=type;
		mWindowManager.updateViewLayout(mLayout, mLayoutParams);
	}
	
	private class TitleBar extends FrameLayout
	{

		private TextView mTitle;
		public TitleBar(Context context)
		{
			super(context);
			mTitle = new TitleView(context);
			TextView mClose=new TextView(context);
			mClose.setText("X");
			mClose.setOnClickListener(new OnClickListener(){

					@Override
					public void onClick(View p1)
					{
						// TODO: Implement this method
						dismiss();
					}
			});
			addView(mTitle);
			addView(mClose,new FrameLayout.LayoutParams(LayoutParams.WRAP_CONTENT,LayoutParams.WRAP_CONTENT,Gravity.RIGHT));
		}
		public void setTitle(CharSequence title)
		{
			mTitle.setText(title);
		}
	}

	private class TitleView extends TextView
	{

		private int lastX=0;
		private int lastY=0;
		private int vx=0;
		private int vy=0;
		private int h=0;
		private int ry;
		private int rx;
		
		public TitleView(Context context)
		{
			super(context);
		}

		@Override
		public boolean onTouchEvent(MotionEvent e)
		{
			// TODO: Implement this method
			Rect frame=new Rect();
			getWindowVisibleDisplayFrame(frame);
			h = frame.top;
			ry = (int) e.getRawY();//获取触摸绝对Y位置
			rx = (int) e.getRawX();//获取触摸绝对X位置
			if (e.getAction() == MotionEvent.ACTION_DOWN)
			{	
				vy = ry - (int) e.getY();//获取视图的Y位置
				vx = rx - (int) e.getX();//获取视图的X位置
				lastY = ry;//记录按下的Y位置
				lastX = rx;//记录按下的X位置
			}
			else if (e.getAction() == MotionEvent.ACTION_MOVE)
			{
				mLayoutParams.gravity = Gravity.LEFT | Gravity.TOP ;//调整悬浮窗口至左上角
				mLayoutParams.x = vx + (rx - lastX);//移动的相对位置
				mLayoutParams.y = vy + (ry - lastY) - h + 3;//移动的相对位置
				mWindowManager.updateViewLayout(mLayout, mLayoutParams);//调整悬浮窗至指定的位置
			}
			return true;

			//return super.onTouchEvent(event);
		}

	}
	
	private class ContentView extends LinearLayout
	{

		private int lastX=0;
		private int lastY=0;
		private int ry;
		private int rx;

		private int vw;

		private int vh;

		public ContentView(Context context)
		{
			super(context);
		}

		@Override
		public boolean onInterceptTouchEvent(MotionEvent e)
		{
			// TODO: Implement this method
			if(e.getAction()==MotionEvent.ACTION_OUTSIDE)
			{
				mLayoutParams.flags = WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE;
				mWindowManager.updateViewLayout(mLayout, mLayoutParams);

			}
			else if (e.getAction() == MotionEvent.ACTION_DOWN)
			{	
				mLayoutParams.flags = WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL|WindowManager.LayoutParams.FLAG_WATCH_OUTSIDE_TOUCH;
				mWindowManager.updateViewLayout(mLayout, mLayoutParams);
			}
			
			return super.onInterceptTouchEvent(e);
		}
		
		

		@Override
		public boolean onTouchEvent(MotionEvent e)
		{
			// TODO: Implement this method
			ry = (int) e.getRawY();//获取触摸绝对Y位置
			rx = (int) e.getRawX();//获取触摸绝对X位置
			if(e.getAction()==MotionEvent.ACTION_OUTSIDE)
			{
				mLayoutParams.flags = WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE;
				mWindowManager.updateViewLayout(mLayout, mLayoutParams);
				
			}
			else if (e.getAction() == MotionEvent.ACTION_DOWN)
			{	
				mLayoutParams.flags = WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL|WindowManager.LayoutParams.FLAG_WATCH_OUTSIDE_TOUCH;
				mWindowManager.updateViewLayout(mLayout, mLayoutParams);
				vw = getWidth();//获取视图的Y位置
				vh = getHeight();//获取视图的X位置
				lastY = ry;//记录按下的Y位置
				lastX = rx;//记录按下的X位置
			}
			else if (e.getAction() == MotionEvent.ACTION_MOVE)
			{
				mLayoutParams.width = vw + (rx - lastX);//移动的相对位置
				mLayoutParams.height = vh + (ry - lastY);//移动的相对位置
				mWindowManager.updateViewLayout(mLayout, mLayoutParams);//-调整悬浮窗至指定的位置
			}
			return true;

			//return super.onTouchEvent(e);
		}

	}
}

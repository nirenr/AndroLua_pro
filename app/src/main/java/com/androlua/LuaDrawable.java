package com.androlua;
import android.graphics.drawable.*;
import android.graphics.*;
import com.luajava.*;

public class LuaDrawable extends Drawable
{

	private LuaObject mDraw;

	private Paint mPaint;
	
	
	public LuaDrawable(LuaObject func)
	{
		mDraw=func;
		mPaint=new Paint();
	}

	@Override
	public void draw(Canvas p1)
	{
		try
		{
			mDraw.call(p1,mPaint);
		}
		catch (LuaException e)
		{
			
		}
		// TODO: Implement this method
	}

	@Override
	public void setAlpha(int p1)
	{
		mPaint.setAlpha(p1);
		// TODO: Implement this method
	}

	@Override
	public void setColorFilter(ColorFilter p1)
	{
		mPaint.setColorFilter(p1);
		// TODO: Implement this method
	}

	@Override
	public int getOpacity()
	{
		// TODO: Implement this method
		return 0;
	}

	public Paint getPaint()
	{
		return mPaint;
	}
}

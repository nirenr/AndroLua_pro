package android.widget;

import android.graphics.*;
import android.graphics.drawable.*;
import android.view.*;
import com.androlua.util.*;

public class RippleHelper extends Drawable implements View.OnTouchListener {

	private int state;

	private int mWidth;

	private int mStep;

	@Override
	public boolean onTouch(View p1, MotionEvent p2) {
		// TODO: Implement this method
		onTouchEvent(p2);
		return false;
	}

	private boolean mEnabled;

	private int mRadius;

	private TimerX mTimer;

	private RippleHelper.task mTask;

	private Paint mPaint2;

	private float mX;

	private float mY;

	private View mView;

	private Drawable mBackground;


	public RippleHelper(View view) {
		mView = view;
		init();
	}

	private void init() {
		if (mView.isClickable())
			mEnabled = true;

		mBackground = mView.getBackground();
		mView.setBackgroundDrawable(this);
		mView.setOnTouchListener(this);

		mPaint2 = new Paint();
		mPaint2.setColor(0x44aaaaaa);
		mPaint2.setStrokeWidth(4);
		mTimer = new TimerX();
		mTask = new task();
		mTimer.schedule(mTask, 0, 16);

		mTask.setEnabled(false);
		mRadius = 0;
	}

	public void onTouchEvent(MotionEvent event) {
		// TODO: Implement this method
		if (!mView.hasOnClickListeners() && !mEnabled)
			return;
		switch (event.getAction()) {
			case MotionEvent.ACTION_DOWN:
				Rect rect=getBounds();
				mWidth=(int)Math.hypot(rect.bottom, rect.right);
				mStep = Math.max(mWidth / 60, 1);
				mRadius=0;
				mTask.setEnabled(true);
				//mEnabled = true;
				mX = event.getX();
				mY = event.getY();
				state = 1;
				break;
			case MotionEvent.ACTION_UP:
				state = 2;
		}

	}

	@Override
	public void draw(Canvas canvas) {
		if (mBackground != null) {
			mBackground.setBounds(getBounds());
			mBackground.draw(canvas);
		}
		if (state != 0) {
			canvas.drawRect(getBounds(),mPaint2);
			int w = mWidth;
			int n=0;
			for (int r=mRadius;r >= 0;r = r - w) {
				if(n++==3){
					mPaint2.setStyle(Paint.Style.STROKE);
				}
				canvas.drawCircle(mX, mY, r, mPaint2);
			}
			mPaint2.setStyle(Paint.Style.FILL);
		}
	}

	public void setBackgroundColor(int color) {
		// TODO: Implement this method
		mBackground = new ColorDrawable(color);
	}

	public void setRippleColor(int color) {
		// TODO: Implement this method
		mPaint2.setColor(color);
	}

	@Override
	public void setAlpha(int p1) {
		// TODO: Implement this method
		mPaint2.setAlpha(p1);
	}

	@Override
	public void setColorFilter(ColorFilter p1) {
		// TODO: Implement this method
		mPaint2.setColorFilter(p1);
	}

	@Override
	public int getOpacity() {
		// TODO: Implement this method
		return 0;
	}

	private class task extends TimerTaskX {

		private int mAlpha;

		private int mOldAlpha;

		@Override
		public void run() {
			// TODO: Implement this method
			if (state == 1) {
				mRadius += mStep;
				mView.postInvalidate();
			}
			else if (state == 2) {
				mRadius += mStep * 4;
				mView.postInvalidate();
				if (mRadius / mWidth >=1) {
					mRadius=mWidth;
					mOldAlpha = mPaint2.getAlpha();
					mAlpha = mOldAlpha;
					state = 3;
				}
			}
			else if (state == 3) {
				mAlpha -= Math.max(mOldAlpha / 10, 2);
				mPaint2.setAlpha(mAlpha);
				mView.postInvalidate();
				if (mAlpha < 16) {
					mAlpha = mOldAlpha;
					mPaint2.setAlpha(mAlpha);
					state = 0;
				}
			}
			else {
				mRadius = 0;
				setEnabled(false);
			}
		}
	}

}

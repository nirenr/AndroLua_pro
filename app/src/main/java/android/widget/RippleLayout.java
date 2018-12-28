package android.widget;
import android.content.Context;
import android.graphics.drawable.Drawable;
import android.view.View;
import android.view.ViewGroup;

public class RippleLayout extends FrameLayout {

	private int mChildCount;

	private int count;

	private int mRippleColor=0x44aaaaaa;

	private boolean mCircle;
	private boolean mSingle;
	private int mRippleLineColor;

	public void setCircle(boolean circle) {
		mCircle = circle;
	}

	public boolean isCircle() {
		return mCircle;
	}

	public RippleLayout(Context context) {
		super(context);
	}

	@Override
	protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
		// TODO: Implement this method
		super.onLayout(changed, left, top, right, bottom);
		count = getChildCount();
		if (mChildCount == count)
			return;
		mChildCount = count;
		setRippleDrawable(this);
	}

	private void setRippleDrawable(View view) {
		// TODO: Implement this method
		if (view instanceof ViewGroup) {

			ViewGroup g=(ViewGroup) view;
			int n=g.getChildCount();
			for (int i=0;i < n;i++) {
				View v=g.getChildAt(i);
				if (!(v instanceof RippleLayout))
					setRippleDrawable(v);
			}
		}
		else {
			Drawable bg=view.getBackground();
			RippleHelper rip;
			if (bg instanceof RippleHelper) {
				rip = (RippleHelper)bg;
			}
			else {
				rip = new RippleHelper(view);
			}
			rip.setRippleColor(mRippleColor);
			rip.setRippleLineColor(mRippleLineColor);
			rip.setCircle(mCircle);
			rip.setSingle(mSingle);
		}
	}

	public void setSingle(boolean single) {
		mSingle = single;
	}
	public void setRippleColor(int color) {
		mRippleColor = color;
	}

	public void setRippleLineColor(int color) {
		// TODO: Implement this method
		mRippleLineColor=color;
	}

}

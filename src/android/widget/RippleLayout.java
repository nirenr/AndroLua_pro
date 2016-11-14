package android.widget;
import android.content.*;
import android.view.*;
import com.androlua.util.*;
import android.graphics.drawable.*;

public class RippleLayout extends FrameLayout {

	private int mChildCount;

	private int count;

	private int mRippleColor=0x44aaaaaa;

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
			if (bg instanceof RippleHelper) {
				((RippleHelper)bg).setRippleColor(mRippleColor);
			}
			else {
				new RippleHelper(view).setRippleColor(mRippleColor);
			}
		}
	}

	public void setRippleColor(int color) {
		mRippleColor = color;
	}
}

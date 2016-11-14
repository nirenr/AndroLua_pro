package android.widget;
import android.content.*;
import android.graphics.*;
import android.util.*;
import android.view.*;
import com.androlua.*;
import com.androlua.util.*;
import android.graphics.drawable.*;

public class FloatButton extends ImageView {

	private PopupWindow mWindow;

	private CircleImageView mButton;

	private int mGravity;

	private CardView mCard;

	private DisplayMetrics dm;

	private RippleHelper mRippleHelper;

	public FloatButton(Context context) {
		super(context);
		init(context);
	}

	@Override
	protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
		// TODO: Implement this method
		super.onLayout(changed, left, top, left, top);
		if (!mWindow.isShowing())
			mWindow.showAtLocation((View)getParent(), mGravity, 0, 0);
		mWindow.update();
		//mWindow.update((View)getParent(),dp(64),dp(64));
	}

	public void setGravity(int gravity){
		mGravity=gravity;
	}
	
	private int dp(float n) {
		// TODO: Implement this method
		return (int)TypedValue.applyDimension(1,n,dm);
	}
	

	private void init(Context context) {
		
		// TODO: Implement this method
		dm=context.getResources().getDisplayMetrics();
		
		FrameLayout layout=new FrameLayout(context);
		FrameLayout.LayoutParams lp=new FrameLayout.LayoutParams(-2, -2);
		lp.setMargins(dp(16),dp(16),dp(16),dp(16));
		mCard = new CardView(context);
		mCard.setCardElevation(dp(8));
		//mCard.setCardBackgroundColor(0xff0000ff);
		
		mButton = new CircleImageView(context);
		mButton.setImageResource(R.drawable.icon);
		mRippleHelper=new RippleHelper(mButton);
		FrameLayout.LayoutParams lp2=new FrameLayout.LayoutParams(dp(64), dp(64));
		layout.addView(mCard,lp);
		mCard.addView(mButton,lp2);
		mCard.setRadius(dp(32));
		mWindow = new PopupWindow(-2,-2);
		
		mWindow.setContentView(layout);
		//setVisibility(GONE);
	}

	public void setImageBitmap(Bitmap bm) {
		// TODO: Implement this method
		mButton.setImageBitmap(bm);
	}

	public void setImageResource(int resId) {
		// TODO: Implement this method
		mButton.setImageResource(resId);
	}

	public void setImageDrawable(Drawable drawable) {
		// TODO: Implement this method
		mButton.setImageDrawable(drawable);
	}
	
	public void setRippleColor(int color) {
		mRippleHelper.setRippleColor(color);
	}
}

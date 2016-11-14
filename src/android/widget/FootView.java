package android.widget;

import android.content.*;
import android.util.*;
import com.androlua.*;

public class FootView extends RelativeLayout {

	private DisplayMetrics dm;
	public FootView(Context context) {
		super(context);
		initView(context);
	}

	public FootView(Context context, AttributeSet attrs) {
		super(context, attrs);
		initView(context);
	}

	public FootView(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		initView(context);
	}

	private void initView(Context context) {
// TODO: Implement this method
		dm = context.getResources().getDisplayMetrics();
		RelativeLayout layout=new RelativeLayout(context);
		layout.setPadding(0, dp(20), 0, dp(20));
		RelativeLayout.LayoutParams lp=new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT);
		lp.addRule(10);
		addView(layout, lp);
		
		RelativeLayout layout2=new RelativeLayout(context);
		RelativeLayout.LayoutParams lp2=new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT);
		lp2.addRule(13);
		layout.addView(layout2, lp2);

		ImageView img1=new ImageView(context);
		img1.setId(id.pullup_icon);
		img1.setBackgroundResource(R.drawable.pullup_icon_big);
		RelativeLayout.LayoutParams lp3=new LayoutParams(dp(19), dp(31));
		lp3.setMargins(dp(60), 0, 0, 0);
		lp3.addRule(15);
		layout2.addView(img1, lp3);

		ImageView img2=new ImageView(context);
		img2.setId(id.loading_icon);
		img2.setVisibility(GONE);
		img2.setBackgroundResource(R.drawable.loading);
		RelativeLayout.LayoutParams lp4=new LayoutParams(dp(16), dp(16));
		lp4.setMargins(dp(60), 0, 0, 0);
		lp4.addRule(15);
		layout2.addView(img2, lp4);

		TextView tv1=new TextView(context);
		tv1.setId(id.loadstate_tv);
		tv1.setText(PullingLayout.string.pullup_to_load);
		tv1.setTextSize(16);
		RelativeLayout.LayoutParams lp5=new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
		lp5.addRule(13);
		layout2.addView(tv1, lp5);

		ImageView img3=new ImageView(context);
		img3.setId(id.loadstate_iv);
		img3.setVisibility(GONE);
		RelativeLayout.LayoutParams lp6=new LayoutParams(dp(16), dp(16));
		lp6.setMargins(0, 0, dp(8), 0);
		lp6.addRule(15);
		lp6.addRule(0, id.loadstate_tv);
		layout2.addView(img3, lp6);

	}

	private int dp(float n) {
// TODO: Implement this method
		return (int)TypedValue.applyDimension(1, n, dm);
	}


	public final class id {

		public static final int pullup_icon = 100;

		public static final int loading_icon = 101;

		public static final int loadstate_tv = 102;

		public static final int loadstate_iv = 103;

	}

}

package android.widget;

import android.content.*;
import android.util.*;
import android.widget.RelativeLayout.*;
import com.androlua.*;

public class HeadView extends RelativeLayout  
{

	private DisplayMetrics dm;
	public HeadView(Context context) {
		super(context);
		initView(context);
	}
	
	public HeadView(Context context, AttributeSet attrs) {
		super(context, attrs);
		initView(context);
	}

	public HeadView(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		initView(context);
	}
	
	private void initView(Context context) {
		// TODO: Implement this method
		dm=context.getResources().getDisplayMetrics();
		RelativeLayout layout=new RelativeLayout(context);
		layout.setPadding(0,dp(20),0,dp(20));
		RelativeLayout.LayoutParams lp=new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT);
		lp.addRule(12);
		addView(layout, lp);
		
		RelativeLayout layout2=new RelativeLayout(context);
		RelativeLayout.LayoutParams lp2=new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT);
		lp2.addRule(13);
		layout.addView(layout2,lp2);
		
		ImageView img1=new ImageView(context);
		img1.setId(id.pull_icon);
		img1.setBackgroundResource(R.drawable.pull_icon_big);
		RelativeLayout.LayoutParams lp3=new LayoutParams(dp(19), dp(31));
		lp3.setMargins(dp(60),0,0,0);
		lp3.addRule(15);
		layout2.addView(img1,lp3);
		
		ImageView img2=new ImageView(context);
		img2.setId(id.refreshing_icon);
		img2.setVisibility(GONE);
		img2.setBackgroundResource(R.drawable.loading);
		RelativeLayout.LayoutParams lp4=new LayoutParams(dp(16), dp(16));
		lp4.setMargins(dp(60),0,0,0);
		lp4.addRule(15);
		layout2.addView(img2,lp4);
		
		TextView tv1=new TextView(context);
		tv1.setId(id.state_tv);
		tv1.setText(PullingLayout.string.pull_to_refresh);
		tv1.setTextSize(16);
		RelativeLayout.LayoutParams lp5=new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
		lp5.addRule(13);
		layout2.addView(tv1,lp5);
		
		ImageView img3=new ImageView(context);
		img3.setId(id.state_iv);
		img3.setVisibility(GONE);
		RelativeLayout.LayoutParams lp6=new LayoutParams(dp(16), dp(16));
		lp6.setMargins(0,0,dp(8),0);
		lp6.addRule(15);
		lp6.addRule(0,id.state_tv);
		layout2.addView(img3,lp6);
		
	}

	private int dp(float n) {
		// TODO: Implement this method
		return (int)TypedValue.applyDimension(1,n,dm);
	}
	
	
	public final class id {

		public static final int pull_icon = 100;

		public static final int refreshing_icon = 101;

		public static final int state_tv = 102;

		public static final int state_iv = 103;
		
	}
	
}

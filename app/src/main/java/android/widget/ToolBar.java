package android.widget;
import android.content.*;
import android.graphics.*;
import android.graphics.drawable.*;
import android.util.*;
import android.view.*;
import com.androlua.util.*;
import android.view.ViewGroup.*;
import com.androlua.*;
import android.widget.ImageView.*;

public class ToolBar extends LinearLayout {

	private TextView mTitle;

	private TextView mSubTitle;

	private ImageView mNavi;

	private ImageView mLogo;

	private ImageView mMore;
	
	private LinearLayout wrapper;
	
	private PopupMenu mMenu;

	private OnLogoClickListener mOnLogoCilck;
	private OnNaviClickListener mOnNaviCilck;
	private OnMenuItemClickListener mOnMenuItemClick;

	private DisplayMetrics dm;

	private int mHeight;

	public ToolBar(Context context) {
		super(context);
		init(context);
	}

	private void init(Context context) {
		// TODO: Implement this method
		dm = context.getResources().getDisplayMetrics();
		mHeight = dp(48);
		LinearLayout.LayoutParams lp = new LayoutParams(mHeight, mHeight);
		setMinimumHeight(mHeight);
		mNavi = new ImageView(context);
		mNavi.setScaleType(ScaleType.FIT_CENTER);
		mNavi.setVisibility(GONE);
		super.addView(mNavi, lp);

		mLogo = new ImageView(context);
		mLogo.setScaleType(ScaleType.FIT_CENTER);
		mLogo.setImageResource(R.drawable.icon);
		mLogo.setVisibility(GONE);
		super.addView(mLogo, lp);

		LinearLayout titleLayout=new LinearLayout(context);
		int p=dp(1);
		titleLayout.setPadding(p * 4, p, p, p);
		titleLayout.setOrientation(LinearLayout.VERTICAL);
		titleLayout.setGravity(Gravity.CENTER);
		super.addView(titleLayout, new LayoutParams(LayoutParams.MATCH_PARENT, mHeight, 1));

		mTitle = new TextView(context);
		mTitle.setTextSize(1, 20);
		mTitle.setSingleLine(true);
		mTitle.setTypeface(Typeface.DEFAULT_BOLD);
		titleLayout.addView(mTitle, new LayoutParams(LayoutParams.MATCH_PARENT, dp(26)));
		mSubTitle = new TextView(context);
		mSubTitle.setTextSize(1, 14);
		mSubTitle.setSingleLine(true);
		mSubTitle.setVisibility(GONE);
		titleLayout.addView(mSubTitle, new LayoutParams(LayoutParams.MATCH_PARENT, dp(20)));

		wrapper=new LinearLayout(context);
		super.addView(wrapper, new LayoutParams(LayoutParams.WRAP_CONTENT, mHeight));
		
		mMore = new ImageView(context);
		mMore.setScaleType(ScaleType.FIT_CENTER);
		mMore.setVisibility(GONE);
		super.addView(mMore, lp);

		new RippleHelper(mNavi);
		new RippleHelper(mMore);

		Paint paint=new Paint();
		paint.setColor(0xff888888);

		Bitmap nb=Bitmap.createBitmap(mHeight, mHeight, Bitmap.Config.ARGB_4444);
		Canvas nc=new Canvas(nb);
		double h=mHeight;
		double w=h;
		nc.drawRect((int)(w / 4), (int)(h / 32 * 10), (int)(w / 4 * 3), (int)(h / 32 * 12), paint);
		nc.drawRect((int)(w / 4), (int)(h / 32 * 15), (int)(w / 4 * 3), (int)(h / 32 * 17), paint);
		nc.drawRect((int)(w / 4), (int)(h / 32 * 20), (int)(w / 4 * 3), (int)(h / 32 * 22), paint);
		mNavi.setImageBitmap(nb);

		Bitmap mb=Bitmap.createBitmap(mHeight, mHeight, Bitmap.Config.ARGB_4444);
		Canvas mc=new Canvas(mb);
		mc.drawCircle((int)(w / 2), (int)(h / 3), (int)(w / 16), paint);
		mc.drawCircle((int)(w / 2), (int)(h / 2), (int)(w / 16), paint);
		mc.drawCircle((int)(w / 2), (int)(h / 3 * 2), (int)(w / 16), paint);
		mMore.setImageBitmap(mb);

		mNavi.setOnClickListener(new OnClickListener(){

				@Override
				public void onClick(View p1) {
					// TODO: Implement this method
					if (mOnNaviCilck != null)
						mOnNaviCilck.onNaviClick(p1);
				}
			});

		mLogo.setOnClickListener(new OnClickListener(){

				@Override
				public void onClick(View p1) {
					// TODO: Implement this method
					if (mOnLogoCilck != null)
						mOnLogoCilck.onLogoClick(p1);
				}
			});

		mMore.setOnClickListener(new OnClickListener(){

				@Override
				public void onClick(View p1) {
					// TODO: Implement this method
					if (mMenu != null)
						mMenu.show();
				}
			});

		mMenu = new PopupMenu(getContext(), mMore);
		mMenu.setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener(){

				@Override
				public boolean onMenuItemClick(MenuItem p1) {
					// TODO: Implement this method
					if (mOnMenuItemClick != null)
						return mOnMenuItemClick.onMenuItemClick(p1);
					return false;
				}
			});
	}

	private int dp(float n) {
		// TODO: Implement this method
		return (int)TypedValue.applyDimension(1, n, dm);
	}

	@Override
	public void addView(View child) {
		// TODO: Implement this method
		wrapper.addView(child);
	}

	@Override
	public void addView(View child, ViewGroup.LayoutParams params) {
		// TODO: Implement this method
		wrapper.addView(child, params);
	}

	
	
	
	public void setLogo(Drawable logo) {
		mLogo.setImageDrawable(logo);
	}

	public void setNaviEnabled(boolean enabled) {
		mNavi.setVisibility(enabled ?VISIBLE: GONE);
	}

	public void setLogoEnabled(boolean enabled) {
		mLogo.setVisibility(enabled ?VISIBLE: GONE);
	}

	public void setMenuEnabled(boolean enabled) {
		mMore.setVisibility(enabled ?VISIBLE: GONE);
	}

	public void setNaviIcon(Drawable icon) {
		mNavi.setImageDrawable(icon);
	}

	public void setTitle(CharSequence title) {
		mTitle.setText(title);
	}

	public void setTitleColor(int color) {
		mTitle.setTextColor(color);
	}

	public void setSubtitle(CharSequence title) {
		if (title == null || title.length() == 0)
			mSubTitle.setVisibility(GONE);
		else
			mSubTitle.setVisibility(VISIBLE);
		mSubTitle.setText(title);
	}

	public void setSubtitleColor(int color) {
		mSubTitle.setTextColor(color);
	}

	public void setOnLogoClickListener(OnLogoClickListener l) {
		mOnLogoCilck = l;
	}

	public void setOnNaviClickListener(OnNaviClickListener l) {
		mOnNaviCilck = l;
	}

	public void setOnMenuItemClickListener(OnMenuItemClickListener listener) {
		mOnMenuItemClick = listener;
	}

	public Menu getMenu() {
		setMenuEnabled(true);
		return mMenu.getMenu();
	}

	public static interface OnLogoClickListener {
		public void onLogoClick(View view);
	}

	public static interface OnNaviClickListener {
		public void onNaviClick(View view);
	}

	public static interface OnMenuItemClickListener {
        public boolean onMenuItemClick(android.view.MenuItem item);
    }

}

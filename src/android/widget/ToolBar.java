package android.widget;
import android.content.*;
import android.graphics.drawable.*;
import android.view.*;
import android.widget.*;
import android.view.View.*;
import android.widget.ToolBar.*;
import android.graphics.*;
import com.androlua.*;

public class ToolBar extends FrameLayout
{

	private ViewGroup toolbar;

	private TextView mTitle;

	private TextView mSubTitle;

	private ImageButton mNavi;

	private ImageButton mLogo;

	private ImageButton mMore;

	private PopupMenu mMenu;

	private OnLogoClickListener mOnLogoCilck;
	private OnNaviClickListener mOnNaviCilck;
	private OnMenuItemClickListener mOnMenuItemClick;

	public ToolBar(Context context)
	{
		super(context);
		init(context);
	}

	private void init(Context context)
	{
		// TODO: Implement this method
		LayoutInflater inflater = (LayoutInflater)context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		toolbar = (ViewGroup) inflater.inflate(R.layout.toolbar, null);
		addView(toolbar,new LayoutParams(LayoutParams.MATCH_PARENT,LayoutParams.MATCH_PARENT));
		mTitle = (TextView)toolbar.findViewById(R.id.title);
		mSubTitle = (TextView)toolbar.findViewById(R.id.subtitle);

		mNavi = (ImageButton)toolbar.findViewById(R.id.navi);
		mNavi.setImageDrawable(new Drawable(){

				@Override
				public void setAlpha(int p1)
				{
					// TODO: Implement this method
				}

				@Override
				public void setColorFilter(ColorFilter p1)
				{
					// TODO: Implement this method
				}

				@Override
				public int getOpacity()
				{
					// TODO: Implement this method
					return 0;
				}


				@Override
				public void draw(Canvas canvas)
				{
					// TODO: Implement this method
					Rect rect=this.getBounds();
					Paint paint=new Paint();
					paint.setColor(0xff888888);
					canvas.drawRect(rect.right / 4, rect.bottom / 16 * 5, rect.right / 4 * 3, rect.bottom / 16 * 6, paint);
					canvas.drawRect(rect.right / 4, rect.bottom / 32 * 15, rect.right / 4 * 3, rect.bottom / 32 * 17, paint);
					canvas.drawRect(rect.right / 4, rect.bottom / 16 * 10, rect.right / 4 * 3, rect.bottom / 16 * 11, paint);
				}
			}
		);

		mNavi.setOnClickListener(new OnClickListener(){

				@Override
				public void onClick(View p1)
				{
					// TODO: Implement this method
					if (mOnNaviCilck != null)
						mOnNaviCilck.onNaviClick(p1);
				}
			});

		mLogo = (ImageButton)toolbar.findViewById(R.id.logo);
		mLogo.setOnClickListener(new OnClickListener(){

				@Override
				public void onClick(View p1)
				{
					// TODO: Implement this method
					if (mOnLogoCilck != null)
						mOnLogoCilck.onLogoClick(p1);
				}
			});


		mMore = (ImageButton)toolbar.findViewById(R.id.menu);
		mMore.setImageDrawable(new Drawable(){

				@Override
				public void draw(Canvas canvas)
				{
					// TODO: Implement this method
					Rect rect=this.getBounds();
					Paint paint=new Paint();
					paint.setColor(0xff888888);
					canvas.drawCircle(rect.right / 2, rect.bottom / 3, rect.bottom / 16, paint);
					canvas.drawCircle(rect.right / 2, rect.bottom / 2, rect.bottom / 16, paint);
					canvas.drawCircle(rect.right / 2, rect.bottom / 3 * 2, rect.bottom / 16, paint);

				}

				@Override
				public void setAlpha(int p1)
				{
					// TODO: Implement this method
				}

				@Override
				public void setColorFilter(ColorFilter p1)
				{
					// TODO: Implement this method
				}

				@Override
				public int getOpacity()
				{
					// TODO: Implement this method
					return 0;
				}
			});

		mMore.setOnClickListener(new OnClickListener(){

				@Override
				public void onClick(View p1)
				{
					// TODO: Implement this method
					if (mMenu != null)
						mMenu.show();
				}
			});


		mMenu = new PopupMenu(getContext(), mMore);
		mMenu.setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener(){

				@Override
				public boolean onMenuItemClick(MenuItem p1)
				{
					// TODO: Implement this method
					if (mOnMenuItemClick != null)
						return mOnMenuItemClick.onMenuItemClick(p1);
					return false;
				}
			});
	}

	public void setLogo(Drawable logo)
	{
		mLogo.setImageDrawable(logo);
	}

	public void setNaviEnabled(boolean enabled)
	{
		mNavi.setVisibility(enabled ?VISIBLE: GONE);
	}

	public void setLogoEnabled(boolean enabled)
	{
		mLogo.setVisibility(enabled ?VISIBLE: GONE);
	}
	
	public void setMenuEnabled(boolean enabled)
	{
		mMore.setVisibility(enabled ?VISIBLE: GONE);
	}
	
	public void setNaviIcon(Drawable icon)
	{
		mNavi.setImageDrawable(icon);
	}

	public void setTitle(CharSequence title)
	{
		mTitle.setText(title);
	}

	public void setSubtitle(CharSequence title)
	{
		if(title==null||title.length()==0)
			mSubTitle.setVisibility(GONE);
		else
			mSubTitle.setVisibility(VISIBLE);
		mSubTitle.setText(title);
	}

	public void setOnLogoClickListener(OnLogoClickListener l)
	{
		mOnLogoCilck = l;
	}

	public void setOnNaviClickListener(OnNaviClickListener l)
	{
		mOnNaviCilck = l;
	}

	public void setOnMenuItemClickListener(OnMenuItemClickListener listener)
	{
		mOnMenuItemClick = listener;
	}

	public Menu getMenu()
	{
		setMenuEnabled(true);
		return mMenu.getMenu();
	}

	public static interface OnLogoClickListener
	{
		public void onLogoClick(View view);
	}

	public static interface OnNaviClickListener
	{
		public void onNaviClick(View view);
	}

	public static interface OnMenuItemClickListener
    {
        public boolean onMenuItemClick(android.view.MenuItem item);
    }

}

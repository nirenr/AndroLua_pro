package android.widget;

/**
 * PageView适配器
 */
import android.os.*;
import android.view.*;
import java.util.*;

public class ArrayPageAdapter extends PageAdapter {
	public List<View> mListViews;

	public ArrayPageAdapter(List<View> views) {
		this.mListViews = views;
	}
	
	public ArrayPageAdapter(View[] views) {
		this.mListViews = Arrays.asList(views);
	}
	
	@Override
	public void destroyItem(View arg0, int arg1, Object arg2) {
		((PageView) arg0).removeView(mListViews.get(arg1));
	}

	@Override
	public int getCount() {
		return mListViews.size();
	}

	@Override
	public Object instantiateItem(View arg0, int arg1) {
		((PageView) arg0).addView(mListViews.get(arg1), 0);
		return mListViews.get(arg1);
	}

	@Override
	public boolean isViewFromObject(View arg0, Object arg1) {
		return arg0 == (arg1);
	}

}

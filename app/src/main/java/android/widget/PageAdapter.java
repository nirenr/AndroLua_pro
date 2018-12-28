package android.widget;

import android.view.View;

/**
 * Created by Administrator on 2018/09/02 0002.
 */

public class PageAdapter extends BasePageAdapter {
    private final Adapter mAdapter;
    private View[] mView;

    public PageAdapter(Adapter adapter) {
        mAdapter = adapter;
        mView=new View[adapter.getViewTypeCount()];
    }


    @Override
    public void destroyItem(View container, int position, Object object) {
        View view = (View) object;
        ((PageView) container).removeView(view);
        mView[mAdapter.getItemViewType(position)] = view;
    }

    @Override
    public int getCount() {
        return mAdapter.getCount();
    }

    @Override
    public Object instantiateItem(View container, int position) {
        int type=mAdapter.getItemViewType(position);
        if (mView[type] != null)
            ((PageView) container).removeView(mView[type]);
        View view = mAdapter.getView(position, mView[type], ((PageView) container));
        ((PageView) container).addView(view, 0);
        mView[type]=null;
        return view;
    }

    @Override
    public boolean isViewFromObject(View arg0, Object arg1) {
        return arg0 == (arg1);
    }


}

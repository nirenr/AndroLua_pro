package android.widget;

/**
 * PageView适配器
 */

import android.view.View;

import java.util.ArrayList;
import java.util.Arrays;

public class ArrayPageAdapter extends BasePageAdapter {
    public ArrayList<View> mListViews;

    public ArrayPageAdapter() {
        this.mListViews = new ArrayList<>();
    }

    public ArrayPageAdapter(ArrayList<View> views) {
        this.mListViews = views;
    }

    public ArrayPageAdapter(View[] views) {
        this.mListViews = new ArrayList<>(Arrays.asList(views));
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

    public void add(View view) {
        mListViews.add(view);
    }

    public void insert(int index, View view) {
        mListViews.add(index, view);
    }

    public View remove(int index) {
       return mListViews.remove(index);
    }

    public boolean remove(View view) {
        return mListViews.remove(view);
    }

    public View getItem(int index) {
        return mListViews.get(index);
    }

    public ArrayList<View> getData() {
        return mListViews;
    }

}

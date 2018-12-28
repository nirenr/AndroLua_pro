package com.androlua;

import android.app.*;
import android.view.*;
import android.os.*;
import android.widget.TextView;

import com.luajava.*;

public class LuaFragment extends Fragment {

    private LuaTable mLayout = null;

    private LuaObject mLoadLayout = null;
    private View mView;

    public LuaFragment() {

    }

    /*
        public LuaFragment(LuaTable layout){
            mLoadLayout=layout.getLuaState().getLuaObject("loadlayout");
            mLayout=layout;
        }*/
    public void setLayout(LuaTable layout) {
        mLayout = layout;
        mView = null;
    }

    public void setLayout(View layout) {
        mView = layout;
        mLayout = null;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        try {
            if (mView != null)
                return mView;
            if (mLayout != null)
                return (View) ((LuaObject) (mLayout.getLuaState().getLuaObject("require").call("loadlayout"))).call(mLayout);
            return new TextView(getActivity());
        } catch (Exception e) {
            throw new IllegalArgumentException(e.getMessage());
        }
    }
}

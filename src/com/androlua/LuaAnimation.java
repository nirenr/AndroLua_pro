package com.androlua;

import android.view.animation.Animation;
import android.view.animation.Transformation;

import com.luajava.LuaException;
import com.luajava.LuaFunction;

/**
 * Created by Administrator on 2016/12/08 0008.
 */

public class LuaAnimation extends Animation {

    private LuaFunction mAnimation;

    public LuaAnimation(LuaFunction animation){
        mAnimation=animation;
    }

    @Override
    protected void applyTransformation(float interpolatedTime, Transformation t) {
        super.applyTransformation(interpolatedTime, t);
        try {
            mAnimation.call(interpolatedTime,t);
        } catch (LuaException e) {
            e.printStackTrace();
        }
    }

    @Override
    protected float resolveSize(int type, float value, int size, int parentSize) {
        return super.resolveSize(type, value, size, parentSize);
    }
}

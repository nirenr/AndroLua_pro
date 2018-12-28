package com.androlua;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.ColorFilter;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PixelFormat;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.drawable.Drawable;
import android.util.DisplayMetrics;
import android.util.TypedValue;

/**
 * Created by Administrator on 2018/09/02 0002.
 */

public class LoadingDrawable extends Drawable {
    private final DisplayMetrics dm;
    private int n = 0;
    private int m = 0;
    private int x = 0;
    private int y = 0;
    private int sn = 3;
    private int sm = 1;

    static final int STATE_LOADING = 0;
    static final int STATE_SUCCESS = 1;
    static final int STATE_FAIL = -1;
    private Paint p;
    private int mState;

    public LoadingDrawable(Context context) {
        dm = context.getResources().getDisplayMetrics();
        p = new Paint();
        p.setStyle(Paint.Style.STROKE);
        p.setAntiAlias(true);
        p.setStrokeWidth(dp(8));
        p.setColor(0x88888888);
    }

    private int dp(float n) {

        return (int) TypedValue.applyDimension(1, n, dm);
    }

    public void setState(int state) {
        mState = state;
    }

    void loading() {
        reset();
    }

    void succe() {
        mState = STATE_SUCCESS;
    }

    void fail() {
        mState = STATE_FAIL;
    }

    void reset() {
        mState = STATE_LOADING;
        sn = 3;
        sm = 1;
        n = 0;
        m = 0;
        x = 0;
        y = 0;
        invalidateSelf();
    }

    @Override
    public void draw(Canvas c) {
        Rect b = new Rect(getBounds());
        int r = (int) ((float) Math.min(b.right, b.bottom));
        int dx=b.right-r;
        int dy=b.bottom-r;
        b.right=r;
        b.bottom=r;
        c.save();
        c.translate(dx/2,dy/2);
        RectF f = new RectF(r * 0.15f, r * 0.15f, r * 0.85f, r * 0.85f);
        if (n >= 360 && mState == STATE_LOADING) {
            sm = 8;
            sn = 0 - 6;
        } else if (n <= 6) {
            sn = 6;
            sm = 2;
        }
        if (n < 360 || mState == STATE_LOADING) {
            if (mState == STATE_LOADING) {
                n += sn;
                m += sm;
                m %= 360;
            } else {
                n += sn * 2;
                m += sm * 2;
                m %= 360;
            }
        }
        c.drawArc(f, m, n, false, p);

        if (n >= 360) {
            sn = -6;
            sm = 8;

            if (mState == STATE_SUCCESS) {
                Path path = new Path();
                path.moveTo(b.right * 0.3f, b.bottom * 0.5f);
                path.lineTo(b.right * 0.45f, b.bottom * 0.7f);
                path.lineTo(b.right * 0.75f, b.bottom * 0.4f);
                c.drawPath(path, p);
            } else if (mState == STATE_FAIL) {
                c.drawLine(b.right / 2, b.bottom * 0.25f, b.right / 2, b.bottom * 0.65f, p);
                c.drawLine(b.right / 2, b.bottom * 0.7f, b.right / 2, b.bottom * 0.75f, p);
            }
        }
        c.restore();
        invalidateSelf();
    }


    public void setStrokeWidth(float width){
        p.setStrokeWidth(width);
    }

    public void setColor(int p1) {

        p.setColor(p1);
    }

    @Override
    public void setAlpha(int p1) {

        p.setAlpha(p1);
    }

    @Override
    public void setColorFilter(ColorFilter p1) {

        p.setColorFilter(p1);
    }

    @Override
    public int getOpacity() {

        return PixelFormat.UNKNOWN;
    }
}

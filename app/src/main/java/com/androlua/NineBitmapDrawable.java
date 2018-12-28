package com.androlua;

import android.graphics.*;
import android.graphics.drawable.*;

import java.io.*;

public class NineBitmapDrawable extends Drawable implements LuaGcable {
    private Paint mPaint = new Paint();
    private Bitmap mBitmap;

    private int mX1;
    private int mY1;
    private int mX2;
    private int mY2;

    private Rect mRect1;
    private Rect mRect2;
    private Rect mRect3;

    private Rect mRect4;
    private Rect mRect5;
    private Rect mRect6;

    private Rect mRect7;
    private Rect mRect8;
    private Rect mRect9;
    private boolean mGc;

    public NineBitmapDrawable(String path) throws IOException {
        this(LuaBitmap.getLocalBitmap(path));
    }

    public NineBitmapDrawable(Bitmap bitmap) {
        int w = bitmap.getWidth();
        int h = bitmap.getHeight();
        int c = Color.BLACK;
        int x1 = 0;
        int x2 = 0;
        for (int i = 0; i < w; i++) {
            if (bitmap.getPixel(i, 0) == c) {
                x1 = i;
                break;
            }
        }
        if (x1 == 0 || x1 == w - 1)
            throw new IllegalArgumentException("not found x1");
        for (int i = x1; i < w; i++) {
            if (bitmap.getPixel(i, 0) != c) {
                x2 = w - i;
                break;
            }
        }
        if (x2 == 0 || x2 == 1)
            throw new IllegalArgumentException("not found x2");

        int y1 = 0;
        int y2 = 0;
        for (int i = 0; i < h; i++) {
            if (bitmap.getPixel(0, i) == c) {
                y1 = i;
                break;
            }
        }
        if (y1 == 0 || y1 == h - 1)
            throw new IllegalArgumentException("not found y1");
        for (int i = y1; i < h; i++) {
            if (bitmap.getPixel(0, i) != c) {
                y2 = h - i;
                break;
            }
        }
        if (y2 == 0 || y2 == 1)
            throw new IllegalArgumentException("not found y2");

        init(bitmap, x1, y1, x2, y2);
    }


    public NineBitmapDrawable(Bitmap bitmap, int x1, int y1, int x2, int y2) {
        init(bitmap, x1, y1, x2, y2);
    }

    private void init(Bitmap bitmap, int x1, int y1, int x2, int y2) {
        mBitmap = bitmap;
        int w = bitmap.getWidth();
        int h = bitmap.getHeight();

        mX1 = x1;
        mY1 = y1;
        mX2 = x2;
        mY2 = y2;

        x2 = w - x2;
        y2 = h - y2;
        mRect1 = new Rect(1, 1, x1, y1);
        mRect2 = new Rect(x1, 1, x2, y1);
        mRect3 = new Rect(x2, 1, w - 1, y1);

        mRect4 = new Rect(1, y1, x1, y2);
        mRect5 = new Rect(x1, y1, x2, y2);
        mRect6 = new Rect(x2, y1, w - 1, y2);

        mRect7 = new Rect(1, y2, x1, h - 1);
        mRect8 = new Rect(x1, y2, x2, h - 1);
        mRect9 = new Rect(x2, y2, w - 1, h - 1);
    }

    @Override
    public void draw(Canvas canvas) {
        // TODO: Implement this method
        Rect rect = getBounds();
        int w = rect.right;
        int h = rect.bottom;

        Rect rect1 = new Rect(0, 0, mX1, mY1);
        Rect rect2 = new Rect(mX1, 0, w - mX2, mY1);
        Rect rect3 = new Rect(w - mX2, 0, w, mY1);

        Rect rect4 = new Rect(0, mY1, mX1, h - mY2);
        Rect rect5 = new Rect(mX1, mY1, w - mX2, h - mY2);
        Rect rect6 = new Rect(w - mX2, mY1, w, h - mY2);

        Rect rect7 = new Rect(0, h - mY2, mX1, h);
        Rect rect8 = new Rect(mX1, h - mY2, w - mX2, h);
        Rect rect9 = new Rect(w - mX2, h - mY2, w, h);

        canvas.drawBitmap(mBitmap, mRect1, rect1, mPaint);
        canvas.drawBitmap(mBitmap, mRect2, rect2, mPaint);
        canvas.drawBitmap(mBitmap, mRect3, rect3, mPaint);

        canvas.drawBitmap(mBitmap, mRect4, rect4, mPaint);
        canvas.drawBitmap(mBitmap, mRect5, rect5, mPaint);
        canvas.drawBitmap(mBitmap, mRect6, rect6, mPaint);

        canvas.drawBitmap(mBitmap, mRect7, rect7, mPaint);
        canvas.drawBitmap(mBitmap, mRect8, rect8, mPaint);
        canvas.drawBitmap(mBitmap, mRect9, rect9, mPaint);
    }

    @Override
    public void setAlpha(int p1) {
        // TODO: Implement this method
        mPaint.setAlpha(p1);
    }

    @Override
    public void setColorFilter(ColorFilter p1) {
        // TODO: Implement this method
        mPaint.setColorFilter(p1);
    }

    @Override
    public int getOpacity() {
        // TODO: Implement this method
        return PixelFormat.UNKNOWN;
    }

    @Override
    public void gc() {
        try {
            mBitmap.recycle();
            mGc = true;
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    public boolean isGc() {
        return mGc;
    }
}

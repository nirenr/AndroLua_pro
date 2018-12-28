package com.androlua;

import android.graphics.Canvas;
import android.graphics.ColorFilter;
import android.graphics.Movie;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.graphics.Rect;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Handler;
import android.util.Log;

import com.androlua.util.AsyncTaskX;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;

/**
 * Created by nirenr on 2018/09/05 0005.
 */

public class LuaBitmapDrawable extends Drawable implements Runnable ,LuaGcable{

    private LuaContext mLuaContext;
    private int mDuration;
    private long mMovieStart;
    private int mCurrentAnimationTime;
    private Movie mMovie;
    private LoadingDrawable mLoadingDrawable;
    private Drawable mBitmapDrawable;
    private NineBitmapDrawable mNineBitmapDrawable;
    private ColorFilter mColorFilter;
    private int mFillColor;
    private int mScaleType = FIT_XY;
    private GifDecoder mGifDecoder;
    private GifDecoder mGifDecoder2;
    private Handler mHandler;
    private GifDecoder.GifFrame mGifFrame;
    private int mDelay;
    private boolean mGc;

    public static void setCacheTime(long time) {
        mCacheTime = time;
    }

    public static long getCacheTime() {
        return mCacheTime;
    }

    private static long mCacheTime = 7 * 24 * 60 * 60 * 1000;
    public LuaBitmapDrawable(LuaContext context, String path,Drawable def) {
        this(context, path);
        mBitmapDrawable=def;
    }

    public LuaBitmapDrawable(LuaContext context, String path) {
        mLuaContext = context;
        mLoadingDrawable = new LoadingDrawable(context.getContext());
        if (path.toLowerCase().startsWith("http://") || path.toLowerCase().startsWith("https://")) {
            initHttp(context, path);
        } else {
            if (!path.startsWith("/")) {
                path = context.getLuaPath(path);
            }
            init(path);
        }
    }

    private void initHttp(final LuaContext context, final String path) {
        new AsyncTaskX<String, String, String>() {
            @Override
            protected String doInBackground(String... strings) {
                try {
                    return getHttpBitmap(context, path);
                } catch (Exception e) {
                    e.printStackTrace();
                }
                return "";
            }

            @Override
            protected void onPostExecute(String s) {
                init(s);
            }
        }.execute();
    }

    private void init(final String path) {
        
        try {
            mGifDecoder = new GifDecoder(new FileInputStream(path), new GifDecoder.GifAction() {
                @Override
                public void parseOk(boolean parseStatus, int frameIndex) {
                    if (!parseStatus&&frameIndex < 0) {
                        init2(path);
                    } else if (parseStatus && mGifDecoder2 == null && mGifDecoder.getFrameCount() > 1) {     //当帧数大于1时，启动动画线程
                        mGifDecoder2 = mGifDecoder;
                    }
                    
                }
            });
            mGifDecoder.start();
        } catch (Exception e) {
            e.printStackTrace();
            init2(path);
        }
        
        
        

    }


    private void init2(String path) {
        if (path.isEmpty()) {
            new Handler().postDelayed(new Runnable() {
                @Override
                public void run() {
                    mLoadingDrawable.setState(-1);
                }
            }, 1000);
            invalidateSelf();
            return;
        }

        //mMovie = Movie.decodeFile(path);
        if (mMovie != null) {
            mDuration = mMovie.duration();
            if (mDuration == 0)
                mDuration = 1000;
        } else {
            try {
                mNineBitmapDrawable = new NineBitmapDrawable(path);
            } catch (Exception e) {
                try {
                    mBitmapDrawable = new BitmapDrawable(LuaBitmap.getLocalBitmap(mLuaContext, path));
                } catch (Exception e1) {
                    e1.printStackTrace();
                }
            }
        }
        if (mMovie == null && mBitmapDrawable == null && mNineBitmapDrawable == null) {
            new Handler().postDelayed(new Runnable() {
                @Override
                public void run() {
                    mLoadingDrawable.setState(-1);
                }
            }, 1000);
        }
        invalidateSelf();
        
        
        
    }

    @Override
    public int getIntrinsicWidth() {
        if (mMovie != null) {
            return mMovie.width();
        } else if (mBitmapDrawable != null) {
            mBitmapDrawable.getIntrinsicWidth();
        } else if (mNineBitmapDrawable != null) {
            mNineBitmapDrawable.getIntrinsicWidth();
        }
        return super.getIntrinsicWidth();
    }

    @Override
    public int getIntrinsicHeight() {
        if (mMovie != null) {
            return mMovie.height();
        } else if (mBitmapDrawable != null) {
            mBitmapDrawable.getIntrinsicHeight();
        } else if (mNineBitmapDrawable != null) {
            mNineBitmapDrawable.getIntrinsicHeight();
        }
        return super.getIntrinsicHeight();
    }

    @Override
    public void draw(Canvas canvas) {
        canvas.drawColor(mFillColor);
        if (mGifDecoder2 != null) {
            long now = System.currentTimeMillis();
            if (mMovieStart == 0 || mGifFrame == null) {
                mGifFrame = mGifDecoder2.next();
                mDelay = mGifFrame.delay;
                mMovieStart = now;
            } else {
                while (now - mMovieStart > mDelay) {
                    mGifFrame = mGifDecoder2.next();
                    mDelay = mGifFrame.delay;
                    mMovieStart += mDelay;
                }
            }
            if (mGifFrame != null) {
                Rect bound = getBounds();
                BitmapDrawable mBitmapDrawable = new BitmapDrawable(mGifFrame.image);
                int width = mBitmapDrawable.getIntrinsicWidth();
                int height = mBitmapDrawable.getIntrinsicHeight();
                float mScale = 1;
                if (mScaleType == FIT_XY) {
                    float mScaleX = (float) (bound.right - bound.left) / (float) width;
                    float mScaleY = (float) (bound.bottom - bound.top) / (float) height;
                    width = (int) (width * mScaleX);
                    height = (int) (height * mScaleY);
                } else if (mScaleType != MATRIX) {
                    mScale = Math.min((float) (bound.bottom - bound.top) / (float) height, (float) (bound.right - bound.left) / (float) width);
                    width = (int) (width * mScale);
                    height = (int) (height * mScale);
                }
                int left = bound.left;
                int top = bound.top;
                switch (mScaleType) {
                    case FIT_CENTER:
                        left = (int) (((bound.right - bound.left) - width) / 2);
                        top = (int) (((bound.bottom - bound.top) - height) / 2);
                        break;
                    case FIT_END:
                        top = (int) ((bound.bottom - bound.top) - height);
                        break;
                }
                //float mScale = Math.min((float) (bound.bottom - bound.top) / (float) mBitmapDrawable.getIntrinsicHeight(), (float) (bound.right - bound.left) / (float) mBitmapDrawable.getIntrinsicWidth());
                mBitmapDrawable.setBounds(new Rect(left, top, left + width, top + height));
                mBitmapDrawable.draw(canvas);

                // canvas.drawBitmap(mGifFrame.image, null, getBounds(), null);

            }
            invalidateSelf();
        } else if (mMovie != null) {
            long now = System.currentTimeMillis();
            if (mMovieStart == 0)
                mMovieStart = now;
            mCurrentAnimationTime = (int) ((now - mMovieStart) % mDuration);
            mMovie.setTime(mCurrentAnimationTime);
            Rect bound = getBounds();
            canvas.save();
            int width = mMovie.width();
            int height = mMovie.height();
            float mScale = 1;
            if (mScaleType == FIT_XY) {
                float mScaleX = (float) (bound.right - bound.left) / (float) width;
                float mScaleY = (float) (bound.bottom - bound.top) / (float) height;
                canvas.scale(mScaleX, mScaleY);
                width = (int) (width * mScaleX);
                height = (int) (height * mScaleY);
            } else if (mScaleType != MATRIX) {
                mScale = Math.min((float) (bound.bottom - bound.top) / (float) height, (float) (bound.right - bound.left) / (float) width);
                canvas.scale(mScale, mScale);
                width = (int) (width * mScale);
                height = (int) (height * mScale);
            }
            int left = bound.left;
            int top = bound.top;
            switch (mScaleType) {
                case FIT_CENTER:
                    left = (int) (((bound.right - bound.left) - width) / mScale / 2);
                    top = (int) (((bound.bottom - bound.top) - height) / mScale / 2);
                    break;
                case FIT_END:
                    top = (int) (((bound.bottom - bound.top)) - height / mScale);
                    break;
            }

            //
            //canvas.translate(left,top);
            
            mMovie.draw(canvas, left, top, new Paint());
            
            canvas.restore();
            invalidateSelf();

        } else if (mBitmapDrawable != null) {
            
            Rect bound = getBounds();
            int width = mBitmapDrawable.getIntrinsicWidth();
            int height = mBitmapDrawable.getIntrinsicHeight();
            float mScale = 1;
            if (mScaleType == FIT_XY) {
                float mScaleX = (float) (bound.right - bound.left) / (float) width;
                float mScaleY = (float) (bound.bottom - bound.top) / (float) height;
                width = (int) (width * mScaleX);
                height = (int) (height * mScaleY);
            } else if (mScaleType != MATRIX) {
                mScale = Math.min((float) (bound.bottom - bound.top) / (float) height, (float) (bound.right - bound.left) / (float) width);
                width = (int) (width * mScale);
                height = (int) (height * mScale);
            }
            int left = bound.left;
            int top = bound.top;
            switch (mScaleType) {
                case FIT_CENTER:
                    left = (int) (((bound.right - bound.left) - width) / 2);
                    top = (int) (((bound.bottom - bound.top) - height) / 2);
                    break;
                case FIT_END:
                    top = (int) ((bound.bottom - bound.top) - height);
                    break;
            }
            //float mScale = Math.min((float) (bound.bottom - bound.top) / (float) mBitmapDrawable.getIntrinsicHeight(), (float) (bound.right - bound.left) / (float) mBitmapDrawable.getIntrinsicWidth());
            mBitmapDrawable.setBounds(new Rect(left, top, left + width, top + height));
            mBitmapDrawable.draw(canvas);
            //canvas.drawBitmap(mBitmapDrawable.getBitmap(),getBounds(),getBounds(),new Paint());
        } else if (mNineBitmapDrawable != null) {
            
            mNineBitmapDrawable.setBounds(getBounds());
            mNineBitmapDrawable.draw(canvas);
        } else if (mLoadingDrawable != null) {
            mLoadingDrawable.setBounds(getBounds());
            mLoadingDrawable.draw(canvas);
            invalidateSelf();
        }
    }

    @Override
    protected void finalize() throws Throwable {
        if(mGifDecoder2!=null)
            mGifDecoder2.free();
    }

    public void setScaleType(int scaleType) {

        if (mScaleType != scaleType) {
            mScaleType = scaleType;
            invalidateSelf();
        }
    }

    public void setFillColor(int fillColor) {
        if (fillColor == mFillColor) {
            return;
        }
        mFillColor = fillColor;
    }

    @Override
    public void setAlpha(int alpha) {

    }

    @Override
    public void setColorFilter(ColorFilter colorFilter) {
        mColorFilter = colorFilter;
    }

    @Override
    public int getOpacity() {
        return PixelFormat.UNKNOWN;
    }

    public static String getHttpBitmap(LuaContext context, String url) throws IOException {
        //Log.d(TAG, url);
        String path = context.getLuaExtDir("cache") + "/" + url.hashCode();
        File f = new File(path);
        if (f.exists() && System.currentTimeMillis() - f.lastModified() < mCacheTime) {
            return path;
        }
        new File(path).delete();
        URL myFileUrl = new URL(url);
        HttpURLConnection conn = (HttpURLConnection) myFileUrl.openConnection();
        conn.setConnectTimeout(120000);
        conn.setDoInput(true);
        conn.connect();
        InputStream is = conn.getInputStream();
        FileOutputStream out = new FileOutputStream(path);
        if (!LuaUtil.copyFile(is, out)) {
            out.close();
            is.close();
            new File(path).delete();
            throw new RuntimeException("LoadHttpBitmap Error.");
        }
        out.close();
        is.close();
        return path;
    }

    public static final int MATRIX = (0);
    public static final int FIT_XY = (1);
    public static final int FIT_START = (2);
    public static final int FIT_CENTER = (3);
    public static final int FIT_END = (4);
    public static final int CENTER = (5);
    public static final int CENTER_CROP = (6);
    public static final int CENTER_INSIDE = (7);

    @Override
    public void run() {
        invalidateSelf();
    }

    @Override
    public void gc() {
        if(mGifDecoder2!=null)
            mGifDecoder2.free();
        if(mBitmapDrawable!=null&&mBitmapDrawable instanceof BitmapDrawable)
            ((BitmapDrawable)mBitmapDrawable).getBitmap().recycle();
        if(mNineBitmapDrawable!=null)
            mNineBitmapDrawable.gc();
        mGifDecoder2=null;
        mBitmapDrawable=null;
        mNineBitmapDrawable=null;
        mLoadingDrawable.setState(-1);
        mGc=true;
    }

    @Override
    public boolean isGc() {
        return mGc;
    }
}

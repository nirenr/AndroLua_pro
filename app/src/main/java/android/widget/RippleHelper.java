package android.widget;

import android.graphics.Canvas;
import android.graphics.ColorFilter;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.graphics.RadialGradient;
import android.graphics.Rect;
import android.graphics.Shader;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.Drawable;
import android.util.DisplayMetrics;
import android.util.TypedValue;
import android.view.MotionEvent;
import android.view.View;

import com.androlua.util.TimerTaskX;
import com.androlua.util.TimerX;

public class RippleHelper extends Drawable implements View.OnTouchListener {

    private final DisplayMetrics dm;
    private int state;

    private int mWidth;

    private int mStep;

    private boolean mCircle;
    private boolean mEnabled;
    private int mRadius;
    private TimerX mTimer;
    private RippleHelper.task mTask;
    private Paint mPaint2;
    private float mX;
    private float mY;
    private View mView;
    private Drawable mBackground;
    private int mAlpha;

    public boolean isSingle() {
        return mSingle;
    }

    public void setSingle(boolean mSingle) {
        this.mSingle = mSingle;
    }

    private boolean mSingle;
    private int mRippleLineColor;
    private int mRippleColor;


    public RippleHelper(View view) {
        mView = view;
        dm = view.getResources().getDisplayMetrics();
        init();
    }

    public boolean isCircle() {
        return mCircle;
    }

    public void setCircle(boolean circle) {
        mCircle = circle;
    }

    @Override
    public boolean onTouch(View p1, MotionEvent p2) {
        // TODO: Implement this method
        onTouchEvent(p2);
        return false;
    }

    private void init() {
        if (mView.isClickable())
            mEnabled = true;

        mBackground = mView.getBackground();
        mView.setBackgroundDrawable(this);
        mView.setOnTouchListener(this);
        mPaint2 = new Paint();
        mPaint2.setColor(0x44aaaaaa);
        mPaint2.setAntiAlias(true);
        mPaint2.setStrokeWidth(dp(4));
        mAlpha = mPaint2.getAlpha();
        mTimer = new TimerX();
        mTask = new task();
        mTimer.schedule(mTask, 0, 16);

        mTask.setEnabled(false);
        mRadius = 0;
    }

    public void onTouchEvent(MotionEvent event) {
        // TODO: Implement this method
        if (!mView.hasOnClickListeners() && !mEnabled)
            return;
        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                Rect rect = getBounds();
                if (mCircle) {
                    mY = rect.bottom / 2;
                    mX = rect.right / 2;
                    mWidth = Math.max(rect.bottom, rect.right) / 2;
                } else {
                    mX = event.getX();
                    mY = event.getY();
                    mWidth = (int) Math.hypot(rect.bottom, rect.right);
                }
                mStep = Math.max(mWidth / 60, 1);
                mRadius = 0;
                mTask.setEnabled(true);
                //mEnabled = true;
                mPaint2.setAlpha(mAlpha);
                state = 1;
                break;
            case MotionEvent.ACTION_CANCEL:
            case MotionEvent.ACTION_OUTSIDE:
            case MotionEvent.ACTION_UP:
                state = 2;
        }

    }

    private int dp(float n) {
        return (int) TypedValue.applyDimension(1, n, dm);
    }

    @Override
    public void draw(Canvas canvas) {
        if (mBackground != null) {
            mBackground.setBounds(getBounds());
            mBackground.draw(canvas);
        }
        mPaint2.setColor(mRippleColor);
        mPaint2.setAlpha(cAlpha);


        if (state != 0) {

            if (mCircle)
                canvas.drawCircle(mX, mY, mWidth, mPaint2);
            else
                canvas.drawRect(getBounds(), mPaint2);
            int w = mWidth;
            int n = 0;

            if (mSingle) {
                canvas.drawCircle(mX, mY, Math.min(mRadius, mWidth), mPaint2);
                return;
            }


            for (int r = mRadius; r >= 0; r = r - w) {
                canvas.drawCircle(mX, mY, Math.min(r, w), mPaint2);
                n++;
                if (n >= 2) {
                    mPaint2.setShader(new RadialGradient(mX, mY, dp(6), new int[]{0x44ffffff, mRippleLineColor, 0x44000000}, null, Shader.TileMode.MIRROR));
                    mPaint2.setStyle(Paint.Style.STROKE);
                    mPaint2.setColor(mRippleLineColor);
                    canvas.drawCircle(mX, mY, mRadius % w, mPaint2);
                    break;
                }
            }
            mPaint2.setShader(null);
            mPaint2.setStyle(Paint.Style.FILL);
        }
    }

    public void setBackgroundColor(int color) {
        // TODO: Implement this method
        mBackground = new ColorDrawable(color);
    }

    public void setRippleColor(int color) {
        // TODO: Implement this method
        mRippleColor = color;
        mPaint2.setColor(color);
        mAlpha = mPaint2.getAlpha();
    }

    public void setRippleLineColor(int color) {
        // TODO: Implement this method
        mRippleLineColor = color;
    }

    @Override
    public void setAlpha(int p1) {
        // TODO: Implement this method
        mAlpha = p1;
        mPaint2.setAlpha(p1);
    }

    @Override
    public void setColorFilter(ColorFilter p1) {
        // TODO: Implement this method
        mPaint2.setColorFilter(p1);
    }

    @Override
    public int getOpacity() {
        // TODO: Implement this method
        return PixelFormat.UNKNOWN;
    }

    private int cAlpha;

    private class task extends TimerTaskX {


        @Override
        public void run() {
            // TODO: Implement this method
            switch (state) {
                case 1:
                    if (mSingle)
                        mRadius += Math.max(mRadius / 16, mStep);
                    else
                        mRadius += mStep;
                    cAlpha = Math.min(mAlpha, mRadius / mStep);
                    mView.postInvalidate();
                    break;
                case 2:
                    mRadius += mStep * 4;
                    cAlpha = Math.min(mAlpha, mRadius / mStep * 2);
                    mView.postInvalidate();
                    if (mRadius / mWidth >= 1) {
                        mRadius = mWidth;
                        cAlpha = mAlpha;
                        state = 3;
                    }
                    break;
                case 3:
                    cAlpha -= Math.max(cAlpha / 16, 4);
                    mPaint2.setAlpha(cAlpha);

                    mView.postInvalidate();
                    if (cAlpha < 4) {
                        //cAlpha = mAlpha;
                        //mPaint2.setAlpha(mAlpha);
                        state = 0;
                    }
                    break;
                default:
                    mRadius = 0;
                    setEnabled(false);
            }
        }
    }

}

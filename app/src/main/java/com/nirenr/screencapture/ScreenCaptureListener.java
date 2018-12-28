package com.nirenr.screencapture;

import android.graphics.Bitmap;

/**
 * Created by Administrator on 2017/08/06 0006.
 */

public interface ScreenCaptureListener {
     public void onScreenCaptureDone(Bitmap bitmap);

     public void onScreenCaptureError(String msg);
}

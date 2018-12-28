package com.androlua.util;

import android.accessibilityservice.AccessibilityService;
import android.accessibilityservice.GestureDescription;
import android.annotation.TargetApi;
import android.graphics.Path;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.support.annotation.RequiresApi;
import android.util.Log;
import android.view.ViewConfiguration;

@RequiresApi(api = Build.VERSION_CODES.N)
public class GlobalActionAutomator {

    private AccessibilityService mService;
    private ScreenMetrics mScreenMetrics;
    private Handler mHandler;

    @TargetApi(Build.VERSION_CODES.N)
    public GlobalActionAutomator(AccessibilityService service,Handler handler) {
        mService = service;
        mHandler = handler;
    }

    public void setService(AccessibilityService service) {
        mService = service;
    }

    public void setScreenMetrics(ScreenMetrics screenMetrics) {
        mScreenMetrics = screenMetrics;
    }

    public boolean back() {
        return performGlobalAction(AccessibilityService.GLOBAL_ACTION_BACK);
    }

    public boolean home() {
        return performGlobalAction(AccessibilityService.GLOBAL_ACTION_HOME);
    }

    public boolean powerDialog() {
        return performGlobalAction(AccessibilityService.GLOBAL_ACTION_POWER_DIALOG);
    }

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
    private boolean performGlobalAction(int globalAction) {
        if (mService == null)
            return false;
        return mService.performGlobalAction(globalAction);
    }

    public boolean notifications() {
        return performGlobalAction(AccessibilityService.GLOBAL_ACTION_NOTIFICATIONS);
    }

    public boolean quickSettings() {
        return performGlobalAction(AccessibilityService.GLOBAL_ACTION_QUICK_SETTINGS);
    }

    public boolean recents() {
        return performGlobalAction(AccessibilityService.GLOBAL_ACTION_RECENTS);
    }

    public boolean splitScreen() {
        return performGlobalAction(AccessibilityService.GLOBAL_ACTION_TOGGLE_SPLIT_SCREEN);
    }

    public boolean gesture(long start, long duration, Path path) {
        return gestures(new GestureDescription.StrokeDescription(path, start, duration));
    }

    public boolean gesture(long start, long duration, int[]... points) {
        Path path = pointsToPath(points);
        return gestures(new GestureDescription.StrokeDescription(path, start, duration));
    }

    private Path pointsToPath(int[][] points) {
        Path path = new Path();
        path.moveTo(scaleX(points[0][0]), scaleY(points[0][1]));
        for (int i = 1; i < points.length; i++) {
            int[] point = points[i];
            path.lineTo(scaleX(point[0]), scaleY(point[1]));
        }
        return path;
    }

   public void gestureAsync(long start, long duration, int[]... points) {
        Path path = pointsToPath(points);
        gesturesAsync(new GestureDescription.StrokeDescription(path, start, duration));
    }

    public boolean gestures(GestureDescription.StrokeDescription... strokes) {
        if (mService == null)
            return false;
        GestureDescription.Builder builder = new GestureDescription.Builder();
        for (GestureDescription.StrokeDescription stroke : strokes) {
            builder.addStroke(stroke);
        }
        if (mHandler == null) {
            return gesturesWithoutHandler(builder.build());
        } else {
            return gesturesWithHandler(builder.build());
        }
    }

    private boolean gesturesWithHandler(GestureDescription description) {
        final VolatileDispose<Boolean> result = new VolatileDispose<>();
        Log.i("GlobalActionAutomator","dispatchGesture");
        return mService.dispatchGesture(description, new AccessibilityService.GestureResultCallback() {
            @Override
            public void onCompleted(GestureDescription gestureDescription) {
                Log.i("GlobalActionAutomator", "onCompleted");
                result.setAndNotify(true);
            }

            @Override
            public void onCancelled(GestureDescription gestureDescription) {
                Log.i("GlobalActionAutomator", "onCancelled");
                result.setAndNotify(false);
            }
        }, mHandler);
        //return result.blockedGet();
    }

    private boolean gesturesWithoutHandler(GestureDescription description) {
        prepareLooperIfNeeded();
        final VolatileBox<Boolean> result = new VolatileBox<>(false);
        Handler handler = new Handler(Looper.myLooper());
        mService.dispatchGesture(description, new AccessibilityService.GestureResultCallback() {
            @Override
            public void onCompleted(GestureDescription gestureDescription) {
                result.set(true);
                quitLoop();
            }

            @Override
            public void onCancelled(GestureDescription gestureDescription) {
                result.set(false);
                quitLoop();
            }
        }, handler);
        Looper.loop();
        return result.get();
    }

    public void gesturesAsync(GestureDescription.StrokeDescription... strokes) {
        if (mService == null)
            return;
        GestureDescription.Builder builder = new GestureDescription.Builder();
        for (GestureDescription.StrokeDescription stroke : strokes) {
            builder.addStroke(stroke);
        }
        mService.dispatchGesture(builder.build(), null, null);
    }

    private void quitLoop() {
        Looper looper = Looper.myLooper();
        if (looper != null) {
            looper.quit();
        }
    }

    private void prepareLooperIfNeeded() {
        if (Looper.myLooper() == null) {
            Looper.prepare();
        }
    }

    public boolean click(int x, int y) {
        return press(x, y, ViewConfiguration.getTapTimeout() );
    }

    public boolean press(int x, int y, int delay) {
        return gesture(0, delay, new int[]{x, y});
    }

    public boolean longClick(int x, int y) {
        return gesture(0, ViewConfiguration.getLongPressTimeout() + 200, new int[]{x, y});
    }

    private int scaleX(int x) {
        if (mScreenMetrics == null)
            return x;
        return mScreenMetrics.scaleX(x);
    }

    private int scaleY(int y) {
        if (mScreenMetrics == null)
            return y;
        return mScreenMetrics.scaleY(y);
    }

    public boolean swipe(int x1, int y1, int x2, int y2, int delay) {
        return gesture(0, delay, new int[]{x1, y1}, new int[]{x2, y2});
    }

}

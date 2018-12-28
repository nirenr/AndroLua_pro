package com.androlua.util;

import android.util.Log;
import android.view.accessibility.AccessibilityNodeInfo;

import com.androlua.LuaAccessibilityService;
import com.luajava.LuaTable;

/**
 * Created by Administrator on 2017/08/31 0031.
 */

public class ClickRunnable implements Runnable {
    private final LuaAccessibilityService mService;
    private LuaTable mButtons;
    private int mIdx = 1;
    private int mN = -1;
    private int mM = -1;
    private ClickCallback mClickCallback;
    private boolean mIsCancel = false;
    private ClickRunnable mClick;

    public ClickRunnable(LuaAccessibilityService service, LuaTable buttons) {
        mService = service;
        mButtons = buttons;
    }

    public void cancel() {
        mIsCancel = true;
        if (mClick != null)
            mClick.cancel();
    }

    public boolean canClick(ClickCallback callback) {
        mClickCallback = callback;
        return canClick();
    }

    public boolean canClick() {
        if (mButtons.length() == 0)
            return false;
        int size = mButtons.length();
        for (int i = 0; i < size; i++) {
            if (mIsCancel) {
                if (mClickCallback != null)
                    mClickCallback.onDone(false, mButtons, null, -1);
                return false;
            }
            Object obj = mButtons.get(i + 1);
            if (obj instanceof LuaTable) {
                LuaTable bs = (LuaTable) obj;
                if (bs.length() == 0)
                    continue;
                String name = (String) bs.get(1);
                if (name == null)
                    continue;
                if (postClick(name)) {
                    mButtons = bs;
                    return true;
                }
            } else if (obj instanceof String) {
                String name = (String) obj;
                AccessibilityNodeInfo node = mService.findAccessibilityNodeInfo(name);
                if (node != null) {
                    mService.toClick2(node);
                    if (mClickCallback != null)
                        mClickCallback.onDone(true, mButtons, name, i);
                    return true;
                }
            }
        }
        if (mClickCallback != null)
            mClickCallback.onDone(false, mButtons, null, -1);
        return false;
    }

    private boolean postClick(String name) {
        if (name == null)
            return false;
        long time = 1000;
        int idx = name.lastIndexOf("$");
        if (idx > 0) {
            try {
                time = Long.valueOf(name.substring(idx + 1));
            } catch (Exception e) {
                time = 1000;
            }
            name = name.substring(0, idx);
        }
        idx = name.lastIndexOf(">");
        if (idx > 0) {
            if (mN < 0) {
                try {
                    mN = Integer.valueOf(name.substring(idx + 1));
                } catch (Exception e) {
                    mN = -1;
                }
            }
            name = name.substring(0, idx);
        }
        idx = name.lastIndexOf("<");
        if (idx > 0) {
            if (mM < 0) {
                try {
                    mM = Integer.valueOf(name.substring(idx + 1));
                } catch (Exception e) {
                    mM = -1;
                }
            }
            name = name.substring(0, idx);
        }
        mM--;
        mN--;
        AccessibilityNodeInfo node = mService.findAccessibilityNodeInfo(name);
        Log.i("lua", "findAccessibilityNodeInfo " + name + "," + mN + "," + mM + "," + node);

        if (node != null) {
            mN = -1;
            mService.toClick2(node);
            mService.getHandler().postDelayed(this, time);
            return true;
        } else if (mN > 0 || mM > 0) {
            mService.getHandler().postDelayed(this, time);
            return true;
        }
        if (mClickCallback != null)
            mClickCallback.onDone(true, mButtons, name, mIdx);
        return false;
    }

    @Override
    public void run() {
        if (mIsCancel) {
            if (mClickCallback != null)
                mClickCallback.onDone(false, mButtons, null, -1);
            return;
        }
        if (mN < 0 && mM < 0)
            mIdx++;
        Object obj = mButtons.get(mIdx);
        if (obj == null) {
            if (mClickCallback != null)
                mClickCallback.onDone(mIdx == mButtons.length(), mButtons, null, mIdx);
            return;
        }
        if (obj instanceof LuaTable) {
            LuaTable bs = (LuaTable) obj;
            if (bs.length() == 0)
                return;
            mClick = new ClickRunnable(mService, bs);
            mClick.canClick(new ClickCallback() {
                @Override
                public void onDone(boolean bool, LuaTable bs, String name, int idx) {
                    mClick = null;
                    run();
                }
            });

            /*String name = (String) bs.get(1);
            if (name == null)
                return;
            if (postClick(name)) {
                mIdx = 1;
                mM = -1;
                mN = -1;
                mButtons = bs;
            }*/
        } else if (obj instanceof String) {
            String name = (String) obj;
            postClick(name);
        }
    }


    public static interface ClickCallback {
        public void onDone(boolean bool, LuaTable bs, String name, int idx);
    }
}

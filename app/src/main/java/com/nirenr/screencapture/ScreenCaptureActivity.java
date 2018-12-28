package com.nirenr.screencapture;

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.media.projection.MediaProjectionManager;
import android.os.Build;
import android.os.Bundle;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;

@TargetApi(Build.VERSION_CODES.LOLLIPOP)
public class ScreenCaptureActivity extends Activity {

    public static final int REQUEST_MEDIA_PROJECTION = 18;
    private TextView view;
    private ArrayList<String> permissions;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        view = new TextView(this);
        view.setText("请授予权限");
        setContentView(view);
        requesturePermission();
    }

    public void requesturePermission() {

        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
            //5.0 之后才允许使用屏幕截图
            Toast.makeText(this, "仅支持安卓5以上系统", Toast.LENGTH_SHORT).show();
            //TalkManAccessibilityService.getInstance().toBack();
            return;
        }
        try {
            MediaProjectionManager mediaProjectionManager = (MediaProjectionManager)
                    getSystemService(Context.MEDIA_PROJECTION_SERVICE);
            startActivityForResult(
                    mediaProjectionManager.createScreenCaptureIntent(),
                    REQUEST_MEDIA_PROJECTION);
        } catch (Exception e) {
            e.printStackTrace();
            ScreenShot.setResultData(null);
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        switch (requestCode) {
            case REQUEST_MEDIA_PROJECTION:
                if (resultCode == RESULT_OK && data != null) {
                    ScreenShot.setResultData(data);
                    //Toast.makeText(this,"获得权限成功",Toast.LENGTH_SHORT).show();
                }
                break;
            default:
                ScreenShot.setResultData(null);
        }
        finish();
    }

    @Override
    public void finish() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            finishAndRemoveTask();
        } else {
            super.finish();
        }
    }
}

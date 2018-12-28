package com.nirenr.screencapture;

import android.content.Context;
import android.os.Environment;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

/**
 * Created by ryze on 2016-5-26.
 */
public class FileUtil {

  //系统保存截图的路径
  public static final String SCREENCAPTURE_PATH = "ScreenCapture" + File.separator + "Screenshots" + File.separator;
//  public static final String SCREENCAPTURE_PATH = "ZAKER" + File.separator + "Screenshots" + File.separator;

  public static final String SCREENSHOT_NAME = "Screenshot";

  public static String getAppPath(Context context) {

    if (Environment.MEDIA_MOUNTED.equals(Environment.getExternalStorageState())) {


      return Environment.getExternalStorageDirectory().toString();

    } else {

      return context.getFilesDir().toString();
    }

  }


  public static String getScreenShots(Context context) {

    StringBuffer stringBuffer = new StringBuffer(getAppPath(context));
    stringBuffer.append(File.separator);

    stringBuffer.append(SCREENCAPTURE_PATH);

    File file = new File(stringBuffer.toString());

    if (!file.exists()) {
      file.mkdirs();
    }

    return stringBuffer.toString();

  }

  public static String getScreenShotsName(Context context) {

    SimpleDateFormat simpleDateFormat = new SimpleDateFormat("yyyy-MM-dd-hh-mm-ss", Locale.CHINESE);

    String date = simpleDateFormat.format(new Date());

    StringBuffer stringBuffer = new StringBuffer(getScreenShots(context));
    stringBuffer.append(SCREENSHOT_NAME);
    stringBuffer.append("_");
    stringBuffer.append(date);
    stringBuffer.append(".png");

    return stringBuffer.toString();

  }


}

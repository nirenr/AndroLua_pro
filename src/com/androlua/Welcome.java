package com.androlua;

import android.app.*;
import android.content.*;
import android.os.*;
import com.androlua.*;


public class Welcome extends Activity {

    @Override
    public void onCreate(Bundle savedInstanceState) {
		
        super.onCreate(savedInstanceState);

        /**全屏设置，隐藏窗口所有装饰
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        标题是属于View的，所以窗口所有的修饰部分被隐藏后标题依然有效,需要去掉标题
        requestWindowFeature(Window.FEATURE_NO_TITLE);
*/
        //setContentView(R.layout.welcome);
        Handler handler = new Handler();
        //使用pastDelayed方法延时
        handler.postDelayed(new Runnable() {
          @Override
          public void run() {
            Intent intent = new Intent(Welcome.this, Main.class);
            startActivityForResult(intent, 1);
                		
            //添加界面切换效果
            overridePendingTransition(android.R.anim.slide_in_left,android.R.anim.slide_out_right);
            
            //结束欢迎界面
            Welcome.this.finish();
          }
        }, 500);
      }
}

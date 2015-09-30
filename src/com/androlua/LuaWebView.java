package com.androlua;

import android.content.*;
import android.view.*;
import android.webkit.*;
import android.view.View.*;

public class LuaWebView extends WebView
{

	public LuaWebView(Context context)
	{
		super(context);

		getSettings().setJavaScriptEnabled(true);  
		requestFocus();
		setWebViewClient(new WebViewClient()
			{
				public boolean shouldOverrideUrlLoading(WebView view, String url)
				{
					view.loadUrl(url);  
					return true;
				}
				public void onPageFinished(WebView view, String url)
				{

					//view.loadUrl("file:///android_asset/a.js");
				} 
			}
		);
	}

	public boolean onKeyDown(int keyCode, KeyEvent event) {       
		if ((keyCode == KeyEvent.KEYCODE_BACK) && canGoBack()) {       
            goBack();       
			return true;       
        }       
		return super.onKeyDown(keyCode, event);       
    }

	@Override
	public void setOnKeyListener(View.OnKeyListener l)
	{
		// TODO: Implement this method
		super.setOnKeyListener(l);
	}
	
}

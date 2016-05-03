package com.androlua;

import java.io.PushbackInputStream;
import android.speech.tts.TextToSpeech;
import android.content.Context;
import java.util.Locale;
public class TTS implements TextToSpeech.OnInitListener
{
	Context context;
	private TextToSpeech mTis;
	public TTS(Context c)
	{ 
		this.context = c;
		mTis = new TextToSpeech(context, this);
	} 

	@Override
	public void onInit(int p1)
	{ 
		if (p1 == TextToSpeech.SUCCESS)
		{
			int result=mTis.setLanguage(Locale.CHINESE);
		}
	}
	//播放 
	public void Du(String s)
	{
		mTis.speak(s, TextToSpeech.QUEUE_FLUSH, null);
	}
}

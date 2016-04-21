package com.myopicmobile.textwarrior.android;

public interface TextChangeListener
{

	public void onNewLine(String c, int _caretPosition, int p2);


	public void onDel(CharSequence text, int _caretPosition, int newCursorPosition);

	public void onAdd(CharSequence text, int _caretPosition, int newCursorPosition);

}

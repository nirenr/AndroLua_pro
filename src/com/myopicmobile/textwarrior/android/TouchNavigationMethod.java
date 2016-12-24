/*
 * Copyright (c) 2013 Tah Wei Hoon.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License Version 2.0,
 * with full text available at http://www.apache.org/licenses/LICENSE-2.0.html
 *
 * This software is provided "as is". Use at your own risk.
 */
package com.myopicmobile.textwarrior.android;

import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.preference.PreferenceManager;
import android.view.GestureDetector;
import android.view.HapticFeedbackConstants;
import android.view.KeyEvent;
import android.view.MotionEvent;

import com.myopicmobile.textwarrior.common.ColorScheme;
import com.myopicmobile.textwarrior.common.*;
import android.util.*;

//TODO minimise unnecessary invalidate calls
/**
 * TouchNavigationMethod classes implementing their own carets have to override
 * getCaretBloat() to return the size of the drawing area it needs, in excess of
 * the bounding box of the character the caret is on, and use
 * onTextDrawComplete(Canvas) to draw the caret. Currently, only a fixed size
 * caret is allowed, but scalable carets may be implemented in future.
 */
public class TouchNavigationMethod extends GestureDetector.SimpleOnGestureListener
{
	protected FreeScrollingTextField _textField;
	private GestureDetector _gestureDetector;
	protected boolean _isCaretTouched = false;

	private float lastDist;

	private float lastX;

	private float lastY;

	private float lastSize;

	private int fling;
	
	public TouchNavigationMethod(FreeScrollingTextField textField)
	{
		_textField = textField;
		_gestureDetector = new GestureDetector(textField.getContext(), this);
		_gestureDetector.setIsLongpressEnabled(true);
	}


	@Override
	public boolean onDown(MotionEvent e)
	{
		int x = screenToViewX((int) e.getX());
		int y = screenToViewY((int) e.getY());
		_isCaretTouched = isNearChar(x, y, _textField.getCaretPosition());

		if (_textField.isFlingScrolling())
		{
			_textField.stopFlingScrolling();
		}
		else if (_textField.isSelectText())
		{
			if (isNearChar(x, y, _textField.getSelectionStart()))
			{
				_textField.focusSelectionStart();
				_textField.performHapticFeedback(HapticFeedbackConstants.LONG_PRESS);
				_isCaretTouched = true;
			}
			else if (isNearChar(x, y, _textField.getSelectionEnd()))
			{
				_textField.focusSelectionEnd();
				_textField.performHapticFeedback(HapticFeedbackConstants.LONG_PRESS);
				_isCaretTouched = true;
			}
		}

		if (_isCaretTouched)
		{
			_textField.performHapticFeedback(HapticFeedbackConstants.LONG_PRESS);
		}

		return true;
	}

	@Override
	public void onShowPress(MotionEvent e) {

	}

	@Override
	public boolean onSingleTapUp(MotionEvent e){
		int x = screenToViewX((int) e.getX());
	int y = screenToViewY((int) e.getY());
	int charOffset = _textField.coordToCharIndex(x, y);

	if (_textField.isSelectText())
	{
		int strictCharOffset = _textField.coordToCharIndexStrict(x, y);
		if (_textField.inSelectionRange(strictCharOffset) ||
				isNearChar(x, y, _textField.getSelectionStart()) ||
				isNearChar(x, y, _textField.getSelectionEnd()))
		{
			// do nothing
		}
		else
		{
			_textField.selectText(false);
			if (strictCharOffset >= 0)
			{
				_textField.moveCaret(charOffset);
			}
		}
	}
	else
	{
		if (charOffset >= 0)
		{
			_textField.moveCaret(charOffset);
		}
	}
	boolean displayIME = true;
	if (displayIME)
	{
		_textField.showIME(true);
	}
	return true;
}

	/**
	 * Note that up events from a fling are NOT captured here.
	 * Subclasses have to call super.onUp(MotionEvent) in their implementations
	 * of onFling().
	 *
	 * Also, up events from non-primary pointers in a multi-touch situation are
	 * not captured here.
	 *
	 * @param e
	 * @return
	 */
	public boolean onUp(MotionEvent e)
	{
		_textField.stopAutoScrollCaret();
		_isCaretTouched = false;
		lastDist=0;
		fling=0;
		return true;
	}

	@Override
	public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX,
							float distanceY)
	{
		
		//onTouchZoon(e2);
			
		if (_isCaretTouched)
		{
			dragCaret(e2);
		}
		else if (e2.getPointerCount() == 1)
		{
			if(fling==0)
				if(Math.abs(distanceX)>Math.abs(distanceY))
					fling=1;
				else
					fling=-1;
			if(fling==1)
				distanceY=0;
			else if(fling==-1)
				distanceX=0;
			
			scrollView(distanceX, distanceY);
			//_textField.smoothScrollBy((int)distanceX, (int)distanceY);
			
		}

		//TODO find out if ACTION_UP events are actually passed to onScroll
		if ((e2.getAction() & MotionEvent.ACTION_MASK) == MotionEvent.ACTION_UP)
		{
			onUp(e2);
		}
		return true;
	}

	// When the caret is dragged to the edges of the text field, the field will
	// scroll automatically. SCROLL_EDGE_SLOP is the width of these edges in pixels
	// and extends inside the content area, not outside to the padding area
	protected static int SCROLL_EDGE_SLOP = 10;
	private void dragCaret(MotionEvent e)
	{
		if (!_textField.isSelectText() && isDragSelect())
		{
			_textField.selectText(true);
		}

		int x = (int) e.getX() - _textField.getPaddingLeft();
		int y = (int) e.getY() - _textField.getPaddingTop();
		boolean scrolled = false;

		// If the edges of the textField content area are touched, scroll in the
		// corresponding direction.
		if (x < SCROLL_EDGE_SLOP)
		{
			scrolled = _textField.autoScrollCaret(FreeScrollingTextField.SCROLL_LEFT);
		}
		else if (x >= (_textField.getContentWidth() - SCROLL_EDGE_SLOP))
		{
			scrolled = _textField.autoScrollCaret(FreeScrollingTextField.SCROLL_RIGHT);
		}
		else if (y < SCROLL_EDGE_SLOP)
		{
			scrolled = _textField.autoScrollCaret(FreeScrollingTextField.SCROLL_UP);
		}
		else if (y >= (_textField.getContentHeight() - SCROLL_EDGE_SLOP))
		{
			scrolled = _textField.autoScrollCaret(FreeScrollingTextField.SCROLL_DOWN);
		}

		if (!scrolled)
		{
			_textField.stopAutoScrollCaret();
			int newCaretIndex = _textField.coordToCharIndex(
				screenToViewX((int) e.getX()),
				screenToViewY((int) e.getY())
			);
			if (newCaretIndex >= 0)
			{
				_textField.moveCaret(newCaretIndex);
			}
		}
	}

	private void scrollView(float distanceX, float distanceY)
	{
		int newX = (int) distanceX + _textField.getScrollX();
		int newY = (int) distanceY + _textField.getScrollY();

		// If scrollX and scrollY are somehow more than the recommended
		// max scroll values, use them as the new maximum
		// Also take into account the size of the caret,
		// which may extend beyond the text boundaries
		int maxWidth = Math.max(_textField.getMaxScrollX(),
								_textField.getScrollX());
		if (newX > maxWidth)
		{
			newX = maxWidth;
		}
		else if (newX < 0)
		{
			newX = 0;
		}

		int maxHeight = Math.max(_textField.getMaxScrollY(),
								 _textField.getScrollY());
		if (newY > maxHeight)
		{
			newY = maxHeight;
		}
		else if (newY < 0)
		{
			newY = 0;
		}
		//_textField.scrollTo(newX, newY);
		_textField.smoothScrollTo(newX, newY);
		
	}


	@Override
	public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX,
						   float velocityY)
	{
		if (!_isCaretTouched)
		{
			
			if(fling==1)
				velocityY=0;
			else if(fling==-1)
				velocityX=0;
			
			_textField.flingScroll((int) -velocityX, (int) -velocityY);
		}
		onUp(e2);
		return true;
	}

	private float spacing(MotionEvent event)
	{ 
		float x = event.getX(0) - event.getX(1); 
		float y = event.getY(0) - event.getY(1); 
		return (float)Math.sqrt(x * x + y * y); 
	}  

	private boolean onTouchZoon(MotionEvent e)
	{
		if (e.getAction() == MotionEvent.ACTION_MOVE)
		{
			if (e.getPointerCount() == 2)
			{
				if(lastDist==0){
					float x = e.getX(0) - e.getX(1); 
					float y = e.getY(0) - e.getY(1); 
					lastDist=(float) Math.sqrt(x * x + y * y);
					lastX = (e.getX(0) + e.getX(1)) / 2; 
					lastY = (e.getY(0) + e.getY(1)) / 2; 
					lastSize=_textField.getTextSize();
				}
				
				float dist=spacing(e);
				if (lastDist != 0){
					_textField.setTextSize((int)(lastSize*(dist / lastDist)));
					//_textField.scrollBy(0,(int)(lastY-lastY*(_textField.getTextSize() / lastSize)));
				}
				//_textField.setTextSize((int)(_textField.getTextSize() * (dist / lastDist)));
				//lastDist = dist;
				return true;
			}
		}
		lastDist = 0;
		return false;
	}

	/**
	 * Subclasses overriding this method have to call the superclass method
	 */
	public boolean onTouchEvent(MotionEvent event)
	{
		onTouchZoon(event);
		boolean handled = _gestureDetector.onTouchEvent(event);
		if (!handled
			&& (event.getAction() & MotionEvent.ACTION_MASK) == MotionEvent.ACTION_UP)
		{
			// propagate up events since GestureDetector does not do so
			handled = onUp(event);
		}
		return handled;
	}

	@Override
	public void onLongPress(MotionEvent e)
	{
		onDoubleTap(e);
	}

	@Override
	public boolean onDoubleTap(MotionEvent e) {
		// TODO: Implement this method
		// TODO: Implement this method
		_isCaretTouched = true;
		int x = screenToViewX((int) e.getX());
		int y = screenToViewY((int) e.getY());
		int charOffset = _textField.coordToCharIndex(x, y);

		/*if (_textField.isSelectText())
		 {
		 if (_textField.inSelectionRange(charOffset))
		 {
		 _textField.moveCaret(charOffset);
		 _textField.selectText(false);
		 }
		 else if (charOffset >= 0)
		 {
		 _textField.setSelectionRange(charOffset, 0);
		 }
		 else
		 {
		 _textField.selectText(false);
		 _isCaretTouched = false;
		 }
		 }
		 else*/
		{
			if (charOffset >= 0)
			{
				_textField.moveCaret(charOffset);
				DocumentProvider doc=_textField.createDocumentProvider();
				int start;int end;
				for (start = charOffset;start >= 0;start--)
				{
					char c=doc.charAt(start);
					if(!Character.isJavaIdentifierPart(c))
						break;
				}
				if (start != charOffset)
					start++;
				for (end = charOffset;end >= 0;end++)
				{
					char c=doc.charAt(end);
					if(!Character.isJavaIdentifierPart(c))
						break;
				}
				_textField.selectText(true);
				_textField.setSelectionRange(start, end - start);
			}
		}
		return true;
	}



	public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		return false;
	}

	public boolean onKeyUp(int keyCode, KeyEvent event)
	{
		return false;
	}

	/**
	 * Android lifecyle event. See {@link android.app.Activity#onPause()}.
	 */
	void onPause()
	{
		//do nothing
	}

	/**
	 * Android lifecyle event. See {@link android.app.Activity#onResume()}.
	 */
	void onResume()
	{
		//do nothing
	}

	/**
	 * Called by FreeScrollingTextField when it has finished drawing text.
	 * Classes extending TouchNavigationMethod can use this to draw, for
	 * example, a custom caret.
	 *
	 * The canvas includes padding in it.
	 *
	 * @param canvas
	 */
	public void onTextDrawComplete(Canvas canvas)
	{
		// Do nothing. Basic caret drawing is handled by FreeScrollingTextField.
	}

	public void onColorSchemeChanged(ColorScheme colorScheme)
	{
		// Do nothing. Derived classes can use this to change their graphic assets accordingly.
	}

	public void onChiralityChanged(boolean isRightHanded)
	{
		// Do nothing. Derived classes can use this to change their input
		// handling and graphic assets accordingly.
	}

	private final static Rect _caretBloat = new Rect(0, 0, 0, 0);
	/**
	 * For any printed character, this method returns the amount of space
	 * required in excess of the bounding box of the character to draw the
	 * caret.
	 * Subclasses should override this method if they are drawing their
	 * own carets.
	 */
	public Rect getCaretBloat()
	{
		return _caretBloat;
	}


	//*********************************************************************
    //**************************** Utilities ******************************
    //*********************************************************************

	final protected int getPointerId(MotionEvent e)
	{
		return (e.getAction() & MotionEvent.ACTION_POINTER_ID_MASK)
			>> MotionEvent.ACTION_POINTER_ID_SHIFT;
	}

	/**
	 * Converts a x-coordinate from screen coordinates to local coordinates,
	 * excluding padding
	 *
	 */
	final protected int screenToViewX(int x)
	{
		return x - _textField.getPaddingLeft() + _textField.getScrollX();
	}

	/**
	 * Converts a y-coordinate from screen coordinates to local coordinates,
	 * excluding padding
	 *
	 */
	final protected int screenToViewY(int y)
	{
		return y - _textField.getPaddingTop() + _textField.getScrollY();
	}

	final public boolean isRightHanded()
	{
		return true;
	}

	final private boolean isDragSelect()
	{
		return false;
	}


	/**
	 * The radius, in density-independent pixels, around a point of interest
	 * where any touch event within that radius is considered to have touched
	 * the point of interest itself
	 */
	protected static int TOUCH_SLOP = 12;

	/**
	 * Determine if a point(x,y) on screen is near a character of interest,
	 * specified by its index charOffset. The radius of proximity is defined
	 * by TOUCH_SLOP.
	 *
	 * @param x X-coordinate excluding padding
	 * @param y Y-coordinate excluding padding
	 * @param charOffset the character of interest
	 * @return Whether (x,y) lies close to the character with index charOffset
	 */
	public boolean isNearChar(int x, int y, int charOffset)
	{
		Rect bounds = _textField.getBoundingBox(charOffset);

		return (y >= (bounds.top - TOUCH_SLOP)
			&& y < (bounds.bottom + TOUCH_SLOP)
			&& x >= (bounds.left - TOUCH_SLOP)
			&& x < (bounds.right + TOUCH_SLOP)
			);
	}

	@SuppressWarnings("unused")
	private TouchNavigationMethod()
	{
		// do not invoke; always needs a valid _textField
	}
}

/*
 * Copyright (c) 2013 Tah Wei Hoon.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License Version 2.0,
 * with full text available at http://www.apache.org/licenses/LICENSE-2.0.html
 *
 * This software is provided "as is". Use at your own risk.
 */

package com.myopicmobile.textwarrior.android;

import android.graphics.*;
import android.util.*;
import android.view.*;
import com.myopicmobile.textwarrior.common.*;
import com.myopicmobile.textwarrior.common.ColorScheme.Colorable;
import com.myopicmobile.textwarrior.common.Pair;

public class YoyoNavigationMethod extends TouchNavigationMethod
{
	private final Yoyo _yoyoCaret;
	private final Yoyo _yoyoStart;
	private final Yoyo _yoyoEnd;

	private boolean _isStartHandleTouched = false;
	private boolean _isEndHandleTouched = false;
	private boolean _isCaretHandleTouched = false;
	private boolean _isShowYoyoCaret = false;

	private int _yoyoSize;

	public YoyoNavigationMethod(FreeScrollingTextField textField)
	{
		super(textField);
		DisplayMetrics dm=textField.getContext().getResources().getDisplayMetrics();
		_yoyoSize = (int) TypedValue.applyDimension(2, (float)(textField.BASE_TEXT_SIZE_PIXELS * 1.5), dm);
		_yoyoCaret = new Yoyo();
		_yoyoStart = new Yoyo();
		_yoyoEnd = new Yoyo();
	}

	@Override
	public boolean onDown(MotionEvent e)
	{
		super.onDown(e);
		if (!_isCaretTouched)
		{
			int x = (int) e.getX() + _textField.getScrollX();
			int y = (int) e.getY() + _textField.getScrollY();
			_isCaretHandleTouched = _yoyoCaret.isInHandle(x, y);
			_isStartHandleTouched = _yoyoStart.isInHandle(x, y);
			_isEndHandleTouched = _yoyoEnd.isInHandle(x, y);

			if (_isCaretHandleTouched)
			{
				_isShowYoyoCaret = true;
				_yoyoCaret.setInitialTouch(x, y);
				_yoyoCaret.invalidateHandle();
			}
			else if (_isStartHandleTouched)
			{
				_yoyoStart.setInitialTouch(x, y);
				_textField.focusSelectionStart();
				_yoyoStart.invalidateHandle();
			}
			else if (_isEndHandleTouched)
			{
				_yoyoEnd.setInitialTouch(x, y);
				_textField.focusSelectionEnd();
				_yoyoEnd.invalidateHandle();
			}
		}

		return true;
	}

	@Override
	public boolean onUp(MotionEvent e)
	{
		_isCaretHandleTouched = false;
		_isStartHandleTouched = false;
		_isEndHandleTouched = false;
		_yoyoCaret.clearInitialTouch();
		_yoyoStart.clearInitialTouch();
		_yoyoEnd.clearInitialTouch();
		super.onUp(e);
		return true;
	}

	@Override
	public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX,
							float distanceY)
	{

		if (_isCaretHandleTouched)
		{
			//TODO find out if ACTION_UP events are actually passed to onScroll
			if ((e2.getAction() & MotionEvent.ACTION_MASK) == MotionEvent.ACTION_UP)
			{
				onUp(e2);
			}
			else
			{	
				_isShowYoyoCaret = true;
				moveHandle(_yoyoCaret, e2);
			}

			return true;
		}
		else if (_isStartHandleTouched)
		{
			//TODO find out if ACTION_UP events are actually passed to onScroll
			if ((e2.getAction() & MotionEvent.ACTION_MASK) == MotionEvent.ACTION_UP)
			{
				onUp(e2);
			}
			else
			{
				moveHandle(_yoyoStart, e2);
			}

			return true;
		}
		else if (_isEndHandleTouched)
		{
			//TODO find out if ACTION_UP events are actually passed to onScroll
			if ((e2.getAction() & MotionEvent.ACTION_MASK) == MotionEvent.ACTION_UP)
			{
				onUp(e2);
			}
			else
			{
				moveHandle(_yoyoEnd, e2);
			}

			return true;
		}
		else
		{
			return super.onScroll(e1, e2, distanceX, distanceY);
		}
	}

	private void moveHandle(Yoyo _yoyo, MotionEvent e)
	{

		Pair foundIndex = _yoyo.findNearestChar((int) e.getX(), (int) e.getY());
		int newCaretIndex = foundIndex.getFirst();

		if (newCaretIndex >= 0)
		{
			_textField.moveCaret(newCaretIndex);
			//snap the handle to the caret
			Rect newCaretBounds = _textField.getBoundingBox(newCaretIndex);
			int newX = newCaretBounds.left + _textField.getPaddingLeft();
			int newY = newCaretBounds.bottom + _textField.getPaddingTop();

			_yoyo.attachYoyo(newX, newY);
		}

	}


	@Override
	public boolean onSingleTapConfirmed(MotionEvent e)
	{
		int x = (int) e.getX() + _textField.getScrollX();
		int y = (int) e.getY() + _textField.getScrollY();

		//ignore taps on handle
		if (_yoyoCaret.isInHandle(x, y) || _yoyoStart.isInHandle(x, y) || _yoyoEnd.isInHandle(x, y))
		{
			return true;
		}
		else
		{
			_isShowYoyoCaret = true;
			return super.onSingleTapConfirmed(e);
		}
	}

	@Override
	public boolean onDoubleTap(MotionEvent e)
	{
		int x = (int) e.getX() + _textField.getScrollX();
		int y = (int) e.getY() + _textField.getScrollY();

		//ignore taps on handle
		if (_yoyoCaret.isInHandle(x, y)){
			_textField.selectText(true);
			return true;
		}
		else if( _yoyoStart.isInHandle(x, y))
		{
			return true;
		}
		else
		{
			return super.onDoubleTap(e);
		}
	}

	@Override
	public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX,
						   float velocityY)
	{

		if (_isCaretHandleTouched || _isStartHandleTouched || _isEndHandleTouched)
		{
			onUp(e2);
			return true;
		}
		else
		{
			return super.onFling(e1, e2, velocityX, velocityY);
		}
	}

	@Override
	public void onTextDrawComplete(Canvas canvas)
	{		
		if (!_textField.isSelectText())
		{
			_yoyoCaret.show();
			_yoyoStart.hide();
			_yoyoEnd.hide();

			if (!_isCaretHandleTouched)
			{
				Rect caret = _textField.getBoundingBox(_textField.getCaretPosition());
				int x = caret.left + _textField.getPaddingLeft();
				int y = caret.bottom + _textField.getPaddingTop();
				_yoyoCaret.setRestingCoord(x, y);
			}
			if (_isShowYoyoCaret)
				_yoyoCaret.draw(canvas, _isCaretHandleTouched);
			_isShowYoyoCaret = false;
		}
		else
		{
			_yoyoCaret.hide();
			_yoyoStart.show();
			_yoyoEnd.show();

			if (!(_isStartHandleTouched && _isEndHandleTouched))
			{
				Rect caret = _textField.getBoundingBox(_textField.getSelectionStart());
				int x = caret.left + _textField.getPaddingLeft();
				int y = caret.bottom + _textField.getPaddingTop();
				_yoyoStart.setRestingCoord(x, y);

				Rect caret2 = _textField.getBoundingBox(_textField.getSelectionEnd());
				int x2 = caret2.left + _textField.getPaddingLeft();
				int y2 = caret2.bottom + _textField.getPaddingTop();
				_yoyoEnd.setRestingCoord(x2, y2);
			}

			_yoyoStart.draw(canvas, _isStartHandleTouched);
			_yoyoEnd.draw(canvas, _isStartHandleTouched);
		}
	}

	@Override
	public Rect getCaretBloat()
	{
		return _yoyoCaret.HANDLE_BLOAT;
	}

	@Override
	public void onColorSchemeChanged(ColorScheme colorScheme)
	{
		// TODO: Implement this method
		_yoyoCaret.setHandleColor(colorScheme.getColor(Colorable.CARET_BACKGROUND));
	}

	private class Yoyo
	{
		private final int YOYO_STRING_RESTING_HEIGHT = _yoyoSize / 2;
		private final Rect HANDLE_RECT = new Rect(0, 0, _yoyoSize, _yoyoSize) ;
		public final Rect HANDLE_BLOAT;

		//coordinates where the top of the yoyo string is attached
		private int _anchorX = 0;
		private int _anchorY = 0;

		//coordinates of the top-left corner of the yoyo handle
		private int _handleX = 0;
		private int _handleY = 0;

		//the offset where the handle is first touched,
		//(0,0) being the top-left of the handle
		private int _xOffset = 0;
		private int _yOffset = 0;

		private final static int YOYO_HANDLE_ALPHA = 180;
		private final static int YOYO_HANDLE_COLOR = 0xFF0000FF;
		private final Paint _brush;

		private boolean _isShow;

		public Yoyo()
		{
			int radius = getRadius();
			HANDLE_BLOAT = new Rect(
				radius,
				0,
				0,
				(int)HANDLE_RECT.bottom + YOYO_STRING_RESTING_HEIGHT);
			
			_brush = new Paint();
			_brush.setColor(_textField.getColorScheme().getColor(Colorable.CARET_BACKGROUND));
			//,_brush.setStrokeWidth(2);
			_brush.setAntiAlias(true);  
		}

		public void setHandleColor(int color)
		{
			// TODO: Implement this method
			_brush.setColor(color);
		}

		/**
		 * Draws the yoyo handle and string. The Yoyo handle can extend into 
		 * the padding region.
		 * 
		 * @param canvas
		 * @param activated True if the yoyo is activated. This causes a 
		 * 		different image to be loaded.
		 */
		public void draw(Canvas canvas, boolean activated)
		{
			int radius = getRadius();
			
			canvas.drawLine(_anchorX, _anchorY,
							_handleX + radius, _handleY + radius, _brush);
			canvas.drawArc(new RectF(_anchorX - radius, _anchorY - radius / 2 - YOYO_STRING_RESTING_HEIGHT,
									 _handleX + radius * 2, _handleY + radius / 2), 60, 60, true, _brush);
			canvas.drawOval(new RectF(_handleX, _handleY, _handleX + HANDLE_RECT.right, _handleY + HANDLE_RECT.bottom), _brush);
		}

		final public int getRadius()
		{
			return HANDLE_RECT.right / 2;
		}

		/**
		 * Clear the yoyo at the current position and attaches it to (x, y),
		 * with the handle hanging directly below.
		 */
		public void attachYoyo(int x, int y)
		{
			invalidateYoyo(); //clear old position
			setRestingCoord(x, y);
			invalidateYoyo(); //update new position
		}


		/**
		 * Sets the yoyo string to be attached at (x, y), with the handle 
		 * hanging directly below, but does not trigger any redrawing
		 */
		public void setRestingCoord(int x, int y)
		{
			_anchorX = x;
			_anchorY = y;
			_handleX = x - getRadius();
			_handleY = y + YOYO_STRING_RESTING_HEIGHT;
		}

		private void invalidateYoyo()
		{
			int handleCenter = _handleX + getRadius();
			int x0, x1, y0, y1;
			if (handleCenter >= _anchorX)
			{
				x0 = _anchorX;
				x1 = handleCenter + 1;
			}
			else
			{
				x0 = handleCenter;
				x1 = _anchorX + 1;
			}

			if (_handleY >= _anchorY)
			{
				y0 = _anchorY;
				y1 = _handleY;
			}
			else
			{
				y0 = _handleY;
				y1 = _anchorY;
			}

			//invalidate the string area
			_textField.invalidate(x0, y0, x1, y1);
			invalidateHandle();
		}

		public void invalidateHandle()
		{
			Rect handleExtent = new Rect(_handleX, _handleY,
										 _handleX + HANDLE_RECT.right, _handleY + HANDLE_RECT.bottom);
			_textField.invalidate(handleExtent);
		}

		/**
		 * This method projects a yoyo string directly above the handle and
		 * determines which character it should be attached to, or -1 if no
		 * suitable character can be found.
		 * 
		 * (handleX, handleY) is the handle origin in screen coordinates,
		 * where (0, 0) is the top left corner of the textField, regardless of
		 * its internal scroll values.
		 * 
		 * @return Pair.first contains the nearest character while Pair.second
		 * 			is the exact character found by a strict search 
		 * 
		 */
		public Pair findNearestChar(int handleX, int handleY)
		{
			int attachedLeft = screenToViewX(handleX) - _xOffset + getRadius();
			int attachedBottom = screenToViewY(handleY) - _yOffset - YOYO_STRING_RESTING_HEIGHT - 2;

			return new Pair(_textField.coordToCharIndex(attachedLeft, attachedBottom),
							_textField.coordToCharIndexStrict(attachedLeft, attachedBottom));
		}

		/**
		 * Records the coordinates of the initial down event on the
		 * handle so that subsequent movement events will result in the
		 * handle being offset correctly.
		 * 
		 * Does not check if isInside(x, y). Calling methods have
		 * to ensure that (x, y) is within the handle area.
		 */
		public void setInitialTouch(int x, int y)
		{
			_xOffset = x - _handleX;
			_yOffset = y - _handleY;
		}

		public void clearInitialTouch()
		{
			_xOffset = 0;
			_yOffset = 0;
		}

		public boolean isShow()
		{
			return _isShow;
		}

		public void show()
		{
			_isShow = true;
		}

		public void hide()
		{
			_isShow = false;
		}

		public boolean isInHandle(int x, int y)
		{
			return _isShow && (x >= _handleX
				&& x < (_handleX + HANDLE_RECT.right)
				&& y >= _handleY
				&& y < (_handleY + HANDLE_RECT.bottom)
				);
		}
	}//end inner class
}

/*
 * Copyright (c) 2013 Tah Wei Hoon.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License Version 2.0,
 * with full text available at http://www.apache.org/licenses/LICENSE-2.0.html
 *
 * This software is provided "as is". Use at your own risk.
 */
package com.myopicmobile.textwarrior.android;

import android.view.MotionEvent;

public class TrackpadNavigationMethod extends TouchNavigationMethod {

    private int fling;

    public TrackpadNavigationMethod(FreeScrollingTextField textField) {
        super(textField);
        MOVEMENT_PIXELS = _textField.rowHeight()*2;
    }

    @Override
    public boolean onDown(MotionEvent e) {
        fling=0;
        MOVEMENT_PIXELS = _textField.rowHeight()*2;
        return true;
    }

    @Override
    public boolean onUp(MotionEvent e) {
        _xAccum = 0.0f;
        _yAccum = 0.0f;
        fling=0;
        super.onUp(e);
        return true;
    }

    @Override
    public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX,
                            float distanceY) {

        /*if (fling == 0)
            if (Math.abs(distanceX) > Math.abs(distanceY))
                fling = 1;
            else
                fling = -1;*/
        if (fling == 1)
            distanceY = 0;
        else if (fling == -1)
            distanceX = 0;

        moveCaretWithTrackpad(-distanceX, -distanceY);

        //TODO find out if ACTION_UP events are actually passed to onScroll
        if ((e2.getAction() & MotionEvent.ACTION_MASK) == MotionEvent.ACTION_UP) {
            onUp(e2);
        }

        return true;
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX,
                           float velocityY) {
        onUp(e2);
        return true;
    }


    //number of pixels to scroll to move the caret one unit
    private static int MOVEMENT_PIXELS = 16;
    //for use in determining whether the displacement is mainly on the x or y axis
    private final static double MIN_ATAN = 0.322; // == atan(1/3)
    private float _xAccum = 0.0f;
    private float _yAccum = 0.0f;

    private void moveCaretWithTrackpad(float distanceX, float distanceY) {
        //reset accumulators when polarity of displacement changes
        if ((_xAccum < 0 && distanceX > 0) || (_xAccum > 0 && distanceX < 0)) {
            _xAccum = 0;
        }
        if ((_yAccum < 0 && distanceY > 0) || (_yAccum > 0 && distanceY < 0)) {
            _yAccum = 0;
        }

        double angle = Math.atan2(Math.abs(distanceX), Math.abs(distanceY));

        if (angle >= MIN_ATAN) {
            //non-negligible x-axis movement
            float x = _xAccum + distanceX;
            int xUnits = ((int) x) / MOVEMENT_PIXELS;
            _xAccum = x - (xUnits * MOVEMENT_PIXELS);

            while (xUnits > 0) {
                _textField.moveCaretRight();
                --xUnits;
                if (fling == 0)
                    fling = 1;
            }
            while (xUnits < 0) {
                _textField.moveCaretLeft();
                ++xUnits;
                if (fling == 0)
                    fling = 1;
            }
        }

        if ((Math.PI / 2 - angle) >= MIN_ATAN) {
            //non-negligible y-axis movement
            float y = _yAccum + distanceY;
            int yUnits = ((int) y) / MOVEMENT_PIXELS;
            _yAccum = y - (yUnits * MOVEMENT_PIXELS);

            for (int i = yUnits; i > 0; --i) {
                _textField.moveCaretDown();
                if (fling == 0)
                    fling = -1;
            }
            for (int i = yUnits; i < 0; ++i) {
                _textField.moveCaretUp();
                if (fling == 0)
                    fling = -1;
            }
        }
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent e) {
        return super.onSingleTapConfirmed(e);
    }

    @Override
    public void onLongPress(MotionEvent e) {
        _textField.setSelected(!_textField.isSelectText());
        _textField.setSelectionRange(_textField.getCaretPosition(),0);
    }

    @Override
    public boolean onDoubleTap(MotionEvent e) {
        _textField.setSelected(!_textField.isSelectText());
        _textField.setSelectionRange(_textField.getCaretPosition(),0);
        return true;
    }

}

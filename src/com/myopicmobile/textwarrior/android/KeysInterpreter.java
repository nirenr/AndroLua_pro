/*
 * Copyright (c) 2013 Tah Wei Hoon.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License Version 2.0,
 * with full text available at http://www.apache.org/licenses/LICENSE-2.0.html
 *
 * This software is provided "as is". Use at your own risk.
 */
package com.myopicmobile.textwarrior.android;

import com.myopicmobile.textwarrior.common.Language;

import android.view.KeyEvent;

/**
 * Interprets shortcut key combinations and contains utility methods
 * to map Android keycodes to Unicode equivalents.
 */
public class KeysInterpreter {
	public static boolean isSwitchPanel(KeyEvent event){
		return (event.isShiftPressed() &&
					(event.getKeyCode() == KeyEvent.KEYCODE_ENTER));
	}

	/**
	 * Maps shortcut keys and Android keycodes to printable characters.
	 * Note that whitespace is considered printable.
	 * 
	 * @param event The KeyEvent to interpret
	 * @return The printable character the event represents, 
	 * 	or Language.NULL_CHAR if the event does not represent a printable char
	 */
	public static char keyEventToPrintableChar(KeyEvent event){
		char c = Language.NULL_CHAR;

    	// convert tab, backspace, newline and space keycodes to standard ASCII values
    	if (isNewline(event)){
    		c = Language.NEWLINE;
        }
    	else if (isBackspace(event)){
    		c = Language.BACKSPACE;
        }
		// This should be before the check for isSpace() because the
    	// shortcut for TAB uses the SPACE key.
    	else if (isTab(event)){
    		c = Language.TAB;
        }
    	else if (isSpace(event)){
    		c = ' ';
        }
    	else if (event.isPrintingKey()){
    		c = (char) event.getUnicodeChar(event.getMetaState());
    	}
    	
		return c;
	}
	
	private static boolean isTab(KeyEvent event){
		return (event.isShiftPressed() &&
					(event.getKeyCode() == KeyEvent.KEYCODE_SPACE)) ||
				(event.getKeyCode() == KeyEvent.KEYCODE_TAB);
	}

	private static boolean isBackspace(KeyEvent event) {
		return (event.getKeyCode() == KeyEvent.KEYCODE_DEL);
	}

	private static boolean isNewline(KeyEvent event) {
		return (event.getKeyCode() == KeyEvent.KEYCODE_ENTER);
	}

	private static boolean isSpace(KeyEvent event) {
		return (event.getKeyCode() == KeyEvent.KEYCODE_SPACE);
	}

	public static boolean isNavigationKey(KeyEvent event) {
		int keyCode = event.getKeyCode();
		return keyCode == KeyEvent.KEYCODE_DPAD_DOWN ||
			keyCode == KeyEvent.KEYCODE_DPAD_UP ||
			keyCode == KeyEvent.KEYCODE_DPAD_RIGHT ||
			keyCode == KeyEvent.KEYCODE_DPAD_LEFT;
	}
}

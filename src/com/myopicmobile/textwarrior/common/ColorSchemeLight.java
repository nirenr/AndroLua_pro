/*
 * Copyright (c) 2013 Tah Wei Hoon.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License Version 2.0,
 * with full text available at http://www.apache.org/licenses/LICENSE-2.0.html
 *
 * This software is provided "as is". Use at your own risk.
 */

package com.myopicmobile.textwarrior.common;


/**
 * Off-black on off-white background color scheme
 */
public class ColorSchemeLight extends ColorScheme {

	public ColorSchemeLight(){
		setColor(Colorable.FOREGROUND, OFF_BLACK);
		setColor(Colorable.BACKGROUND, OFF_WHITE);
		setColor(Colorable.SELECTION_FOREGROUND, OFF_WHITE);
		setColor(Colorable.CARET_FOREGROUND, OFF_WHITE);
	}

	private static final int OFF_WHITE = 0xFFF0F0ED;
	private static final int OFF_BLACK = 0xFF333333;

	@Override
	public boolean isDark() {
		return false;
	}
}

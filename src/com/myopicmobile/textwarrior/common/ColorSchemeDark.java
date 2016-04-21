/*
 * Copyright (c) 2013 Tah Wei Hoon.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License Version 2.0,
 * with full text available at http://www.apache.org/licenses/LICENSE-2.0.html
 *
 * This software is provided "as is". Use at your own risk.
 */

package com.myopicmobile.textwarrior.common;


public class ColorSchemeDark extends ColorScheme {

	public ColorSchemeDark(){
		setColor(Colorable.FOREGROUND, OFF_WHITE);
		setColor(Colorable.BACKGROUND, OFF_BLACK);
		//setColor(Colorable.SELECTION_FOREGROUND, OFF_WHITE);
		//setColor(Colorable.SELECTION_BACKGROUND, OCEAN_BLUE);
		//setColor(Colorable.CARET_FOREGROUND, OFF_BLACK);
		//setColor(Colorable.CARET_BACKGROUND, FLUORESCENT_YELLOW);
		//setColor(Colorable.CARET_DISABLED, LIGHT_GREY);
		//setColor(Colorable.LINE_HIGHLIGHT, 0xFf00FF00);
		setColor(Colorable.NON_PRINTING_GLYPH, DARK_GREY);
		//setColor(Colorable.COMMENT, JUNGLE_GREEN);
		//setColor(Colorable.KEYWORD, MARINE);
		//setColor(Colorable.LITERAL, PEACH);
		//setColor(Colorable.SECONDARY, BEIGE);
	}

	private static final int BEIGE = 0xFFD7BA7D;
	private static final int DARK_GREY = 0xFF606060;
	private static final int FLUORESCENT_YELLOW = 0xFFEFF193;
	private static final int JUNGLE_GREEN = 0xFF608B4E;
	private static final int LIGHT_GREY = 0xFFD3D3D3;
	private static final int MARINE = 0xFF569CD6;
	private static final int OCEAN_BLUE = 0xFF256395;
	private static final int OFF_BLACK = 0xFF040404;
	private static final int OFF_WHITE = 0xFFD0D2D3;
	private static final int PEACH = 0xFFD69D85;

	@Override
	public boolean isDark() {
		return true;
	}
}

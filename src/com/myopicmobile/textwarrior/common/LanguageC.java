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
 * Singleton class containing the symbols and operators of the C language
 */
public class LanguageC extends Language{
	private static Language _theOne = null;
	
	private final static String[] keywords = {
		"char", "double", "float", "int", "long", "short", "void",
		"auto", "const", "extern", "register", "static", "volatile",
		"signed", "unsigned", "sizeof", "typedef",
		"enum", "struct", "union",
		"break", "case", "continue", "default", "do", "else", "for",
		"goto", "if", "return", "switch", "while"
		};

	public static Language getInstance(){
		if(_theOne == null){
			_theOne = new LanguageC();
		}
		return _theOne;
	}
	
	private LanguageC(){
		super.setKeywords(keywords);
	}
}

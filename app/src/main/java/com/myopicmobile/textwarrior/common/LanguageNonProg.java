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
 * Singleton class that represents a non-programming language without keywords, 
 * operators etc.
 */
public class LanguageNonProg extends Language{
	private static Language _theOne = null;
	
	private final static String[] keywords = {};
	
	private final static char[] operators = {};


	public static Language getInstance(){
		if(_theOne == null){
			_theOne = new LanguageNonProg();
		}
		return _theOne;
	}
	
	private LanguageNonProg(){
		super.setKeywords(keywords);
		super.setOperators(operators);
	}

	@Override
	public boolean isProgLang(){
		return false;
	}

	@Override
	public boolean isEscapeChar(char c){
		return false;
	}

	@Override
	public boolean isDelimiterA(char c){
		return false;
	}

	@Override
	public boolean isDelimiterB(char c){
		return false;
	}
	
	@Override
	public boolean isLineAStart(char c){
		return false;
	}

	@Override
	public boolean isLineStart(char c0, char c1){
		return false;
	}

	@Override
	public boolean isMultilineStartDelimiter(char c0, char c1){
		return false;
	}

	@Override
	public boolean isMultilineEndDelimiter(char c0, char c1){
		return false;
	}
}

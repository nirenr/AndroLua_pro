/*
 * Copyright (c) 2013 Tah Wei Hoon.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License Version 2.0,
 * with full text available at http://www.apache.org/licenses/LICENSE-2.0.html
 *
 * This software is provided "as is". Use at your own risk.
 */
package com.myopicmobile.textwarrior.common;

import java.util.List;

/**
 * Iterator class to access characters of the underlying text buffer.
 *
 * The usage procedure is as follows:
 * 1. Call seekChar(offset) to mark the position to start iterating
 * 2. Call hasNext() to see if there are any more char
 * 3. Call next() to get the next char
 *
 * If there is more than 1 DocumentProvider pointing to the same Document,
 * changes made by one DocumentProvider will not cause other DocumentProviders
 * to be notified. Implement a publish/subscribe interface if required.
 */
public class DocumentProvider implements java.lang.CharSequence
{

	@Override
	public int length()
	{
		// TODO: Implement this method
		return _theText.length();
	}

	/** Current position in the text. Range [ 0, _theText.getTextLength() ) */
	private int _currIndex;
	private final Document _theText;

	public DocumentProvider(Document.TextFieldMetrics metrics){
		_currIndex = 0;
		_theText = new Document(metrics);
	}

	public DocumentProvider(Document doc){
		_currIndex = 0;
		_theText = doc;
	}

	public DocumentProvider(DocumentProvider rhs){
		_currIndex = 0;
		_theText = rhs._theText;
	}

	/**
	 * Get a substring of up to maxChars length, starting from charOffset
	 */
	public CharSequence subSequence(int charOffset, int maxChars){
		return _theText.subSequence(charOffset, maxChars);
	}

	public char charAt(int charOffset){
		if(_theText.isValid(charOffset)){
			return _theText.charAt(charOffset);
		}
		else{
			return Language.NULL_CHAR;
		}
	}

	public String getRow(int rowNumber){
		return _theText.getRow(rowNumber);
	}

	/**
	 * Get the row number that charOffset is on
	 */
	public int findRowNumber(int charOffset){
		return _theText.findRowNumber(charOffset);
	}

	/**
	 * Get the line number that charOffset is on. The difference between a line
	 * and a row is that a line can be word-wrapped into many rows.
	 */
	public int findLineNumber(int charOffset){
		return _theText.findLineNumber(charOffset);
	}

	/**
	 * Get the offset of the first character on rowNumber
	 */
	public int getRowOffset(int rowNumber){
		return _theText.getRowOffset(rowNumber);
	}


	/**
	 * Get the offset of the first character on lineNumber. The difference
	 * between a line and a row is that a line can be word-wrapped into many rows.
	 */
	public int getLineOffset(int lineNumber){
		return _theText.getLineOffset(lineNumber);
	}

	/**
	 * Sets the iterator to point at startingChar.
	 *
	 * If startingChar is invalid, hasNext() will return false, and _currIndex
	 * will be set to -1.
	 *
	 * @return startingChar, or -1 if startingChar does not exist
	 */
	public int seekChar(int startingChar){
		if(_theText.isValid(startingChar)){
			_currIndex = startingChar;
		}
		else{
			_currIndex = -1;
		}
		return _currIndex;
	}

	public boolean hasNext(){
		return (_currIndex >= 0 &&
				_currIndex < _theText.getTextLength());
	}

	/**
	 * Returns the next character and moves the iterator forward.
	 *
	 * Does not do bounds-checking. It is the responsibility of the caller
	 * to check hasNext() first.
	 *
	 * @return Next character
	 */
	public char next(){
		char nextChar = _theText.charAt(_currIndex);
		++_currIndex;
		return nextChar;
	}

	/**
	 * Inserts c into the document, shifting existing characters from
	 * insertionPoint (inclusive) to the right
	 *
	 * If insertionPoint is invalid, nothing happens.
	 */
	public void insertBefore(char c, int insertionPoint, long timestamp){
		if(!_theText.isValid(insertionPoint)){
			return;
		}

		char[] a = new char[1];
		a[0] = c;
		_theText.insert(a, insertionPoint, timestamp, true);
	}

	/**
	 * Inserts characters of cArray into the document, shifting existing
	 * characters from insertionPoint (inclusive) to the right
	 *
	 * If insertionPoint is invalid, nothing happens.
	 */
	public void insertBefore(char[] cArray, int insertionPoint, long timestamp){
		if(!_theText.isValid(insertionPoint) || cArray.length == 0){
			return;
		}

		_theText.insert(cArray, insertionPoint, timestamp, true);
	}

	public void insert(int i, CharSequence s)
	{
		_theText.insert(new char[]{s.charAt(0)},i,System.nanoTime(),true);
	}
	/**
	 * Deletes the character at deletionPoint index.
	 * If deletionPoint is invalid, nothing happens.
	 */
	public void deleteAt(int deletionPoint, long timestamp){
		if(!_theText.isValid(deletionPoint)){
			return;
		}
		_theText.delete(deletionPoint, 1, timestamp, true);
	}


	/**
	 * Deletes up to maxChars number of characters starting from deletionPoint
	 * If deletionPoint is invalid, or maxChars is not positive, nothing happens.
	 */
	public void deleteAt(int deletionPoint, int maxChars, long time){
		if(!_theText.isValid(deletionPoint) || maxChars <= 0){
			return;
		}
		int totalChars = Math.min(maxChars, _theText.getTextLength() - deletionPoint);
		_theText.delete(deletionPoint, totalChars, time, true);
	}

	/**
	 * Returns true if the underlying text buffer is in batch edit mode
	 */
	public boolean isBatchEdit(){
		return _theText.isBatchEdit();
	}

	/**
	 * Signals the beginning of a series of insert/delete operations that can be
	 * undone/redone as a single unit
	 */
	public void beginBatchEdit(){
		_theText.beginBatchEdit();
	}

	/**
	 * Signals the end of a series of insert/delete operations that can be
	 * undone/redone as a single unit
	 */
	public void endBatchEdit(){
		_theText.endBatchEdit();
	}

	/**
	 * Returns the number of rows in the document
	 */
	public int getRowCount(){
		return _theText.getRowCount();
	}

	/**
	 * Returns the number of characters in the row specified by rowNumber
	 */
	public int getRowSize(int rowNumber){
		return _theText.getRowSize(rowNumber);
	}

	/**
	 * Returns the number of characters in the document, including the terminal
	 * End-Of-File character
	 */
	public int docLength(){
		return _theText.getTextLength();
	}

	//TODO make thread-safe
	/**
	 * Removes spans from the document.
	 * Beware: Not thread-safe! Another thread may be modifying the same spans
	 * returned from getSpans()
	 */
	public void clearSpans(){
		_theText.clearSpans();
	}

	/**
	 * Beware: Not thread-safe!
	 */
	public List<Pair> getSpans(){
		return _theText.getSpans();
	}

	/**
	 * Sets the spans to use in the document.
	 * Spans are continuous sequences of characters that have the same format
	 * like color, font, etc.
	 *
	 * @param spans A collection of Pairs, where Pair.first is the start
	 * 		position of the token, and Pair.second is the type of the token.
	 */
	public void setSpans(List<Pair> spans){
		_theText.setSpans(spans);
	}

	public void setMetrics(Document.TextFieldMetrics metrics){
		_theText.setMetrics(metrics);
	}

	/**
	 * Enable/disable word wrap for the document. If enabled, the document is
	 * immediately analyzed for word wrap breakpoints, which might take an
	 * arbitrarily long time.
	 */
	public void setWordWrap(boolean enable){
		_theText.setWordWrap(enable);
	}

	public boolean isWordWrap(){
		return _theText.isWordWrap();
	}

	/**
	 * Analyze the document for word wrap break points. Does nothing if word
	 * wrap is disabled for the document.
	 */
	public void analyzeWordWrap(){
		_theText.analyzeWordWrap();
	}

	public boolean canUndo() {
		return _theText.canUndo();
	}

	public boolean canRedo() {
		return _theText.canRedo();
	}

	public int undo() {
		return _theText.undo();
	}

	public int redo() {
		return _theText.redo();
	}

	@Override
	public String toString()
	{
		// TODO: Implement this method
		return _theText.toString();
	}
	
	
}

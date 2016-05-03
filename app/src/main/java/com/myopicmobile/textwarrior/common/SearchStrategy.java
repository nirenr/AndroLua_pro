/*
 * Copyright (c) 2013 Tah Wei Hoon.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License Version 2.0,
 * with full text available at http://www.apache.org/licenses/LICENSE-2.0.html
 *
 * This software is provided "as is". Use at your own risk.
 */
package com.myopicmobile.textwarrior.common;

public interface SearchStrategy {
	/**
	 * Searches for target, starting from start (inclusive),
	 * and stopping at end (exclusive).
	 * 
	 * @return charOffset of found string; -1 if not found
	 */
	public int find(DocumentProvider src, String target, int start, int end,
			boolean isCaseSensitive, boolean isWholeWord);

	/**
	 * Searches for target, starting from start (inclusive),
	 * wrapping around to the beginning of document and
	 * stopping at start (exclusive).
	 * 
	 * @return charOffset of found string; -1 if not found
	 */
	public int wrappedFind(DocumentProvider src, String target, int start,
			boolean isCaseSensitive, boolean isWholeWord);

	/**
	 * Searches backwards from startCharOffset (inclusive),
	 * and stopping at end (exclusive).
	 * 
	 * @return charOffset of found string; -1 if not found
	 */
	public int findBackwards(DocumentProvider src, String target, int start, int end,
			boolean isCaseSensitive, boolean isWholeWord);
	
	/**
	 * Searches backwards from start (inclusive), wrapping around to
	 * the end of document and stopping at start (exclusive).
	 * 
	 * @return charOffset of found string; -1 if not found
	 */
	public int wrappedFindBackwards(DocumentProvider src, String target, int start,
			boolean isCaseSensitive, boolean isWholeWord);
	
	/**
	 * Replace all matches of searchText in src with replacementText.
	 * 
	 * @param mark Optional. A position in src that can be tracked for changes.
	 * 		After replacements are made, the position may be shifted because of
	 * 		insertion/deletion of text before it. The new position of mark is 
	 * 		returned in Pair.second. If mark is an invalid position, Pair.second
	 * 		is undefined.
	 * 
	 * @return Pair.first is the number of replacements made.
	 * 		Pair.second is new position of mark after replacements are made.
	 */
	public Pair replaceAll(DocumentProvider src, String searchText,
			String replacementText, int mark,
			boolean isCaseSensitive, boolean isWholeWord);
	
	
	/**
	 * The number of characters that have been examined by the current find
	 * operation. This method is not synchronized, and the value returned
	 * may be outdated.
	 * 
	 * @return The number of characters searched so far
	 */
	public int getProgress();
}

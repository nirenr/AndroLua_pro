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
 * A LRU cache that stores the last seek line and its corresponding index so
 * that future lookups can start from the cached position instead of
 * the beginning of the file
 * 
 * _cache.Pair.First = line index
 * _cache.Pair.Second = character offset of first character in that line
 * 
 * TextBufferCache always has one valid entry (0,0) signifying that in line 0,
 * the first character is at offset 0. This is true even for an "empty" file, 
 * which is not really empty because TextBuffer inserts a EOF character in it.
 * 
 * Therefore, _cache[0] is always occupied by the entry (0,0). It is not affected
 * by invalidateCache, cache miss, etc. operations
 */
public class TextBufferCache {
	private static final int CACHE_SIZE = 4; // minimum = 1
	private Pair[] _cache = new Pair[CACHE_SIZE];
	
	public TextBufferCache(){
		_cache[0] = new Pair(0, 0); // invariant lineIndex and charOffset relation
		for (int i = 1; i < CACHE_SIZE; ++i){
			_cache[i] = new Pair(-1, -1);
			// -1 line index is used implicitly in calculations in getNearestMatch
		}
	}

	//TODO consider extracting common parts with getNearestCharOffset(int)
	public Pair getNearestLine(int lineIndex){
		int nearestMatch = 0;
		int nearestDistance = Integer.MAX_VALUE;
		for(int i = 0; i < CACHE_SIZE; ++i){
			int distance = Math.abs(lineIndex - _cache[i].getFirst());
			if (distance < nearestDistance){
				nearestDistance = distance;
				nearestMatch = i;
			}
		}

		Pair nearestEntry = _cache[nearestMatch];
		makeHead(nearestMatch);
		return nearestEntry;
	}
	
	public Pair getNearestCharOffset(int charOffset){
		int nearestMatch = 0;
		int nearestDistance = Integer.MAX_VALUE;
		for(int i = 0; i < CACHE_SIZE; ++i){
			int distance = Math.abs(charOffset - _cache[i].getSecond());
			if (distance < nearestDistance){
				nearestDistance = distance;
				nearestMatch = i;
			}
		}
		
		Pair nearestEntry = _cache[nearestMatch];
		makeHead(nearestMatch);
		return nearestEntry;
	}
	
	/**
	 * Place _cache[newHead] at the top of the list
	 */
	private void makeHead(int newHead){
		if(newHead == 0){
			return;
		}
		
		Pair temp = _cache[newHead];
		for(int i = newHead; i > 1; --i){
			_cache[i] = _cache[i-1];
		}
		_cache[1] = temp; // _cache[0] is always occupied by (0,0)
	}
	
	public void updateEntry(int lineIndex, int charOffset){
		if(lineIndex <= 0){
		// lineIndex 0 always has 0 charOffset; ignore. Also ignore negative lineIndex
			return;
		}

		if(!replaceEntry(lineIndex, charOffset)){
			insertEntry(lineIndex, charOffset);
		}
	}
	
	private boolean replaceEntry(int lineIndex, int charOffset){
		for (int i = 1; i < CACHE_SIZE; ++i){
			if(_cache[i].getFirst() == lineIndex){
				_cache[i].setSecond(charOffset);
				return true;
			}
		}
		return false;
	}
	
	private void insertEntry(int lineIndex, int charOffset){
		makeHead(CACHE_SIZE-1); // rotate right list of entries
		// replace head (most recently used entry) with new entry
		_cache[1] = new Pair(lineIndex, charOffset);
	}

	/**
	 * Invalidate all cache entries that have char offset >= fromCharOffset
	 */
	final protected void invalidateCache(int fromCharOffset){
		for (int i = 1; i < CACHE_SIZE; ++i){
			if(_cache[i].getSecond() >= fromCharOffset){
				_cache[i] = new Pair(-1, -1);
			}
		}
	}
}

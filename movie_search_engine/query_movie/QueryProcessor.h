
#ifndef QUERYPROCESSOR_H
#define QUERYPROCESSOR_H

#include "MovieIndex.h"

/**
 *  A SearchResult is a doc_idd and row_id,
 * representing a particular row in a particular file.
 *
 */
typedef struct searchResult {
  uint64_t doc_id;
  int row_id;
} *SearchResult;

/**
 * A SearchResultIter goes through every element in the hashtable,
 * which are all lists of document locations.
 *
 */
typedef struct searchResultIter {
  int cur_doc_id;
  HTIter doc_iter;
  LLIter offset_iter;
} *SearchResultIter;

SearchResultIter CreateSearchResultIter(MovieSet set);

void DestroySearchResultIter(SearchResultIter iter);

int SearchResultGet(SearchResultIter iter, SearchResult output);

int SearchResultNext(SearchResultIter iter);

int SearchResultIterHasMore(SearchResultIter iter);

SearchResultIter FindMovies(Index index, char *term);

#endif
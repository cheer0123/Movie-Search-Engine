
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "QueryProcessor.h"
#include "MovieIndex.h"
#include "htll/LinkedList.h"
#include "htll/Hashtable.h"

SearchResultIter CreateSearchResultIter(MovieSet set) {
  SearchResultIter iter =
    (SearchResultIter)malloc(sizeof(struct searchResultIter));

  if (iter == NULL) {
    printf("Couldn't malloc for an iter in CreateSearchResultIter\n");
    return NULL;
  }

  // Initialize doc_iter
  iter->doc_iter = CreateHashtableIterator((Hashtable)set->doc_index);

  if (iter->doc_iter == NULL) {
    printf("Couldn't create an iterator; or iterator was empty (no docs)\n");
  }

  // Initialize offset_iter
  HTKeyValue kvp;
  HTIteratorGet(iter->doc_iter, &kvp);

  // key is docid
  iter->cur_doc_id = kvp.key;
  // value is offset list
  iter->offset_iter = CreateLLIter((LinkedList)kvp.value);

  return iter;
}

void DestroySearchResultIter(SearchResultIter iter) {
  // Destroy LLIter
  if (iter->offset_iter != NULL) {
    DestroyLLIter(iter->offset_iter);
  }

  // Destroy doc_iter
  DestroyHashtableIterator(iter->doc_iter);

  free(iter);
}



SearchResultIter FindMovies(Index index, char *term) {
  MovieSet set = GetMovieSet(index, term);
  if (set == NULL) {
    return NULL;
  }
  printf("Getting docs for movieset term: \"%s\"\n", set->desc);
  SearchResultIter iter = CreateSearchResultIter(set);
  return iter;
}


int SearchResultGet(SearchResultIter iter, SearchResult output) {
  void *payload;
  LLIterGetPayload(iter->offset_iter, &payload);
  int row_id = *((int*)payload);
  output->doc_id = iter->cur_doc_id;
  output->row_id = row_id;
  return 0;
}

int SearchResultNext(SearchResultIter iter) {
  // If there are no more offsets for this doc
  if (LLIterHasNext(iter->offset_iter) == 0) {
    // destroy LLIter, get next docid, create new offset_iter.
    DestroyLLIter(iter->offset_iter);

    // Get next document
    if (HTIteratorHasMore(iter->doc_iter)) {
      HTKeyValue kvp;
      HTIteratorNext(iter->doc_iter);
      HTIteratorGet(iter->doc_iter, &kvp);
      // key is docid
      iter->cur_doc_id = kvp.key;
      // value is offset list
      iter->offset_iter = CreateLLIter((LinkedList)kvp.value);
    } else {
      iter->offset_iter = NULL;
      iter->doc_iter = NULL;
      return -1;
    }
  } else {
    LLIterNext(iter->offset_iter);
  }
  return 0;
}

// Return 0 if no more
int SearchResultIterHasMore(SearchResultIter iter) {
  if (iter->doc_iter == NULL) {
    return 0;
  }
  if (LLIterHasNext(iter->offset_iter) == 0) {
    return (HTIteratorHasMore(iter->doc_iter));
  }

  return 1;
}






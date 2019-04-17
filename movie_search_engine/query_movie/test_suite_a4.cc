/*
 *  Created by Adrienne Slaughter
 *  CS 5007 Spring 2019
 *  Northeastern University, Seattle
 *
 *  This is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  It is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  See <http://www.gnu.org/licenses/>.
 */

#include "gtest/gtest.h"

extern "C" {
#include "htll/Hashtable.h"
#include "htll/LinkedList.h"
#include "DocIdMap.h"
#include "FileCrawler.h"
#include "FileParser.h"
#include "MovieIndex.h"
#include "Movie.h"
#include <string.h>
}

TEST(DocIdMap, Full) {
  // Create DocIdMap
  DocIdMap docs = CreateDocIdMap();

  EXPECT_TRUE(docs != NULL);

  // Some files
  // Not ideal; opportunity for improvement next time around.
  char *f1 = (char*)(malloc(sizeof(char)*10));
  strcpy(f1, "file1");
  PutFileInMap(f1, docs);
  EXPECT_EQ(NumElemsInHashtable(docs), 1);

  HTKeyValue kvp;
  LookupInHashtable(docs, 1, &kvp);
  EXPECT_EQ(strcmp((const char*)kvp.value, "file1"), 0);

  char *f2 = (char*)(malloc(sizeof(char)*10));
  strcpy(f2, "file2");
  PutFileInMap(f2, docs);
  EXPECT_EQ(NumElemsInHashtable(docs), 2);

  HTKeyValue kvp2;
  LookupInHashtable(docs, 2, &kvp2);
  EXPECT_EQ(strcmp((const char*)kvp2.value, "file2"), 0);

  char *out = GetFileFromId(docs, 1);
  EXPECT_EQ(strcmp(out, "file1"), 0);

  DocIdIter docIter = CreateDocIdIterator(docs);

  int numItems = 0;
  while (HTIteratorHasMore(docIter) != 0) {
    HTKeyValue kvp3;
    HTIteratorGet(docIter, &kvp3);
    EXPECT_TRUE(numItems < 2);
    numItems++;
    HTIteratorNext(docIter);
  }

  EXPECT_EQ(numItems, 1);  // Last element not counted

  DestroyDocIdIterator(docIter);

  DestroyDocIdMap(docs);
}

TEST(FileCrawler, Create) {
  // Create a DocIdMap
  DocIdMap docs = CreateDocIdMap();
  // Choose a directory
  CrawlFilesToMap("data_tiny/", docs);

  EXPECT_EQ(NumElemsInHashtable(docs), 10);

  int ids[11] = {0};

  HTIter iter = CreateHashtableIterator(docs);
  HTKeyValue kv;

  while (HTIteratorHasMore(iter) != 0) {
    HTIteratorGet(iter, &kv);
    //   printf("docId: %d\tfilename: %s\n", kv->key, kv->value);
    ids[kv.key]++;
    HTIteratorNext(iter);
  }
  // Get the last
  HTIteratorGet(iter, &kv);
  //   printf("docId: %d\tfilename: %s\n", kv->key, kv->value);
  ids[kv.key]++;

  // Should be a unique id for each file.
  for (int i = 1; i <= 10; i++) {
    EXPECT_EQ(ids[i], 1);
  }

  DestroyHashtableIterator(iter);
  DestroyDocIdMap(docs);
}

TEST(MovieStruct, Full) {
  // Again, the weirdness to work with strings as not-constants.

  char row[100];
  strcpy(row,"1|movie|Sleepless in Seattle|Sleepless in Seattle|0|1984|\\N|120|Comedy,Romance");
  Movie *movie = CreateMovieFromRow(row);

  EXPECT_EQ(strcmp(movie->id, "1"), 0);
  EXPECT_EQ(strcmp(movie->type, "movie"), 0);
  EXPECT_EQ(strcmp(movie->title, "Sleepless in Seattle"), 0);
  EXPECT_EQ(movie->isAdult, 0);
  EXPECT_EQ(movie->year, 1984);
  EXPECT_EQ(movie->runtime, 120);
  //  EXPECT_EQ(strcmp(movie->genres, "Comedy,Romance"), 0);
  // TODO: check the right thing for correct genre.
  EXPECT_EQ(strcmp(movie->genres[0], "Comedy"), 0);
  EXPECT_EQ(strcmp(movie->genres[1], "Romance"), 0);

  DestroyMovie(movie);
}

TEST(MovieIndex, Full) {
  // Create a movie index
  Index ind = CreateIndex();

  // Add some movies to the index
  Movie *m1 = CreateMovie();
  char* title = (char*)(malloc(sizeof(char)*50));
  strcpy(title, "Sleepless in Seattle");

  m1->title = title;
  AddMovieTitleToIndex(ind, m1, 1, 19);

  // Check the indices
  MovieSet set = GetMovieSet(ind, "seattle");
  EXPECT_EQ(strcmp(set->desc, "seattle"), 0);
  EXPECT_EQ(NumElemsInHashtable(set->doc_index), 1);
  HTKeyValue kvp;
  LookupInHashtable(set->doc_index, 1, &kvp);

  EXPECT_EQ(NumElementsInLinkedList((LinkedList)kvp.value), 1u);
  EXPECT_EQ(NumElemsInHashtable(ind->ht), 3);

  DestroyOffsetIndex(ind);
  DestroyMovie(m1);
}


TEST(FileParser, Full) {
  // Given a docIdMap and an index,
  // the index should contain the info.
  DocIdMap docs = CreateDocIdMap();
  // Some files
  // Not ideal; opportunity for improvement next time around.
  char *f1 = (char*)(malloc(sizeof(char)*30));
  strcpy(f1, "data_tiny/fourth/fifth/tinyaa");
  PutFileInMap(f1, docs);
  EXPECT_EQ(NumElemsInHashtable(docs), 1);

  Index ind = CreateIndex();

  ParseTheFiles(docs, ind);

  MovieSet set = GetMovieSet(ind, "coffee.");
  EXPECT_EQ(NumElemsInHashtable(set->doc_index), 1);

  set = GetMovieSet(ind, "elephant");
  EXPECT_EQ(NumElemsInHashtable(set->doc_index), 1);

  set = GetMovieSet(ind, "foobar");
  EXPECT_TRUE(set == NULL);

  DestroyOffsetIndex(ind);
  DestroyDocIdMap(docs);
  // Not freeing f1 because DestroyDocIdMap does
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

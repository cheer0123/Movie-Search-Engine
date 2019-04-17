#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/time.h>
#include <time.h>

#include <unistd.h>
#include <errno.h>

#include "htll/Hashtable.h"
#include "htll/LinkedList.h"

#include "MovieSet.h"
#include "DocIdMap.h"
#include "FileParser.h"
#include "FileCrawler.h"
#include "MovieIndex.h"
#include "Movie.h"
#include "QueryProcessor.h"
#include "MovieReport.h"


DocIdMap docs;
Index docIndex;
Index movie_index;


/**
 * Open the specified file, read the specified row into the
 * provided pointer to the movie.
 */
int CreateMovieFromFileRow(char *file, long rowId, Movie** movie) {
  FILE *fp;

  char buffer[1000];

  fp = fopen(file, "r");

  int i=0;
  while (i <= rowId) {
    fgets(buffer, 1000, fp);
    i++;
  }
  // taking \n out of the row
  buffer[strlen(buffer)-1] = ' ';
  // Create movie from row
  *movie = CreateMovieFromRow(buffer);
  fclose(fp);
  return 0;
}

void BenchmarkSetOfMovies(DocIdMap docs) {
  HTIter iter = CreateHashtableIterator(docs);
  // Now go through all the files, and insert them into the index.
  HTKeyValue kv;
  HTIteratorGet(iter, &kv);
  LinkedList movie_list  = ReadFile((char*)kv.value);
  movie_index = BuildMovieIndex(movie_list, Genre);

  while (HTIteratorHasMore(iter) != 0) {
    HTIteratorNext(iter);
    HTIteratorGet(iter, &kv);
    movie_list = ReadFile((char*)kv.value);
    AddToMovieIndex(movie_index, movie_list, Genre);
  }

  printf("%d entries in the index.\n", NumElemsInHashtable(movie_index->ht));
  DestroyHashtableIterator(iter);
}


void BenchmarkMovieSet(DocIdMap docs) {
  // Create the index
  docIndex = CreateIndex();

  // Index the files
  printf("Parsing and indexing files with multithread...\n");
  ParseTheFiles_MT(docs, docIndex);
  printf("%d entries in the index.\n", NumElemsInHashtable(docIndex->ht));
}

void WriteFile(FILE *file) {
  int buffer_size = 1000;
  char buffer[buffer_size];

  while (fgets(buffer, buffer_size, file) != NULL) {
    printf("%s\n", buffer);
  }
}

/*
 * Measures the current (and peak) resident and virtual memories
 * usage of your linux C process, in kB
 */
void getMemory() {
  //    int* currRealMem, int* peakRealMem,
  //    int* currVirtMem, int* peakVirtMem) {

  int currRealMem, peakRealMem, currVirtMem, peakVirtMem;

  // stores each word in status file
  char buffer[1024] = "";

  // linux file contains this-process info
  FILE* file = fopen("/proc/self/status", "r");

  // read the entire file
  while (fscanf(file, " %1023s", buffer) == 1) {
    if (strcmp(buffer, "VmRSS:") == 0) {
      fscanf(file, " %d", &currRealMem);
    }
    if (strcmp(buffer, "VmHWM:") == 0) {
      fscanf(file, " %d", &peakRealMem);
    }
    if (strcmp(buffer, "VmSize:") == 0) {
      fscanf(file, " %d", &currVirtMem);
    }
    if (strcmp(buffer, "VmPeak:") == 0) {
      fscanf(file, " %d", &peakVirtMem);
    }
  }

  fclose(file);

  printf("Cur Real Mem: %d\tPeak Real Mem: %d "
         "\t Cur VirtMem: %d\tPeakVirtMem: %d\n",
         currRealMem, peakRealMem,
         currVirtMem, peakVirtMem);
}

void runQuery(char *term, char* genre) {
  SearchResultIter results = FindMovies(docIndex, term);
  printf("done searching\n");
  LinkedList movies = CreateLinkedList();

  if (results == NULL) {
    printf("No results for this term. Please try another.\n");
    DestroyLinkedList(movies, &NullFree);
    return;
  } else {
    SearchResult sr = (SearchResult)malloc(sizeof(*sr));
    if (sr == NULL) {
      printf("Couldn't malloc SearchResult in main.c\n");
      return;
    }
    int result;
    char *filename;

    // Get the first
    SearchResultGet(results, sr);
    filename = GetFileFromId(docs, sr->doc_id);

    Movie* movie;
    CreateMovieFromFileRow(filename, sr->row_id, &movie);
    InsertLinkedList(movies, (void*) movie);


    // Check if there are more
    while (SearchResultIterHasMore(results) != 0) {
      result =  SearchResultNext(results);
      if (result < 0) {
        printf("error retrieving result\n");
        break;
      }
      SearchResultGet(results, sr);
      char *filename = GetFileFromId(docs, sr->doc_id);
      Movie* movie;
      CreateMovieFromFileRow(filename, sr->row_id, &movie);
      InsertLinkedList(movies, (void*) movie);
    }

    free(sr);
    DestroySearchResultIter(results);
  }
  // Search for the movie which in crime genre
  printf("Searching for Crime movie which has the title of \"seattle\"\n");
  int count = 0;
  Movie* payload;
  LLIter iter = CreateLLIter(movies);
  LLIterGetPayload(iter, (void**) &payload);
  for (int i = 0; i < NUM_GENRES; i++) {
    if (payload->genres[i] == NULL)
      break;
    char buffer[20];
    strcpy(buffer, payload->genres[i]);
    Trim(buffer);
    if (!strcmp(buffer, genre)) {
      count++;
      break;
    }
  }

  while (LLIterHasNext(iter)) {
    LLIterNext(iter);
    LLIterGetPayload(iter, (void**) &payload);
    for (int i = 0; i < NUM_GENRES; i++) {
      if (payload->genres[i] == NULL)
        break;
      char buffer[20];
      strcpy(buffer, payload->genres[i]);
      Trim(buffer);
      if (!strcmp(buffer, genre)) {
        count++;
        break;
      }
    }
  }
  printf("The number of Crime movies which has the title of \"seattle\": %d\n", count);
  DestroyLLIter(iter);
  DestroyLinkedList(movies, &DestroyMovieWrapper);
}

void searchInTypeIndex(char* term, char* genre) {
  HTKeyValue kvp;
  printf("\n\n");
  uint64_t genre_key = FNVHash64((unsigned char*)genre, strlen(genre));
  LookupInHashtable(movie_index->ht, genre_key, &kvp);
  printf("Searching for Crime movie which has the title of \"seattle\"\n");
  LinkedList movie_list = ((SetOfMovies)kvp.value)->movies;

  int count = 0;
  Movie* payload;
  LLIter iter = CreateLLIter(movie_list);
  LLIterGetPayload(iter, (void**) &payload);
  if (strstr(payload->title, term))
    count++;

  while (LLIterHasNext(iter)) {
    LLIterNext(iter);
    LLIterGetPayload(iter, (void**) &payload);
    if (strstr(payload->title, term))
    count++;
  }
  printf("The number of Crime movies which has the title of \"seattle\": %d\n", count);
  DestroyLLIter(iter);
}

int main(int argc, char *argv[]) {
  // Check arguments
  if (argc != 2) {
    printf("Wrong number of arguments.\n");
    printf("usage: main <directory_to_crawl>\n");
    return 0;
  }
  pid_t pid = getpid();
  printf("Process ID: %d\n", pid);
  getMemory();

  // Create a DocIdMap
  docs = CreateDocIdMap();
  CrawlFilesToMap(argv[1], docs);
  printf("Crawled %d files.\n", NumElemsInHashtable(docs));
  printf("Created DocIdMap\n");

  getMemory();

  clock_t start2, end2;
  double cpu_time_used;


  // =======================
  // Benchmark MovieSet
  printf("\n\nBuilding the OffsetIndex\n");
  start2 = clock();
  BenchmarkMovieSet(docs);
  end2 = clock();
  cpu_time_used = ((double) (end2 - start2)) / CLOCKS_PER_SEC;
  printf("Took %f seconds to execute. \n", cpu_time_used);
  printf("Memory usage: \n");

  getMemory();

  // Searching "Seattle"
  printf("\n\nSearching term \"seattle\"\n");
  start2 = clock();
  runQuery("Seattle", "Crime");
  end2 = clock();
  cpu_time_used = ((double) (end2 - start2)) / CLOCKS_PER_SEC;
  printf("Took %f seconds to execute. \n", cpu_time_used);


  DestroyOffsetIndex(docIndex);
  printf("\n\nDestroyed OffsetIndex\n");
  getMemory();
  // =======================

  // ======================
  // Benchmark SetOfMovies
  printf("\n\nBuilding the GenreIndex\n");
  start2 = clock();
  BenchmarkSetOfMovies(docs);
  end2 = clock();
  cpu_time_used = ((double) (end2 - start2)) / CLOCKS_PER_SEC;
  printf("Took %f seconds to execute. \n", cpu_time_used);
  printf("Memory usage: \n");
  start2 = clock();
  searchInTypeIndex("Seattle", "Crime");
  end2 = clock();
  cpu_time_used = ((double) (end2 - start2)) / CLOCKS_PER_SEC;
  printf("Took %f seconds to execute. \n", cpu_time_used);
  printf("\n\n");
  getMemory();
  DestroyTypeIndex(movie_index);
  printf("Destroyed GenreIndex\n");
  getMemory();
  // ======================

  DestroyDocIdMap(docs);
  printf("\n\nDestroyed DocIdMap\n");
  getMemory();
}


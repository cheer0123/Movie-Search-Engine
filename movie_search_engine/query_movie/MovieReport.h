
#include <stdio.h>
#include <stdlib.h>

#include "MovieIndex.h"
#include "MovieSet.h"

#ifndef MOVIEREPORT_H
#define MOVIEREPORT_H

/**
 * MovieReport contains functions to generate and write out a report
 * of movies, based on an indexed set of movies (that is, and Index).
 */

/**
 * Prints a report to the terminal, given an index of movies.
 */
void PrintReport(Index index);

/**
 * Helper function; Prints just the movies in a set.
 *
 */
void OutputMovieSet(LinkedList movies, char* desc);

/**
 * Writes the report to the specified output FILE.
 */
void OutputReport(Index index, FILE* output);


/*
 * Writes the report to the specified output.
 */
void SaveReport(Index index, const char* filename);

void OutputSetOfMovies(SetOfMovies movie_set);

#endif   // MOVIEREPORT_H

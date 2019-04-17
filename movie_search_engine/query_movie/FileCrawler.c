/*
 *  TODO Tags finished by Yiming Zhao
 *  Date 2019-03-31
 *
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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#include "FileCrawler.h"
#include "DocIdMap.h"
#include "LinkedList.h"


void CrawlFilesToMap(const char *dir, DocIdMap map) {
  // struct stat s;
  struct dirent **namelist;
  int n;
  n = scandir(dir, &namelist, 0, alphasort);

  // TODO: use namelist to find all the files and put them in map.
  // NOTE: There may be nested folders.
  // Be sure to lookup how scandir works. Don't forget about memory use.
  if (n == -1) {
    char* finalDest = malloc(sizeof(char) * (strlen(dir) + 1));
    // printf("%ld\n", strlen(dir));
    strcpy(finalDest, dir);
    // printf("%s\n", finalDest);
    PutFileInMap(finalDest, map);
    return;
  }
  char buffer1[50];
  char buffer2[10];
  for (int i = 0; i < n; i++) {
    strcpy(buffer2, namelist[i]->d_name);
    free(namelist[i]);
    if (!strcmp(".", buffer2) || !strcmp("..", buffer2))
      continue;
    snprintf(buffer1, sizeof(buffer1), "%s%c%s", dir, '/', buffer2);
    CrawlFilesToMap(buffer1, map);
  }
  free(namelist);
}

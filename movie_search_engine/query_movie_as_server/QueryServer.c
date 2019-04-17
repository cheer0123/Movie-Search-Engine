#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>


#include "QueryProtocol.h"
#include "MovieSet.h"
#include "MovieIndex.h"
#include "DocIdMap.h"
#include "htll/Hashtable.h"
#include "QueryProcessor.h"
#include "FileParser.h"
#include "FileCrawler.h"

DocIdMap docs;
Index docIndex;

#define BUFFER_SIZE 1000
#define SEARCH_RESULT_LENGTH 1500
char movieSearchResult[SEARCH_RESULT_LENGTH];

int Cleanup();

void sigint_handler(int sig) {
  write(0, "Exit signal sent. Cleaning up...\n", 34);
  Cleanup();
  exit(0);
}


void Setup(char *dir) {
  printf("Crawling directory tree starting at: %s\n", dir);
  // Create a DocIdMap
  docs = CreateDocIdMap();
  CrawlFilesToMap(dir, docs);
  printf("Crawled %d files.\n", NumElemsInHashtable(docs));

  // Create the index
  docIndex = CreateIndex();

  // Index the files
  printf("Parsing and indexing files...\n");
  ParseTheFiles(docs, docIndex);
  printf("%d entries in the index.\n", NumElemsInHashtable(docIndex->ht));
}

int Cleanup() {
  DestroyOffsetIndex(docIndex);
  DestroyDocIdMap(docs);
  return 0;
}

int singleConnection(int client_fd) {
  // send ACK after connected
  SendAck(client_fd);

  // read query 
  char resp[BUFFER_SIZE];
  int len= read(client_fd, resp, sizeof(resp)-1);
  resp[len] = '\0';
  printf("receive %s\n", resp);


  SearchResultIter results = FindMovies(docIndex, resp);

  if (results == NULL) {
    printf("No result.\n");
    int numResponse = 0;
    write(client_fd, &numResponse, sizeof(numResponse));

    // read ACK after send numResponse
    len = read(client_fd, resp, 999);
    resp[len] = '\0';
    if (CheckAck(resp) == -1) {
      printf("Read ACK failed!\n");
      return 0;
    }

    SendGoodbye(client_fd);
    close(client_fd);
    return 0;
  } else {
    int numResponse = results->numResults;
    printf("numbers of response: %d\n", numResponse);
    write(client_fd, &numResponse, sizeof(numResponse));

    len = read(client_fd, resp, 999);
    resp[len] = '\0';
    if (CheckAck(resp) == -1) {
      printf("Read ACK failed!\n");
      return 0;
    }

    SearchResult sr = (SearchResult)malloc(sizeof(*sr));
    if (sr == NULL) {
      printf("Coundn't malloc searchResult\n");
      SendGoodbye(client_fd);
      close(client_fd);
      return 0;
    }

    int result;
    SearchResultGet(results, sr);
    CopyRowFromFile(sr, docs, movieSearchResult);
    write(client_fd, movieSearchResult, strlen(movieSearchResult));

    int len = read(client_fd, resp, 999);
    resp[len] = '\0';
    while (CheckAck(resp) == 0 && SearchResultIterHasMore(results) != 0) {
      result = SearchResultNext(results);
      if (result < 0) {
        printf("error retrieving result\n");
        break;
      }

      SearchResultGet(results, sr);
      CopyRowFromFile(sr, docs, movieSearchResult);
      write(client_fd, movieSearchResult, strlen(movieSearchResult));

      int len = read(client_fd, resp, 999);
      resp[len] = '\0';
    }

    free(sr);
    DestroySearchResultIter(results);
  }

  // SendGoodbye
  SendGoodbye(client_fd);
  printf("client connection closed\n");
  close(client_fd);
  return 0;
}


int HandleConnections(int sock_fd) {
  while (1) {
    int client_fd = accept(sock_fd, NULL, NULL);
    printf("Connection made: client_fd=%d\n", client_fd);
    singleConnection(client_fd);
  }

  return 0;
}

int main(int argc, char **argv) {
  // Check/get arguments
  if (argc != 3) {
    printf("Invalid input, please put dir and port.");
    exit(1);
  }

  // Get args
  char *dir_to_crawl = argv[1];
  char *port_string = argv[2];

  // Setup graceful exit
  Setup(dir_to_crawl);
  struct sigaction kill;

  kill.sa_handler = sigint_handler;
  kill.sa_flags = 0;  // or SA_RESTART
  sigemptyset(&kill.sa_mask);

  if (sigaction(SIGINT, &kill, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  // Step 1: get address/port info to open
  struct addrinfo hints, *result;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  int getRes = getaddrinfo(NULL, port_string, &hints, &result);
  if (getRes < 0) {
    printf("getaddrinfo failed\n");
    return -1;
  }

  // Step 2: Open socket
  int sock_fd = socket(result->ai_family, result->ai_socktype, 0);

  // Step 3: Bind socket
  if (bind(sock_fd, result->ai_addr, result->ai_addrlen) != 0) {
    perror("bind()");
    exit(1);
  }

  // Step 4: Listen on the socket
  if (listen(sock_fd, 10) != 0) {
    perror("listen()");
    exit(1);
  }
  struct sockaddr_in *result_addr = (struct sockaddr_in *) result->ai_addr;

  printf("Listening on file descriptor %d, port %d\n",
    sock_fd, ntohs(result_addr->sin_port));
  printf("Waiting for connection...\n");
  freeaddrinfo(result);

  // Step 5: Handle clients that connect
  HandleConnections(sock_fd);

  // Step 6: Close the socket
  close(sock_fd);
  Cleanup();

  return 0;
}

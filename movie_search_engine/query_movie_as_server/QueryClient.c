#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "includes/QueryProtocol.h"
#include "queryclient.h"

char *port_string;
unsigned short int port;
char *ip;

#define BUFFER_SIZE 1000
#define RESULT_SIZE 1500


void RunQuery(char *query) {
  // Find the address
  struct addrinfo hints, *result;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  printf("port_string %s\n", port_string);
  printf("ip %s\n", ip);

  int s = getaddrinfo(ip, port_string, &hints, &result);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
  }

  // Create the socket
  int sock_fd = socket(result->ai_family, result->ai_socktype, 0);

  if (connect(sock_fd, result->ai_addr, result->ai_addrlen) == -1) {
    perror("Connect falied\n");
    exit(2);
  } else {
    printf("connection is good!\n");
  }

  freeaddrinfo(result);

  // Do the query-protocol
  // read ACK 
  char buffer[BUFFER_SIZE];
  int len = read(sock_fd, buffer, 999);
  buffer[len] = '\0';
  if (len == -1) {
    printf("Read ACK failed!\n");
    return;
  }

  // if ACK correct, send query
  if (CheckAck(buffer) == 0) {
    printf("connected to server.\n");
    write(sock_fd, query, strlen(query));
  }

  int numResponse = 0;
  read(sock_fd, &numResponse, sizeof(numResponse) -1);
  printf("numbers of response: %d\n",numResponse);
  SendAck(sock_fd);


  len = read(sock_fd, buffer, 999);
  buffer[len] = '\0';
  if (len == -1) {
    printf("Read ACK failed!\n");
    return;
  }

  while (strcmp(buffer, "GOODBYE") != 0) {
    printf("%s\n", buffer);
    SendAck(sock_fd);

    len = read(sock_fd, buffer, 999);
    buffer[len] = '\0';
  }

  close(sock_fd);
}

void RunPrompt() {
  char input[BUFFER_SIZE];

  while (1) {
    printf("Enter a term to search for, or q to quit: ");
    scanf("%s", input);

    printf("input was: %s\n", input);

    if (strlen(input) == 1) {
      if (input[0] == 'q') {
        printf("Thanks for playing! \n");
        return;
      }
    }
    printf("\n\n");
    RunQuery(input);
  }
}

int main(int argc, char **argv) {
  // Check/get arguments
  if (argc != 3) {
    printf("Input is invalid, please input address and port!");
    exit(1);
  }

  // Get info from user
  ip = argv[1];
  port_string = argv[2];

  // Run Query
  RunPrompt();
  return 0;
}

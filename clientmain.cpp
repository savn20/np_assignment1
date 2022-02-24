#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

// Included to get the support library
#include <calcLib.h>

// Enable if you want debugging to be printed, see examble below.
// Alternative, pass CFLAGS=-DDEBUG to make, make CFLAGS=-DDEBUG
#define DEBUG
// Remove comment to add 10s delay to client
// #define DELAY
#define MAXDATASIZE 1400
#define SERVER_VERSION "TEXT TCP 1.0\n\n"

using namespace std;

int main(int argc, char *argv[]) {
  // disables debugging when there's no DEBUG macro defined
#ifndef DEBUG
  cout.setstate(ios_base::failbit);
  cerr.setstate(ios_base::failbit);
#endif

  /*
    Read first input, assumes <ip>:<port> syntax, convert into one string (Desthost) and one integer (port). 
     Atm, works only on dotted notation, i.e. IPv4 and DNS. IPv6 does not work if its using ':'. 
  */

  if (argc != 2) {
    cerr << "usage: client <ip>:<port>\n"
         << "program terminated due to wrong usage" << endl;

    exit(-1);
  }

  char delim[] = ":";
  char *serverIp = strtok(argv[1], delim);
  char *serverPort = strtok(NULL, delim);

  int sockFd, rv, byteSize;
	struct addrinfo hints, *servinfo, *ptr;
	char buffer[MAXDATASIZE];
	char s[INET6_ADDRSTRLEN];

  timeval timeout;
  timeout.tv_sec = 5;
  timeout.tv_usec = 0;

  memset(&hints, 0, sizeof hints);

  /******************/
  /* server setup  */
  /****************/
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

  verify((rv = getaddrinfo(serverIp, serverPort, &hints, &servinfo)) != 0);

	for(ptr = servinfo; ptr != NULL; ptr = ptr->ai_next) {
    // creating socket
		if((sockFd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == -1) {
			cerr << "talker: socket\n";
			continue;
		}

		break;
	}

  if (ptr == NULL) {
		cerr << "talker: failed to create socket\n";
    exit(-1);
	}

  // connecting to established socket
  verify(connect(sockFd,ptr->ai_addr, ptr->ai_addrlen));

  inet_ntop(ptr->ai_family, getSocketAddress((struct sockaddr *)ptr->ai_addr),
		  s, sizeof s);
	
  printf("Host %s and port %s\n", s, serverPort);
	freeaddrinfo(servinfo);

  setsockopt(sockFd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof timeout);


  /************************************/
  /*  getting verison from server    */
  /**********************************/
  verify(byteSize = recv(sockFd, buffer, MAXDATASIZE-1, 0));
  buffer[byteSize] = '\0';
	
  cout << "Server running with version: " << buffer;

  if(strcmp(buffer, SERVER_VERSION) != 0){
    cerr << "invalid version\n";
  }

  printf("Sending OK\n");
  verify(send(sockFd, "OK\n", 3, 0));
  cout << "connected to " << s << ":" << serverPort << endl;

  /*************************************/
  /*  server gives task to perform    */
  /***********************************/
  verify(byteSize = recv(sockFd, buffer, MAXDATASIZE-1, 0));
  buffer[byteSize] = '\0';

  printf("ASSIGNMENT: %s\n", buffer);

  auto result = calculateTask(strtok(buffer, "\n"));
  cout << "Calculated the result to " << result->result;

#ifdef DELAY
  sleep(10);
#endif

  verify(send(sockFd, result->result, strlen(result->result), 0));

  verify(byteSize = recv(sockFd, buffer, MAXDATASIZE-1, 0));
  buffer[byteSize] = '\0';

  if(strcmp(buffer, "OK\n") != 0){
    if(strcmp(buffer, "ERROR\n") != 0){
      printf("ERROR TO\n");
    }
    else{
      printf("%s", buffer);
    }

    close(sockFd);
    exit(-1);
  }
  
  printf("OK (myresult=%s)\n", strtok(result->result, "\n"));

  close(sockFd);
  return 0;
}
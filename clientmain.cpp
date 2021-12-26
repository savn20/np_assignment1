#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
/* You will to add includes here */

#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

// Enable if you want debugging to be printed, see examble below.
// Alternative, pass CFLAGS=-DDEBUG to make, make CFLAGS=-DDEBUG
#define DEBUG
#define BUFFER_SIZE 1000
#define SERVER_VERSION "TEXT TCP 1.0"

// Included to get the support library
#include <calcLib.h>

// stores response from server
char response[BUFFER_SIZE];

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

  if (argc != 2)
  {
    cerr << "usage: server <ip>:<port>\n"
         << "program terminated due to wrong usage" << endl;

    exit(-1);
  }

  char delim[] = ":";
  char *serverIp = strtok(argv[1], delim);
  int serverPort = atoi(strtok(NULL, delim));

  int socketConnection = -1;
  
  sockaddr_in serverAddress;
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(serverPort);
  serverAddress.sin_addr.s_addr = inet_addr(serverIp);

  timeval timeout;
  timeout.tv_sec = 5;

  // creating socket
  if ((socketConnection = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    cerr << "client: failed to create socket\n"
         << "program terminated while socket()" << endl;

    exit(-1);
  }

  cout << "establishing connection to given host:" << serverIp << " and port: " << serverPort << endl;

  // connecting to socket to ip and port
  if (connect(socketConnection, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
  {
    close(socketConnection);
    cerr << "error: failed to connect to socket\n"
         << "program terminated while connect()" << endl;

    exit(-1);
  }

  setsockopt(socketConnection, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);
  setsockopt(socketConnection, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof timeout);

  cout << "client: socket created, waiting for server response..." << endl;

  /************************************/
  /*  getting verison from server    */
  /**********************************/
  if ((recv(socketConnection, response, sizeof(response), 0)) <= 0){
    cerr << "error: no bytes rec from server\n"
         << "program terminated while recv()" << endl;
    close(socketConnection);
    exit(-1);
  }

  cout << "server: ", strtok(response, "\n");
  
  if(strcmp(strtok(response, "\n"), SERVER_VERSION) == 0){
        
    if (send(socketConnection, "OK\n", 3, 0) < 0)
    {
      cerr << "error: failed to send message\n"
           << "program terminated while send()" << endl;
      close(socketConnection);
      exit(-1);
    }

    printf("connected to %s and port %d\n",serverIp,serverPort);
  }
  else{
    printf("the version the server operates is invalid: %s\n",response);
    exit(-1);
  }

  /*************************************/
  /*  server gives task to perform    */
  /***********************************/
  if ((recv(socketConnection, response, sizeof(response), 0)) <= 0){
    cerr << "error: no bytes rec from server\n"
         << "program terminated while recv()" << endl;
    close(socketConnection);
    exit(-1);
  }

  printf("server: %s\n", strtok(response, "\n"));

  auto result = calculateTask(strtok(response, "\n"));

  if (send(socketConnection, result->result, sizeof(result->result), 0) < 0)
  {
    cerr << "error: failed to send message\n"
         << "program terminated while send()" << endl;
    close(socketConnection);
    exit(-1);
  }

  printf("client: calculated %s", result->result);

  /*************************************/
  /*  server verifies the result      */
  /***********************************/
  if ((recv(socketConnection, response, sizeof(response), 0)) <= 0){
    cerr << "error: no bytes rec from server\n"
         << "program terminated while recv()" << endl;
    close(socketConnection);
    exit(-1);
  }

  printf("server: %s\n", strtok(response, "\n"));
  
  cout << "closing connection..." << endl;

  close(socketConnection);

  return 0;
}
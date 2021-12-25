#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

// helper library for calculations
#include "calcLib.h"

// comment the DEBUG macro to turn off comments in the console
#define DEBUG
#define BACKLOG 5

using namespace std;

int main(int argc, char *argv[])
{
  // disables debugging when there's no DEBUG macro defined
#ifndef DEBUG
  cout.setstate(ios_base::failbit);
  cerr.setstate(ios_base::failbit);
#endif

  /*
   * parses command line input to <ip>:<port>
   * terminates program if there's mismatch
   */

  if (argc != 2)
  {
    cerr << "usage: server <ip>:<port>\n"
         << "program terminated due to wrong usage" << endl;

    exit(-1);
  }

  char seperator[] = ":";
  string serverIp = strtok(argv[1], seperator);
  string destPort = strtok(NULL, seperator);

  int serverPort = atoi(destPort.c_str());

  int serverSocket = -1,
      clientSocket = -1,
      acceptMultipleClients = 1;

  printf("server at given host:%s and port:%d\n", serverIp.c_str(), serverPort);

  // setting up address metadata
  sockaddr_in serverAddr, clientAddr;
  socklen_t clientAddrLen = sizeof(struct sockaddr_in);

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(serverPort);
  serverAddr.sin_addr.s_addr = inet_addr(serverIp.c_str());

  // creating socket
  if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    cerr << "server: failed to create socket\n"
         << "program terminated while socket()" << endl;

    exit(-1);
  }

  // flag to accept multiple clients
  if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &acceptMultipleClients,
                 sizeof(int)) < 0)
  {
    cerr << "server: failed to accept multiple connections\n"
         << "program terminated while setsockopt()" << endl;
    close(serverSocket);
    exit(-1);
  }

  // binding socket to ip and port
  if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
  {
    close(serverSocket);
    cerr << "error: failed to bind socket\n"
         << "program terminated while bind()" << endl;

    exit(-1);
  }

  if (listen(serverSocket, BACKLOG) < 0)
  {
    cerr << "error: failed to listen to socket\n"
         << "program terminated while listen()" << endl;
    exit(-1);
  }

  cout << "server is ready, listening for clients..." << endl;

  char buffer[80];

  for (int i = 0; i < BACKLOG; i++)
  {

    //TODO: set timeout
    clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (clientSocket < 0)
    {
      cerr << "error: can't accept client\n"
           << "program terminated while accept()" << endl;
      close(serverSocket);
      exit(-1);
    }

    cout << "server: new client connected at host: " << inet_ntoa(clientAddr.sin_addr) << " and port: " << ntohs(clientAddr.sin_port) << endl;

    cout << "server: sending version - TEXT TCP 1.0" << endl;

    string message = "TEXT TCP 1.0\n\n";

    // sending the version server accepts
    if (send(clientSocket, message.c_str(), strlen(message.c_str()), 0) < 0)
    {
      cerr << "error: failed to send message to client\n"
           << "program terminated while send()" << endl;
      close(clientSocket);
      exit(-1);
    }

    // response from server: OK! (in most cases)
    if ((recv(clientSocket, buffer, sizeof(buffer), 0)) <= 0)
    {
      cerr << "error: no bytes rec from client\n"
           << "program terminated while recv()" << endl;
      close(serverSocket);
      close(clientSocket);
      exit(-1);
    }

    printf("client: %s\n", buffer);

    //TODO: Assign a random task
    message = "add 2 3 \n\n";

    // sending the assignment: <operation> <value1> <value2>
    if (send(clientSocket, message.c_str(), strlen(message.c_str()), 0) <= 0)
    {
      cerr << "error: failed to send message to client\n"
           << "program terminated while send()" << endl;
      close(serverSocket);
      close(clientSocket);
      exit(-1);
    }

    cout << "server: add 2 3" << endl;

    // result from client
    if ((recv(clientSocket, buffer, sizeof(buffer), 0)) <= 0)
    {
      cerr << "error: no bytes rec from client\n"
           << "program terminated while recv()" << endl;
      close(serverSocket);
      close(clientSocket);
      exit(-1);
    }

    printf("client: %s\n", buffer);

    //TODO: Compare the result
    message = "OK\n\n";

    // sending the result of calculated operation
    if (send(clientSocket, message.c_str(), strlen(message.c_str()), 0) <= 0)
    {
      cerr << "error: failed to send message to client\n"
           << "program terminated while send()" << endl;
      close(serverSocket);
      close(clientSocket);
      exit(-1);
    }

    cout << "server: OK" << endl;

    // closing the client bye!!
    close(clientSocket);
  }

  close(serverSocket);

  return 0;
}

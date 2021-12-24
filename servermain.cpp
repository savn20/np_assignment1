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
#define QUEUE 5

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

    return -1;
  }

  char seperator[] = ":";
  string serverIp = strtok(argv[1], seperator);
  string destPort = strtok(NULL, seperator);

  int serverPort = atoi(destPort.c_str());

  int serverSocket = -1,
      clientSocket = -1,
      acceptMultipleClients = 1;

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
         << "program terminated while creating socket" << endl;

    return -3;
  }

  // flag to accept multiple clients
  if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &acceptMultipleClients,
                 sizeof(int)) < 0)
  {
    cerr << "server: failed to accept multiple connections\n"
         << "program terminated while setsockopt" << endl;
    
    exit(1);
  }

  // binding socket to ip and port
  if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
  {
    close(serverSocket);
    cerr << "error: failed to bind socket\n"
         << "program terminated while binding socket" << endl;

    return -4;
  }

  if (listen(serverSocket, QUEUE) < 0)
  {
    cerr << "error: failed to listen to socket\n";
    return -5;
  }

  cout << "listening for clients..." << endl;

  clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);

  //TODO: handle 5 clients
  if (clientSocket < 0)
  {
    cerr << "error: can't accept client\n";
    return -6;
  }

  cout << "client connected at IP: " << inet_ntoa(clientAddr.sin_addr) << " and port: " << ntohs(clientAddr.sin_port) << endl;

  string message = "TEXT TCP 1.0\n\n";

  // sending the version server accepts
  if (send(clientSocket, message.c_str(), strlen(message.c_str()), 0) < 0)
  {
    cout << "can't send" << endl;
    return -7;
  }

  char client_message[2000];

  // receiving client's message:
  if (recv(clientSocket, &client_message, sizeof(client_message), 0) < 0)
  {
    cerr << "couldn't receive" << endl;
    return -8;
  }

  cout << "message from client: " << client_message << endl;

  // TODO: Assign random task
  message = "add 2 3\n\n";

  if (send(clientSocket, message.c_str(), strlen(message.c_str()), 0) < 0)
  {
    cout << "can't send" << endl;
    return -7;
  }

  if (recv(clientSocket, &client_message, sizeof(client_message), 0) < 0)
  {
    cerr << "couldn't receive\n";
    return -8;
  }

  cout << "message from client: " << client_message << endl;

  return 0;
}

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
  string serverPort = strtok(NULL, seperator);

  int serverSocket = -1, clientSocket = -1, responseBytes = -1;

  sockaddr_in clientAddr;
  socklen_t clientAddrLen = sizeof(struct sockaddr_in);

  // setting up address metadata
  addrinfo addressInfo, *serverAddr;
  memset(&addressInfo, 0, sizeof addressInfo);

  addressInfo.ai_family = AF_INET;
  addressInfo.ai_socktype = SOCK_STREAM;
  addressInfo.ai_flags = AI_PASSIVE;

  if (responseBytes = getaddrinfo(serverIp.c_str(), serverPort.c_str(), &addressInfo, &serverAddr) != 0)
  {
    cerr << "error: unable to connect to specified host\n"
         << "program terminated due to host error" << endl;

    return -2;
  }

  // creating socket
  if ((serverSocket = socket(serverAddr->ai_family, serverAddr->ai_socktype, serverAddr->ai_protocol)) == -1)
  {
    cerr << "error: failed to create socket\n"
         << "program terminated while creating socket" << endl;

    return -3;
  }

  // Bind to the set port and IP:
  if (bind(serverSocket, serverAddr->ai_addr, serverAddr->ai_addrlen) < 0)
  {
    cerr << "error: failed to bind socket\n"
         << "program terminated while binding socket" << endl;

    return -4;
  }

  freeaddrinfo(serverAddr);

  if (listen(serverSocket, QUEUE) < 0)
  {
    cerr << "error: failed to bind socket\n";
    return -5;
  }

  cout << "Listening for incoming connections.....";

  //TODO: handle 5 clients
  if (clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen) < 0)
  {
    cerr << "error: can't accept client\n";
    return -6;
  }

  cout << "client connected at IP: " << inet_ntoa(clientAddr.sin_addr) << "and port: " << ntohs(clientAddr.sin_port) << endl;

  string version = "TEXT TCP 1.0\n\n";

  if (sendto(clientSocket, version.c_str(), strlen(version.c_str()), 0,
             (struct sockaddr *)NULL, clientAddrLen) < 0)
  {
    cout << "can't send" << endl;
    return -7;
  }

  char client_message[2000];

  // Receive client's message:
  if (recv(clientSocket, &client_message, sizeof(client_message), 0) < 0)
  {
    cerr << "couldn't receive\n";
    return -8;
  }

  cout << "message from client: " << client_message << endl;

  return 0;
}

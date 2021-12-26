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
#define BUFFER 1000

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
  timeval timeout;
  timeout.tv_sec = 5;

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

  // buffer to store client message and send server response
  string response("");
  char message[BUFFER];

  for (int i = 0; i < BACKLOG; i++)
  {

    clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (clientSocket < 0)
    {
      cerr << "error: can't accept client\n"
           << "program terminated while accept()" << endl;
      close(serverSocket);
      exit(-1);
    }

    //set timeout
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);
    setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof timeout);

    cout << "server: new client connected at host: " << inet_ntoa(clientAddr.sin_addr) << " and port: " << ntohs(clientAddr.sin_port) << endl;

    cout << "server: sending version - TEXT TCP 1.0" << endl;

    response = "TEXT TCP 1.0\n\n";

    // sending the version server accepts
    if (send(clientSocket, response.c_str(), strlen(response.c_str()), 0) < 0)
    {
      cerr << "error: failed to send message to client\n"
           << "program terminated while send()" << endl;
      close(clientSocket);
      exit(-1);
    }

    // response from server: OK! (in most cases)
    if ((recv(clientSocket, message, sizeof(message), 0)) <= 0)
    {
      cerr << "error: no bytes rec from client\n"
           << "program terminated while recv()" << endl;
      close(serverSocket);
      close(clientSocket);
      exit(-1);
    }
    else
    {
      printf("client: %s\n", strtok(message, "\n"));
    }

    calcTask *task = randomTask();
    
    response = task->task;

    // sending the assignment: <operation> <value1> <value2>
    if (send(clientSocket, response.c_str(), strlen(response.c_str()), 0) <= 0)
    {
      cerr << "error: failed to send message to client\n"
           << "program terminated while send()" << endl;
      close(serverSocket);
      close(clientSocket);
      exit(-1);
    }
    else
    {
      cout << "server: " << strtok(task->task, "\n") << endl;
    }

    // result from client
    if ((recv(clientSocket, message, sizeof(message), 0)) <= 0)
    {
      cerr << "error: no bytes rec from client\n"
           << "program terminated while recv()" << endl;
      close(serverSocket);
      close(clientSocket);
      exit(-1);
    }

    printf("client: %s\n", strtok(message, "\n"));

    if(response[0] == 'f'){
      float res = stof(strtok(message, "\n"));
      res = abs(res - task->fResult);
      
      if(res < 0.0001){
        response = "OK\n";
        printf("server: OK\n");
      }
      else{
        response = "ERROR\n";
        printf("server: ERROR\n");
      }
    }
    else{
      int res = atoi(strtok(message, "\n"));
      
      if(res == task->iResult){
        response = "OK\n";
        printf("server: OK\n");
      }
      else{
        response = "ERROR\n";
        printf("server: ERROR\n");
      }
    }

    // sending the result of calculated operation
    if (send(clientSocket, response.c_str(), strlen(response.c_str()), 0) <= 0)
    {
      cerr << "error: failed to send message to client\n"
           << "program terminated while send()" << endl;
      close(serverSocket);
      close(clientSocket);
      exit(-1);
    }

    // closing the client bye!!
    close(clientSocket);
  }

  close(serverSocket);

  return 0;
}

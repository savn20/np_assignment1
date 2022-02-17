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
#define MAXDATASIZE 1400
#define BACKLOG 5
#define SERVER_VERSION "TEXT TCP 1.0\n\n"

using namespace std;

int totalClients = 0;

void handleTimeout(int hasError, int sockFd){
  if (hasError == SOCKET_FAILURE) {
    perror("error: closing on timeout");
    
    char *errorMsg = (char *)malloc(MAXDATASIZE);
    errorMsg = strdup("ERROR TO\n");

    send(sockFd,&errorMsg,strlen(errorMsg),0);
    close(sockFd);
    free(errorMsg);
    
    exit(-1);
  }
}

void serveClient(int clientFd){
  char *readBuffer = (char *)malloc(MAXDATASIZE);
  char *writeBuffer = (char *)malloc(MAXDATASIZE);
  int bytes = 0;

  cout << "Im client " << totalClients << "\n";
  
  /* sending version */
  printf("server: sending version 1.0\n");
  writeBuffer = strdup("TEXT TCP 1.0\n\n");
  verify(send(clientFd, writeBuffer, strlen(writeBuffer), 0));
  
  handleTimeout(bytes = recv(clientFd, readBuffer, MAXDATASIZE, 0), clientFd);
  readBuffer[bytes] = '\0';
  printf("client: %s\n", readBuffer);

  /* task */
  calcTask *task = randomTask();
  printf("server: %s", task->task);
  writeBuffer = task->task;
  verify(send(clientFd,  writeBuffer, strlen(writeBuffer), 0));

  handleTimeout(recv(clientFd, readBuffer, MAXDATASIZE, 0), clientFd);
  printf("client: %s\n", strtok(readBuffer, "\n"));

  if (writeBuffer[0] == 'f') {
      double res = stod(strtok(readBuffer, "\n"));
      res = abs(res - task->fResult);

      if (res < 0.0001) {
        writeBuffer = strdup("OK\n");
        printf("server: OK\n");
      }
      else {
        writeBuffer = strdup("ERROR\n");
        printf("server: ERROR\n");
      }
  }
  
  else {
    int res = atoi(strtok(readBuffer, "\n"));

    if (res == task->iResult) {
      writeBuffer = strdup("OK\n");
      printf("server: OK\n");
    }
    else
    {
      writeBuffer = strdup("ERROR\n");
      printf("server: ERROR\n");
    }
  }

  verify(send(clientFd, writeBuffer, strlen(writeBuffer), 0));
  free(writeBuffer);
  free(readBuffer);
  return;
}

int main(int argc, char *argv[]) {

  /* disables debugging if DEBUG is not defined */
#ifndef DEBUG
  cout.setstate(ios_base::failbit);
  cerr.setstate(ios_base::failbit);
#endif

  /* parses arguments to <ip>:<port> terminates program if there's mismatch */
  if (argc != 2) {
    cerr << "usage: server <ip>:<port>\n"
         << "program terminated due to wrong usage" << endl;

    exit(-1);
  }

  char seperator[] = ":";
  char cli[INET6_ADDRSTRLEN]; 
  string serverIp = strtok(argv[1], seperator);
  string destPort = strtok(NULL, seperator);

  int serverPort = atoi(destPort.c_str());
  int listenFd = -1, // for server socket
      acceptFd = -1, // for client socket
      reuseAddress = 1,
      pid;

  struct sockaddr_in serverAddress; // server receive on this address
  struct sockaddr_in clientAddress; // server sends to client on this address

  timeval timeout;
  timeout.tv_sec = 5;
  timeout.tv_usec = 0;

  verify(listenFd = socket(AF_INET, SOCK_STREAM, 0));

  /* initialize the socket addresses */
  memset(&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(serverPort);
  serverAddress.sin_addr.s_addr = inet_addr(serverIp.c_str());

  socklen_t clientAddressLength = sizeof(struct sockaddr_in);

  /* bind the socket with the server address and port */
  verify(setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &reuseAddress, sizeof(int)));
  
  /* bind the socket with the server address and port */
  verify(bind(listenFd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)));

  /* listen for connection from client */
  verify(listen(listenFd, BACKLOG));

  while (1) {
    // parent process waiting to accept a new connection
    cout << "\n*****server waiting for new client connection:*****\n";
    clientAddressLength = sizeof(clientAddress);
    acceptFd = accept(listenFd, (struct sockaddr *)&clientAddress, &clientAddressLength);
    totalClients++;
    
    cout << "accept = " << acceptFd << " listenFd = " << listenFd << endl;

    // set timeout
    setsockopt(acceptFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);
    setsockopt(acceptFd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof timeout);

    /* child process is created for serving each new clients */
    pid = fork();
    
    if(pid == 0) {
      close(listenFd); // sock is closed BY child
      getIpAddress((struct sockaddr *)&clientAddress, cli, 1);
      cout << "client connection from host: " << cli << " port: " << ntohs(clientAddress.sin_port) << endl;
      serveClient(acceptFd);
      close(acceptFd);
      exit(0);
    } 
    
    cout << "Parent, close acceptFd().\n";
    close(acceptFd); // sock is closed BY PARENT
  } // close exterior while

  return 0;
}
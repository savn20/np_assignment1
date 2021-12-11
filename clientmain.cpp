#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
#define ERROR "ERROR"
#define OK "OK"

// Included to get the support library
#include <calcLib.h>

void performOperation(char* operation);

int performCalculation(char* op,int a, int b);

float performCalculation(char* op,float a, float b);

char message[BUFFER_SIZE];

int main(int argc, char *argv[]){

  /*
    Read first input, assumes <ip>:<port> syntax, convert into one string (Desthost) and one integer (port). 
     Atm, works only on dotted notation, i.e. IPv4 and DNS. IPv6 does not work if its using ':'. 
  */
  char delim[]=":";
  char *Desthost=strtok(argv[1],delim);
  char *Destport=strtok(NULL,delim);
  // *Desthost now points to a sting holding whatever came before the delimiter, ':'.
  // *Dstport points to whatever string came after the delimiter. 

  char buffer[BUFFER_SIZE];
  // buffer stores the responses recieved from server

  char *operation=buffer;

  char *version;
  // *version will be used to store server version

  /* Do magic */
  int port=atoi(Destport);
#ifdef DEBUG 
  printf("Host %s, and port %d.\n",Desthost,port);
#endif

  struct addrinfo ip_metadata, *p;
  // ip_metadata now stores address information needed to communicate with TCP/IP server
  memset(&ip_metadata, 0, sizeof(ip_metadata));
  ip_metadata.ai_family   = AF_UNSPEC;
  ip_metadata.ai_socktype = SOCK_STREAM;
  ip_metadata.ai_flags    = AI_PASSIVE;

  int addr_meta = getaddrinfo(Desthost, Destport, &ip_metadata, &p);
  // addr_meta returns the connection status with error code
  // if error code == 0, then the connection is established
  if (addr_meta != 0) {
#ifdef DEBUG
    printf("Something wrong with address, here's the error code %d:\n",addr_meta);
    printf("\t%s",gai_strerror(addr_meta));
#endif
    return -2;
  }

  if (p == NULL) {
#ifdef DEBUG
    printf("No addresses found\n");
#endif
    return -3;
  }

  int sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
  // sockfd stores the information about incoming connection
  if (sockfd == -1) {
#ifdef DEBUG
    printf("Oops! failed to create socket\n");
    return -4;
#endif
  }

  int socket_connection = connect(sockfd, p->ai_addr, p->ai_addrlen);
  // connects to the socket specified i.e sockfd
  if (socket_connection == -1) {
    close(sockfd);
#ifdef DEBUG
  printf("Oops! failed to connect with socket\n");
#endif
    return -5;
  }

#ifdef DEBUG 
  printf("Establishing Connection to %s...\n",Desthost);
#endif

  int bytes_recv = recv(sockfd, buffer, BUFFER_SIZE-1, 0);
  if (bytes_recv == -1) {
#ifdef DEBUG 
  printf("Oops! No bytes received, Terminating the program\n");
#endif
    exit(1);
  }

  version = strtok(buffer, "\n");
  
  if(strcmp(version, SERVER_VERSION) == 0){
    send(sockfd, "OK\n", 3, 0);
#ifdef DEBUG
    printf("Server: %s\n", version);
    printf("Connected to %s and port %d\n",Desthost,port);
#endif    
  }

  while(operation != NULL){
    bytes_recv = recv(sockfd, buffer, BUFFER_SIZE-1, 0);
    
    if (bytes_recv == -1) {
#ifdef DEBUG 
    printf("Oops! No bytes received, Terminating the program\n");
#endif
    exit(1);
    }
    
    operation = strtok(buffer, "\n");

#ifdef DEBUG 
    printf("Server: %s\n", operation);
#endif
  
    if(strcmp(operation, ERROR) == 0 || strcmp(operation, OK) == 0){
      operation = strtok(NULL, "\n");
      continue;
    }

    performOperation(operation);
#ifdef DEBUG
    printf("Calculated: %s", message);
#endif
    send(sockfd, message, strlen(message), 0);
  }

  printf("Closing connection...\n");
  close(sockfd);
}

void performOperation(char* opstring){
  int count = 0;
  char *operation, *var1, *var2;
  char *token = strtok(opstring, " ");   
  
  while (token != NULL){
    switch (count){
      case 0:
        operation = token;
      case 1:
        var1 = token;
      case 2:
        var2 = token;
    default:
      break;
    }
    token = strtok(NULL, " ");
    count++;
  }


  if(operation[0] == 'f'){
    sprintf(message,"%8.8f\n", performCalculation(operation, (float) atof(var1), (float) atof(var2)));
  }
  else{
    sprintf(message,"%d\n", performCalculation(operation, atoi(var1), atoi(var2)));
  }
}

int performCalculation(char* operation,int a, int b){
  if(strcmp(operation, "add") == 0){
    return a + b;
  }
  else if(strcmp(operation, "sub") == 0){
    return a - b;
  }
  else if(strcmp(operation, "mul") == 0){
    return a * b;
  }
  else {
    return a / b;
  }
}

float performCalculation(char* operation,float a, float b){
  if(strcmp(operation, "fadd") == 0){
    return a + b;
  }
  else if(strcmp(operation, "fsub") == 0){
    return a - b;
  }
  else if(strcmp(operation, "fmul") == 0){
    return a * b;
  }
  else {
    return a / b;
  }
}


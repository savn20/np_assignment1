#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <netdb.h>
#include <sys/socket.h>
/* Here we use " as the calcLib.c and calcLib.h files are in the same folder, and are to be BUILT
   to into a library, that will be included in other files.

   This is a C lib, and will be built as such.

*/
#include "calcLib.h"

#define SOCKET_FAILURE -1

/* array of char* that points to char arrays.  */
char *arith[] = {"add", "div", "mul", "fsub", "fadd", "fdiv", "fmul", "fsub"};

/* Used for random number */
time_t myData_seedValue;

int initCalcLib(void)
{
  /* Init the random number generator with a seed, based on the current time--> should be randomish each time called */
  srand((unsigned)time(&myData_seedValue));
  return (0);
}

int initCalcLib_seed(unsigned int seed)
{
  /*
     Init the random number generator with a FIXED seed, will allow us to grab random numbers
     in the same sequence all the time. Good when debugging, bad when running live.

     This is 'messy' for more details see https://en.wikipedia.org/wiki/Pseudorandom_number_generator.

     DO NOT USE rand() for production, wher you NEED good random numbers.
  */

  myData_seedValue = seed;
  srand(seed);
  return (0);
}

char *randomType(void)
{
  int Listitems = sizeof(arith) / (sizeof(char *));
  /* Figure out HOW many entries there are in the list.
     First we get the total size that the array of pointers use, sizeof(arith). Then we divide with
     the size of a pointer (sizeof(char*)), this gives us the number of pointers in the list.
  */
  int itemPos = rand() % Listitems;
  /* As we know the number of items, we can just draw a random number and modulo it with the number
     of items in the list, then we will get a random number between 0 and the number of items in the list

     Using that information, we just return the string found at that position arith[itemPos];
  */
  return (arith[itemPos]);
};

int randomInt(void)
{
  /* Draw a random interger between o and RAND_MAX, then modulo this with 100 to get a random
     number between 0 and 100. */

  return (rand() % 100);
};

double randomFloat(void)
{
  /* The same as for the interber, but for a double, and without the modulo. We cant use
     the module approach as it would generate integers, which we do not want. */
  double x = (double)rand() / (double)(RAND_MAX / 100.0);
  return (x);
};

calcTask *randomTask()
{
  char *operation = randomType();
  char taskString[100];
  double fResult;
  int iResult;

  calcTask *task = malloc(sizeof(calcTask));

  // double
  if (operation[0] == 'f')
  {
    double f1 = randomFloat();
    double f2 = randomFloat();
    if (strcmp(operation, "fadd") == 0)
    {
      fResult = f1 + f2;
    }
    else if (strcmp(operation, "fsub") == 0)
    {
      fResult = f1 - f2;
    }
    else if (strcmp(operation, "fmul") == 0)
    {
      fResult = f1 * f2;
    }
    else if (strcmp(operation, "fdiv") == 0)
    {
      fResult = f1 / f2;
    }

    sprintf(taskString, "%s %8.8g %8.8g\n", operation, f1, f2);
    task->fResult = fResult;
    task->task = strdup(taskString);
  }
  // integer
  else
  {
    int i1 = randomInt();
    int i2 = randomInt();
    if (strcmp(operation, "add") == 0)
    {
      iResult = i1 + i2;
    }
    else if (strcmp(operation, "sub") == 0)
    {
      iResult = i1 - i2;
    }
    else if (strcmp(operation, "mul") == 0)
    {
      iResult = i1 * i2;
    }
    else if (strcmp(operation, "div") == 0)
    {
      iResult = i1 / i2;
    }

    sprintf(taskString, "%s %d %d\n", operation, i1, i2);
    task->iResult = iResult;
    task->task = strdup(taskString);
  }

  return task;
}

calcResult *calculateTask(char *task)
{
  calcResult *result = malloc(sizeof(calcResult));

  char operation[5], val1[20], val2[20];
  sscanf(task, "%s %s %s", operation, val1, val2);

  char taskString[150];

  if (operation[0] == 'f')
  {
    double f1 = atof(val1);
    double f2 = atof(val2);
    double fResult;

    if (strcmp(operation, "fadd") == 0)
    {
      fResult = f1 + f2;
    }
    else if (strcmp(operation, "fsub") == 0)
    {
      fResult = f1 - f2;
    }
    else if (strcmp(operation, "fmul") == 0)
    {
      fResult = f1 * f2;
    }
    else if (strcmp(operation, "fdiv") == 0)
    {
      fResult = f1 / f2;
    }

    sprintf(taskString, "%8.8g\n", fResult);
  }
  else
  {
    int i1 = atoi(val1);
    int i2 = atoi(val2);
    int iResult;

    if (strcmp(operation, "add") == 0)
    {
      iResult = i1 + i2;
    }
    else if (strcmp(operation, "sub") == 0)
    {
      iResult = i1 - i2;
    }
    else if (strcmp(operation, "mul") == 0)
    {
      iResult = i1 * i2;
    }
    else if (strcmp(operation, "div") == 0)
    {
      iResult = i1 / i2;
    }

    sprintf(taskString, "%d\n", iResult);
  }

  result->result = strdup(taskString);
  return result;
}

void verify(int hasError)
{
  if (hasError == SOCKET_FAILURE)
  {
    perror("error: something went wrong dealing with sockets\n");
    exit(-1);
  }
}

void *getSocketAddress(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET)
  {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  if (sa->sa_family == AF_INET6)
  {
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
  }

  perror("Unknown FAMILY!!!!\n");
  return (0);
}

char *getIpAddress(const struct sockaddr *sa, char *s, size_t maxlen)
{
  switch (sa->sa_family)
  {
  case AF_INET:
    inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
              s, maxlen);
    break;

  case AF_INET6:
    inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr),
              s, maxlen);
    break;

  default:
    strncpy(s, "Unknown AF", maxlen);
    return NULL;
  }

  return s;
}
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef __CALC_LIB
#define __CALC_LIB

  /* 

This is the header file for the calcLib. It is a C library.

If compiled with a C++ Compiler, the __cplusplus variable is defined, so when the PREPROCESSOR finds it, it adds the extern "C" { as to indicate to the compiler (next step) that this code is C, not C++. 

The next item __CALC_LIB, is just a define to prevent issues if the header file is included from multiple files while the soultion is building. 

Both ifdef/ifndef have corresponding endif's at the end, and in the correct sequence. 

Implementation in calcLib.c

*/

#define SOCKET_FAILURE -1

  struct calcTask
  {
    int iResult;
    double fResult;
    char *task;
  };

  struct calcResult{
    char* result;
  };

  typedef struct calcTask calcTask;
  typedef struct calcResult calcResult;

  int initCalcLib(void);                   // Init internal variables to the library, if needed.
  int initCalcLib_seed(unsigned int seed); // Init internal variables to the library, use <seed> for specific variable.

  char *randomType(void);   // Return a string to an mathematical operator
  int randomInt(void);      // Return a random integer, between 0 and 100.
  double randomFloat(void); // Return a random float between 0.0 and 100.0
  calcTask *randomTask();
  calcResult* calculateTask(char* message);
  void verify(int hasError);
  void *getSocketAddress(struct sockaddr *sa);
  char *getIpAddress(const struct sockaddr *sa, char *s, size_t maxlen);
#endif

#ifdef __cplusplus
}
#endif

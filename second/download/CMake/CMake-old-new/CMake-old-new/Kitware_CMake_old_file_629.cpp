/*=========================================================================

  Program:   KWSys - Kitware System Library
  Module:    $RCSfile$

  Copyright (c) Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "kwsysPrivate.h"
#include KWSYS_HEADER(Process.h)

/* Work-around CMake dependency scanning limitation.  This must
   duplicate the above list of headers.  */
#if 0
# include "Process.h.in"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
# include <windows.h>
#else
# include <unistd.h>
#endif

int runChild(const char* cmd[], int state, int exception, int value,
             int share, int output, int delay, double timeout, int poll);

int test1(int argc, const char* argv[])
{
  (void)argc; (void)argv;
  fprintf(stdout, "Output on stdout from test returning 0.\n");
  fprintf(stderr, "Output on stderr from test returning 0.\n");
  return 0;
}

int test2(int argc, const char* argv[])
{
  (void)argc; (void)argv;
  fprintf(stdout, "Output on stdout from test returning 123.\n");
  fprintf(stderr, "Output on stderr from test returning 123.\n");
  return 123;
}

int test3(int argc, const char* argv[])
{
  (void)argc; (void)argv;
  fprintf(stdout, "Output before sleep on stdout from timeout test.\n");
  fprintf(stderr, "Output before sleep on stderr from timeout test.\n");
  fflush(stdout);
  fflush(stderr);
#if defined(_WIN32)
  Sleep(15000);
#else
  sleep(15);
#endif
  fprintf(stdout, "Output after sleep on stdout from timeout test.\n");
  fprintf(stderr, "Output after sleep on stderr from timeout test.\n");
  return 0;
}

int test4(int argc, const char* argv[])
{
#if defined(_WIN32)
  /* Avoid error diagnostic popups since we are crashing on purpose.  */
  SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
#endif
  (void)argc; (void)argv;
  fprintf(stdout, "Output before crash on stdout from crash test.\n");
  fprintf(stderr, "Output before crash on stderr from crash test.\n");  
  fflush(stdout);
  fflush(stderr);
  *(int*)0 = 0;
  fprintf(stdout, "Output after crash on stdout from crash test.\n");
  fprintf(stderr, "Output after crash on stderr from crash test.\n");
  return 0;
}

int test5(int argc, const char* argv[])
{
  int r;
  const char* cmd[4];
  (void)argc;
  cmd[0] = argv[0];
  cmd[1] = "run";
  cmd[2] = "4";
  cmd[3] = 0;
  fprintf(stdout, "Output on stdout before recursive test.\n");
  fprintf(stderr, "Output on stderr before recursive test.\n");
  fflush(stdout);
  fflush(stderr);
  r = runChild(cmd, kwsysProcess_State_Exception,
               kwsysProcess_Exception_Fault, 1, 1, 1, 0, 15, 0);
  fprintf(stdout, "Output on stdout after recursive test.\n");
  fprintf(stderr, "Output on stderr after recursive test.\n");
  fflush(stdout);
  fflush(stderr);
  return r;
}

#define TEST6_SIZE (4096*2)
void test6(int argc, const char* argv[])
{
  int i;
  char runaway[TEST6_SIZE+1];
  (void)argc; (void)argv;
  for(i=0;i < TEST6_SIZE;++i)
    {
    runaway[i] = '.';
    }
  runaway[TEST6_SIZE] = '\n';

  /* Generate huge amounts of output to test killing.  */
  for(;;)
    {
    fwrite(runaway, 1, TEST6_SIZE+1, stdout);
    fflush(stdout);
    }
}

/* Define MINPOLL to be one more than the number of times output is
   written.  Define MAXPOLL to be the largest number of times a loop
   delaying 1/10th of a second should ever have to poll.  */
#define MINPOLL 5
#define MAXPOLL 20
int test7(int argc, const char* argv[])
{
  (void)argc; (void)argv;
  fprintf(stdout, "Output on stdout before sleep.\n");
  fprintf(stderr, "Output on stderr before sleep.\n");
  fflush(stdout);
  fflush(stderr);
  /* Sleep for 1 second.  */
#if defined(_WIN32)
  Sleep(1000);
#else
  sleep(1);
#endif
  fprintf(stdout, "Output on stdout after sleep.\n");
  fprintf(stderr, "Output on stderr after sleep.\n");
  fflush(stdout);
  fflush(stderr);
  return 0;
}

int runChild(const char* cmd[], int state, int exception, int value,
             int share, int output, int delay, double timeout,
             int poll)
{
  int result = 0;
  char* data = 0;
  int length = 0;
  double userTimeout = 0;
  double* pUserTimeout = 0;
  kwsysProcess* kp = kwsysProcess_New();
  if(!kp)
    {
    fprintf(stderr, "kwsysProcess_New returned NULL!\n");
    return 1;
    }
  
  kwsysProcess_SetCommand(kp, cmd);
  if(timeout >= 0)
    {
    kwsysProcess_SetTimeout(kp, timeout);
    }
  if(share)
    {
    kwsysProcess_SetPipeShared(kp, kwsysProcess_Pipe_STDOUT, 1);
    kwsysProcess_SetPipeShared(kp, kwsysProcess_Pipe_STDERR, 1);
    }
  kwsysProcess_Execute(kp);

  if(poll)
    {
    pUserTimeout = &userTimeout;
    }

  if(!share)
    {
    int p;
    while((p = kwsysProcess_WaitForData(kp, &data, &length, pUserTimeout)))
      {
      if(output)
        {
        if(poll && p == kwsysProcess_Pipe_Timeout)
          {
          fprintf(stdout, "WaitForData timeout reached.\n");
          fflush(stdout);

          /* Count the number of times we polled without getting data.
             If it is excessive then kill the child and fail.  */
          if(++poll >= MAXPOLL)
            {
            fprintf(stdout, "Poll count reached limit %d.\n",
                    MAXPOLL);
            kwsysProcess_Kill(kp);
            }
          }
        else
          {
          fwrite(data, 1, length, stdout);
          fflush(stdout);
          }
        }
      if(poll)
        {
        /* Delay to avoid busy loop during polling.  */
#if defined(_WIN32)
        Sleep(100);
#else
        usleep(100000);
#endif
        }
      if(delay)
        {
        /* Purposely sleeping only on Win32 to let pipe fill up.  */
#if defined(_WIN32)
        Sleep(100);
#endif
        }
      }
    }
  
  kwsysProcess_WaitForExit(kp, 0);

  switch (kwsysProcess_GetState(kp))
    {
    case kwsysProcess_State_Starting:
      printf("No process has been executed.\n"); break;
    case kwsysProcess_State_Executing:
      printf("The process is still executing.\n"); break;
    case kwsysProcess_State_Expired:
      printf("Child was killed when timeout expired.\n"); break;
    case kwsysProcess_State_Exited:
      printf("Child exited with value = %d\n",
             kwsysProcess_GetExitValue(kp));
      result = ((exception != kwsysProcess_GetExitException(kp)) ||
                (value != kwsysProcess_GetExitValue(kp))); break;
    case kwsysProcess_State_Killed:
      printf("Child was killed by parent.\n"); break;
    case kwsysProcess_State_Exception:
      printf("Child terminated abnormally: %s\n",
             kwsysProcess_GetExceptionString(kp));
      result = ((exception != kwsysProcess_GetExitException(kp)) ||
                (value != kwsysProcess_GetExitValue(kp))); break;
    case kwsysProcess_State_Error:
      printf("Error in administrating child process: [%s]\n",
             kwsysProcess_GetErrorString(kp)); break;
    };
  
  if(result)
    {
    if(exception != kwsysProcess_GetExitException(kp))
      {
      fprintf(stderr, "Mismatch in exit exception.  "
              "Should have been %d, was %d.\n",
              exception, kwsysProcess_GetExitException(kp));
      }
    if(value != kwsysProcess_GetExitValue(kp))
      {
      fprintf(stderr, "Mismatch in exit value.  "
              "Should have been %d, was %d.\n",
              value, kwsysProcess_GetExitValue(kp));
      }
    }
  
  if(kwsysProcess_GetState(kp) != state)
    {
    fprintf(stderr, "Mismatch in state.  "
            "Should have been %d, was %d.\n",
            state, kwsysProcess_GetState(kp));
    result = 1;
    }

  /* We should have polled more times than there were data if polling
     was enabled.  */
  if(poll && poll < MINPOLL)
    {
    fprintf(stderr, "Poll count is %d, which is less than %d.\n",
            poll, MINPOLL);
    result = 1;
    }
  
  kwsysProcess_Delete(kp);
  return result;
}

int main(int argc, const char* argv[])
{
  int n = 0;
#if 0
    {
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    DuplicateHandle(GetCurrentProcess(), out,
                    GetCurrentProcess(), &out, 0, FALSE,
                    DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);
    SetStdHandle(STD_OUTPUT_HANDLE, out);
    }
    {
    HANDLE out = GetStdHandle(STD_ERROR_HANDLE);
    DuplicateHandle(GetCurrentProcess(), out,
                    GetCurrentProcess(), &out, 0, FALSE,
                    DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);
    SetStdHandle(STD_ERROR_HANDLE, out);
    }
#endif
  if(argc == 2)
    {
    n = atoi(argv[1]);
    }
  else if(argc == 3)
    {
    n = atoi(argv[2]);
    }
  /* Check arguments.  */
  if(n < 1 || n > 7 || (argc == 3 && strcmp(argv[1], "run") != 0))
    {
    fprintf(stdout, "Usage: %s <test number>\n", argv[0]);
    return 1;
    }
  if(argc == 3)
    {
    switch (n)
      {
      case 1: return test1(argc, argv);
      case 2: return test2(argc, argv);
      case 3: return test3(argc, argv);
      case 4: return test4(argc, argv);
      case 5: return test5(argc, argv);
      case 6: test6(argc, argv); return 0;
      case 7: return test7(argc, argv);
      }
    fprintf(stderr, "Invalid test number %d.\n", n);
    return 1;
    }
  
  if(n >= 0 && n <= 7)
    {
    int states[7] =
    {
      kwsysProcess_State_Exited,
      kwsysProcess_State_Exited,
      kwsysProcess_State_Expired,
      kwsysProcess_State_Exception,
      kwsysProcess_State_Exited,
      kwsysProcess_State_Expired,
      kwsysProcess_State_Exited
    };
    int exceptions[7] =
    {
      kwsysProcess_Exception_None,
      kwsysProcess_Exception_None,
      kwsysProcess_Exception_None,
      kwsysProcess_Exception_Fault,
      kwsysProcess_Exception_None,
      kwsysProcess_Exception_None,
      kwsysProcess_Exception_None
    };
    int values[7] = {0, 123, 1, 1, 0, 0, 0};
    int outputs[7] = {1, 1, 1, 1, 1, 0, 1};
    int delays[7] = {0, 0, 0, 0, 0, 1, 0};
    double timeouts[7] = {10, 10, 10, 10, 30, 10, -1};
    int polls[7] = {0, 0, 0, 0, 0, 0, 1};
    int r;
    const char* cmd[4];
#ifdef _WIN32
    char* argv0 = 0;
    if(n == 0 && (argv0 = strdup(argv[0])))
      {
      /* Try converting to forward slashes to see if it works.  */
      char* c;
      for(c=argv0; *c; ++c)
        {
        if(*c == '\\')
          {
          *c = '/';
          }
        }
      cmd[0] = argv0;
      }
    else
      {
      cmd[0] = argv[0];
      }
#else
    cmd[0] = argv[0];
#endif
    cmd[1] = "run";
    cmd[2] = argv[1];
    cmd[3] = 0;
    fprintf(stdout, "Output on stdout before test %d.\n", n);
    fprintf(stderr, "Output on stderr before test %d.\n", n);
    fflush(stdout);
    fflush(stderr);
    r = runChild(cmd, states[n-1], exceptions[n-1], values[n-1], 0,
                 outputs[n-1], delays[n-1], timeouts[n-1],
                 polls[n-1]);
    fprintf(stdout, "Output on stdout after test %d.\n", n);
    fprintf(stderr, "Output on stderr after test %d.\n", n);
    fflush(stdout);
    fflush(stderr);
#if _WIN32
    if(argv0) { free(argv0); }
#endif
    return r;
    }
  else
    {
    fprintf(stderr, "Test number out of range\n");
    return 1;
    }  
}

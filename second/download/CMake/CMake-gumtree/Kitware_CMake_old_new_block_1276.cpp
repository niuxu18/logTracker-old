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
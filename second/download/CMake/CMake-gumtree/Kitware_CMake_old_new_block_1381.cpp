{
    /* This is Win9x.  We need the console forwarding executable to
       work-around a Windows 9x bug.  */
    char fwdName[_MAX_FNAME+1] = "";
    char tempDir[_MAX_PATH+1] = "";

    /* We will try putting the executable in the system temp
       directory.  Note that the returned path already has a trailing
       slash.  */
    DWORD length = GetTempPath(_MAX_PATH+1, tempDir);

    /* Construct the executable name from the process id and kwsysProcess
       instance.  This should be unique.  */
    sprintf(fwdName, "cmw9xfwd_%u_%p.exe", GetCurrentProcessId(), cp);

    /* If we have a temp directory, use it.  */
    if(length > 0 && length <= _MAX_PATH)
      {
      /* Allocate a buffer to hold the forwarding executable path.  */
      size_t tdlen = strlen(tempDir);
      win9x = (char*)malloc(tdlen + strlen(fwdName) + 2);
      if(!win9x)
        {
        kwsysProcess_Delete(cp);
        return 0;
        }

      /* Construct the full path to the forwarding executable.  */
      sprintf(win9x, "%s%s", tempDir, fwdName);
      }

    /* If we found a place to put the forwarding executable, try to
       write it. */
    if(win9x)
      {
      if(!kwsysEncodedWriteArrayProcessFwd9x(win9x))
        {
        /* Failed to create forwarding executable.  Give up.  */
        free(win9x);
        kwsysProcess_Delete(cp);
        return 0;
        }
      }
    else
      {
      /* Failed to find a place to put forwarding executable.  */
      kwsysProcess_Delete(cp);
      return 0;
      }
    }
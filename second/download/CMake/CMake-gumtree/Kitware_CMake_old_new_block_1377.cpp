{
    int states[4] =
      {
        kwsysProcess_State_Exited,
        kwsysProcess_State_Exited,
        kwsysProcess_State_Expired,
        kwsysProcess_State_Exception
      };
    int exceptions[4] = {kwsysProcess_Exception_None, kwsysProcess_Exception_None,
                         kwsysProcess_Exception_None, kwsysProcess_Exception_Fault};
    int values[4] = {0, 123, 1, 1};
    const char* cmd[4];
    cmd[0] = argv[0];
    cmd[1] = "run";
    cmd[2] = argv[1];
    cmd[3] = 0;
    return runChild(cmd, states[n-1], exceptions[n-1], values[n-1]);
    }
ap_log_perror(APLOG_MARK, APLOG_WARNING, 0, p,
                      "pid file %s overwritten -- Unclean "
                      "shutdown of previous Apache run?",
                      fname);
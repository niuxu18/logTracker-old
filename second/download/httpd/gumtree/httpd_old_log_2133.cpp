ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
                              "Request exceeded the limit of %d subrequest "
                              "nesting levels due to probable confguration "
                              "error. Use 'LimitInternalRecursion' to increase "
                              "the limit if necessary. Use 'LogLevel debug' to "
                              "get a backtrace.", slimit);
ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server,
               "ajp_parse_reuse: wrong type 0x%02x expecting 0x%02x",
               result, CMD_AJP13_END_RESPONSE);
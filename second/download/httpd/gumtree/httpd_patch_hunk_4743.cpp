     {"match_handler", cfg_lua_map_handler},
     {"directory", cfg_directory},
     /* {"root", cfg_root}, */
     {NULL, NULL}
 };
 
-
-static int cmd_foo(lua_State *L)
-{
-    cmd_parms *cmd = check_cmd_parms(L, 1);
-    ap_log_error(APLOG_MARK, APLOG_ERR, 0, cmd->server, APLOGNO(01479) "FOO!");
-    return 0;
-}
-
 /* helper function for the logging functions below */
 static int cmd_log_at(lua_State *L, int level)
 {
     const char *msg;
     cmd_parms *cmd = check_cmd_parms(L, 1);
     lua_Debug dbg;

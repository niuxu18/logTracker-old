"  Other interesting options for it -t include:\n"
"\n"
"   - XDISPLOC=<X display> Sets the X display location.\n"
"\n"
"   - NEW_ENV=<var,val> Sets an environment variable.\n"
"\n"
"  NOTE: the telnet protocol does not specify any way to login with a specified\n"
, stdout);
 fputs(
"  user and password so curl can't do that automatically. To do that, you need\n"
"  to track when the login prompt is received and send the username and\n"
"  password accordingly.\n"
"\n"

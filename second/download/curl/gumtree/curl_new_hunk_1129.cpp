"       --tlsv1.2\n"
"              (SSL) Forces curl to use TLS version 1.2 when negotiating with a\n"
"              remote TLS server.  (Added in 7.34.0)\n"
"\n"
"       --tr-encoding\n"
"              (HTTP) Request a compressed Transfer-Encoding response using one\n"
"              of  the  algorithms curl supports, and uncompress the data while\n"
"              receiving it.\n"
"\n"
"              (Added in 7.21.6)\n"
"\n"
"       --trace <file>\n"
"              Enables a full trace dump of all  incoming  and  outgoing  data,\n"
, stdout);
 fputs(
"              including descriptive information, to the given output file. Use\n"
"              \"-\" as filename to have the output sent to stdout.  Use  \"%\"  as\n"
"              filename to have the output sent to stderr.\n"
"\n"
"              This option overrides previous uses of -v, --verbose or --trace-\n"
"              ascii.\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
"       --trace-ascii <file>\n"
, stdout);
 fputs(
"              Enables a full trace dump of all  incoming  and  outgoing  data,\n"
"              including descriptive information, to the given output file. Use\n"
"              \"-\" as filename to have the output sent to stdout.\n"
"\n"
"              This is very similar to --trace, but leaves out the hex part and\n"
"              only  shows  the ASCII part of the dump. It makes smaller output\n"
"              that might be easier to read for untrained humans.\n"
"\n"
, stdout);
 fputs(
"              This option overrides previous uses of -v, --verbose or --trace.\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
"       --trace-time\n"
"              Prepends a time stamp to each trace or verbose  line  that  curl\n"
"              displays.  (Added in 7.14.0)\n"
"\n"
"       --unix-socket <path>\n"
"              (HTTP) Connect through this Unix domain socket, instead of using\n"
"              the network. (Added in 7.40.0)\n"
"\n"
"       -u, --user <user:password>\n"
, stdout);
 fputs(
"              Specify the user name and password to use for server authentica-\n"
"              tion. Overrides -n, --netrc and --netrc-optional.\n"
"\n"
"              If  you  simply  specify  the  user name, curl will prompt for a\n"
"              password.\n"
"\n"
"              The user name and passwords are split up  on  the  first  colon,\n"
"              which  makes  it impossible to use a colon in the user name with\n"
"              this option. The password can, still.\n"
"\n"
, stdout);
 fputs(
"              When using Kerberos V5 with a Windows based  server  you  should\n"
"              include  the  Windows domain name in the user name, in order for\n"
"              the server to successfully obtain  a  Kerberos  Ticket.  If  you\n"
"              don't then the initial authentication handshake may fail.\n"
"\n"
"              When  using  NTLM,  the user name can be specified simply as the\n"
"              user name, without the domain, if there is a single  domain  and\n"
, stdout);
 fputs(
"              forest in your setup for example.\n"
"\n"
"              To  specify  the domain name use either Down-Level Logon Name or\n"
"              UPN (User Principal Name) formats. For example, EXAMPLE\\user and\n"
"              user@example.com respectively.\n"
"\n"
"              If  you  use a Windows SSPI-enabled curl binary and perform Ker-\n"
"              beros V5, Negotiate, NTLM or Digest authentication then you  can\n"

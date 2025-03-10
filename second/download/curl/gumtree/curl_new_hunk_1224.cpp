"\n"
"       --proxy-ciphers <list>\n"
"              Same as --ciphers but used in HTTPS proxy context.\n"
"\n"
"              Added in 7.52.0.\n"
"\n"
"       --proxy-crlfile <file>\n"
"              Same as --crlfile but used in HTTPS proxy context.\n"
"\n"
"              Added in 7.52.0.\n"
"\n"
, stdout);
 fputs(
"       --proxy-digest\n"
"              Tells  curl to use HTTP Digest authentication when communicating\n"
"              with the given proxy. Use --digest for enabling HTTP Digest with\n"
"              a remote host.\n"
"\n"
"              See also -x, --proxy and --proxy-anyauth and --proxy-basic.\n"
"\n"
"       --proxy-header <header>\n"
"              (HTTP)  Extra header to include in the request when sending HTTP\n"
"              to a proxy. You may specify any number of extra headers. This is\n"
, stdout);
 fputs(
"              the  equivalent option to -H, --header but is for proxy communi-\n"
"              cation only like in CONNECT requests when you  want  a  separate\n"
"              header  sent  to  the proxy to what is sent to the actual remote\n"
"              host.\n"
"\n"
"              curl will make sure that each header  you  add/replace  is  sent\n"
"              with the proper end-of-line marker, you should thus not add that\n"
"              as a part of the header content: do not add newlines or carriage\n"
, stdout);
 fputs(
"              returns, they will only mess things up for you.\n"
"\n"
"              Headers  specified  with  this  option  will  not be included in\n"
"              requests that curl knows will not be sent to a proxy.\n"
"\n"
"              This option can be used  multiple  times  to  add/replace/remove\n"
"              multiple headers.\n"
"\n"
"              Added in 7.37.0.\n"
"       --proxy-insecure\n"
"              Same as -k, --insecure but used in HTTPS proxy context.\n"
"\n"
"              Added in 7.52.0.\n"
"\n"
, stdout);
 fputs(
"       --proxy-key-type <type>\n"
"              Same as --key-type but used in HTTPS proxy context.\n"
"\n"
"              Added in 7.52.0.\n"
"\n"
"       --proxy-key <key>\n"
"              Same as --key but used in HTTPS proxy context.\n"
"\n"
"       --proxy-negotiate\n"
"              Tells  curl  to  use HTTP Negotiate (SPNEGO) authentication when\n"
"              communicating with the given proxy. Use --negotiate for enabling\n"
"              HTTP Negotiate (SPNEGO) with a remote host.\n"
"\n"
, stdout);
 fputs(
"              See also --proxy-anyauth and --proxy-basic. Added in 7.17.1.\n"
"\n"
"       --proxy-ntlm\n"
"              Tells  curl  to  use HTTP NTLM authentication when communicating\n"
"              with the given proxy. Use --ntlm for enabling NTLM with a remote\n"
"              host.\n"
"\n"
"              See also --proxy-negotiate and --proxy-anyauth.\n"
"\n"
"       --proxy-pass <phrase>\n"
"              Same as --pass but used in HTTPS proxy context.\n"
"\n"
"              Added in 7.52.0.\n"
"\n"
"       --proxy-service-name <name>\n"
, stdout);
 fputs(
"              This  option  allows  you  to  change the service name for proxy\n"
"              negotiation.\n"
"\n"
"              Added in 7.43.0.\n"
"\n"
"       --proxy-ssl-allow-beast\n"
"              Same as --ssl-allow-beast but used in HTTPS proxy context.\n"
"\n"
"              Added in 7.52.0.\n"
"\n"
"       --proxy-tlsauthtype <type>\n"
"              Same as --tlsauthtype but used in HTTPS proxy context.\n"
"\n"
"              Added in 7.52.0.\n"
"\n"
"       --proxy-tlspassword <string>\n"
, stdout);
 fputs(
"              Same as --tlspassword but used in HTTPS proxy context.\n"
"\n"
"              Added in 7.52.0.\n"
"\n"
"       --proxy-tlsuser <name>\n"
"              Same as --tlsuser but used in HTTPS proxy context.\n"

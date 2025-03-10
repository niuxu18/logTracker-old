 fputs(
"                     This curl supports transfers of large files, files larger\n"
"                     than 2GB.\n"
"\n"
"              IDN    This curl supports IDN - international domain names.\n"
"\n"
"              SSPI   SSPI is supported. If you use NTLM and set a  blank  user\n"
"                     name,  curl  will authenticate with your current user and\n"
"                     password.\n"
"\n"
"              TLS-SRP\n"
"                     SRP (Secure Remote Password) authentication is  supported\n"
, stdout);
 fputs(
"                     for TLS.\n"
"\n"
"FILES\n"
"       ~/.curlrc\n"
"              Default config file, see -K, --config for details.\n"
"\n"
"ENVIRONMENT\n"
"       The environment variables can be specified in lower case or upper case.\n"
"       The lower case version has precedence. http_proxy is an exception as it\n"
"       is only available in lower case.\n"
"\n"
"       Using  an  environment variable to set the proxy has the same effect as\n"
"       using the --proxy option.\n"
"\n"
"       http_proxy [protocol://]<host>[:port]\n"
, stdout);
 fputs(
"              Sets the proxy server to use for HTTP.\n"
"       HTTPS_PROXY [protocol://]<host>[:port]\n"
"              Sets the proxy server to use for HTTPS.\n"
"\n"
"       [url-protocol]_PROXY [protocol://]<host>[:port]\n"
"              Sets the proxy server to use for [url-protocol], where the  pro-\n"
"              tocol  is  a  protocol  that curl supports and as specified in a\n"
"              URL. FTP, FTPS, POP3, IMAP, SMTP, LDAP etc.\n"
"\n"
"       ALL_PROXY [protocol://]<host>[:port]\n"
, stdout);
 fputs(
"              Sets the proxy server to use if no  protocol-specific  proxy  is\n"
"              set.\n"
"\n"
"       NO_PROXY <comma-separated list of hosts>\n"
"              list  of  host names that shouldn't go through any proxy. If set\n"
"              to a asterisk '*' only, it matches all hosts.\n"
"\n"
"PROXY PROTOCOL PREFIXES\n"
"       Since curl version 7.21.7, the proxy string may  be  specified  with  a\n"
"       protocol:// prefix to specify alternative proxy protocols.\n"
"\n"
, stdout);
 fputs(
"       If  no  protocol  is  specified  in  the  proxy string or if the string\n"
"       doesn't match a supported one, the proxy will  be  treated  as  a  HTTP\n"
"       proxy.\n"
"\n"
"       The supported proxy protocol prefixes are as follows:\n"
"\n"
"       socks4://\n"

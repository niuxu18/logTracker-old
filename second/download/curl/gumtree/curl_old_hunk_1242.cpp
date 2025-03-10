, stdout);
 fputs(
"              with the proper end-of-line marker, you should thus not add that\n"
"              as a part of the header content: do not add newlines or carriage\n"
"              returns, they will only mess things up for you.\n"
"\n"
"              See also the -A, --user-agent and -e, --referer options.\n"
"\n"
"              Starting in 7.37.0, you need --proxy-header to send custom head-\n"
"              ers intended for a proxy.\n"
"\n"
"              Example:\n"
"\n"
"               curl -H \"X-First-Name: Joe\" http://example.com/\n"
, stdout);
 fputs(
"\n"
"              WARNING:  headers  set  with  this  option  will  be  set in all\n"
"              requests - even after redirects are  followed,  like  when  told\n"
"              with  -L,  --location. This can lead to the header being sent to\n"
"              other hosts than the original host, so sensitive headers  should\n"
"              be used with caution combined with following redirects.\n"
"\n"
"              This  option  can  be  used multiple times to add/replace/remove\n"
"              multiple headers.\n"
"\n"
, stdout);
 fputs(
"       -h, --help\n"
"              Usage help. This lists all current command line options  with  a\n"
"              short description.\n"
"       --hostpubmd5 <md5>\n"
"              (SFTP  SCP)  Pass a string containing 32 hexadecimal digits. The\n"
"              string should be the 128 bit MD5 checksum of the  remote  host's\n"
"              public key, curl will refuse the connection with the host unless\n"
"              the md5sums match.\n"
"\n"
"              Added in 7.17.1.\n"
"\n"
"       -0, --http1.0\n"
, stdout);
 fputs(
"              (HTTP) Tells curl to use HTTP version 1.0 instead of  using  its\n"
"              internally preferred HTTP version.\n"
"\n"
"              This option overrides --http1.1 and --http2.\n"
"\n"
"       --http1.1\n"
"              (HTTP) Tells curl to use HTTP version 1.1.\n"
"\n"
"              This  option  overrides  -0,  --http1.0  and  --http2.  Added in\n"
"              7.33.0.\n"
"\n"
"       --http2-prior-knowledge\n"
"              (HTTP) Tells curl to  issue  its  non-TLS  HTTP  requests  using\n"
, stdout);
 fputs(
"              HTTP/2  without  HTTP/1.1  Upgrade.  It requires prior knowledge\n"
"              that the server supports HTTP/2 straight  away.  HTTPS  requests\n"
"              will  still  do HTTP/2 the standard way with negotiated protocol\n"
"              version in the TLS handshake.\n"
"\n"
"              --http2-prior-knowledge requires that the underlying libcurl was\n"
"              built to support HTTP/2. This option overrides --http1.1 and -0,\n"
"              --http1.0 and --http2. Added in 7.49.0.\n"
"\n"
, stdout);
 fputs(
"       --http2\n"
"              (HTTP) Tells curl to use HTTP version 2.\n"
"\n"
"              See also --no-alpn. --http2 requires that the underlying libcurl\n"
"              was built to support HTTP/2. This option overrides --http1.1 and\n"
"              -0, --http1.0 and --http2-prior-knowledge. Added in 7.33.0.\n"
"\n"
"       --ignore-content-length\n"
"              (FTP HTTP) For HTTP, Ignore the Content-Length header.  This  is\n"

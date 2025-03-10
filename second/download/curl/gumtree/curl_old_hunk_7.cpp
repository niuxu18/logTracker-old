"  or\n"
"\n"
"	curl --interface 192.168.1.10 http://www.netscape.com/\n"
"\n"
"HTTPS\n"
"\n"
"  Secure HTTP requires SSL libraries to be installed and used when curl is\n"
"  built. If that is done, curl is capable of retrieving and posting documents\n"
"  using the HTTPS procotol.\n"
"\n"
"  Example:\n"
"\n"
"        curl https://www.secure-site.com\n"
"\n"
);
 puts(
"  Curl is also capable of using your personal certificates to get/post files\n"
"  from sites that require valid certificates. The only drawback is that the\n"
"  certificate needs to be in PEM-format. PEM is a standard and open format to\n"
"  store certificates with, but it is not used by the most commonly used\n"
"  browsers (Netscape and MSEI both use the so called PKCS#12 format). If you\n"
"  want curl to use the certificates you use with your (favourite) browser, you\n"
);
 puts(
"  may need to download/compile a converter that can convert your browser's\n"
"  formatted certificates to PEM formatted ones. This kind of converter is\n"
"  included in recent versions of OpenSSL, and for older versions Dr Stephen\n"
"  N. Henson has written a patch for SSLeay that adds this functionality. You\n"
"  can get his patch (that requires an SSLeay installation) from his site at:\n"
"  http://www.drh-consultancy.demon.co.uk/\n"
"\n"
);
 puts(
"  Example on how to automatically retrieve a document using a certificate with\n"
"  a personal password:\n"
"\n"
"        curl -E /path/to/cert.pem:password https://secure.site.com/\n"
"\n"
"  If you neglect to specify the password on the command line, you will be\n"
"  prompted for the correct password before any data can be received.\n"
"\n"
"  Many older SSL-servers have problems with SSLv3 or TLS, that newer versions\n"
"  of OpenSSL etc is using, therefore it is sometimes useful to specify what\n"
);
 puts(
"  SSL-version curl should use. Use -3 or -2 to specify that exact SSL version\n"
"  to use:\n"
"\n"
"        curl -2 https://secure.site.com/\n"
"\n"
"  Otherwise, curl will first attempt to use v3 and then v2.\n"
"\n"
"  To use OpenSSL to convert your favourite browser's certificate into a PEM\n"
"  formatted one that curl can use, do something like this (assuming netscape,\n"
"  but IE is likely to work similarly):\n"
"\n"
"    You start with hitting the 'security' menu button in netscape. \n"
"\n"
);
 puts(
"    Select 'certificates->yours' and then pick a certificate in the list \n"
"\n"
"    Press the 'export' button \n"
"\n"
"    enter your PIN code for the certs \n"
"\n"
"    select a proper place to save it \n"
"\n"
"    Run the 'openssl' application to convert the certificate. If you cd to the\n"
"    openssl installation, you can do it like:\n"
"\n"
"     # ./apps/openssl pkcs12 -in [file you saved] -clcerts -out [PEMfile]\n"
"\n"
"\n"
"RESUMING FILE TRANSFERS\n"
"\n"
" To continue a file transfer where it was previously aborted, curl supports\n"
);
 puts(
" resume on http(s) downloads as well as ftp uploads and downloads.\n"
"\n"
" Continue downloading a document:\n"
"\n"
"        curl -C - -o file ftp://ftp.server.com/path/file\n"
"\n"
" Continue uploading a document(*1):\n"
"\n"
"        curl -C - -T file ftp://ftp.server.com/path/file\n"
"\n"
" Continue downloading a document from a web server(*2):\n"
"\n"
"        curl -C - -o file http://www.server.com/\n"
"\n"
" (*1) = This requires that the ftp server supports the non-standard command\n"
"        SIZE. If it doesn't, curl will say so.\n"
"\n"
);
 puts(
" (*2) = This requires that the web server supports at least HTTP/1.1. If it\n"
"        doesn't, curl will say so.\n"
"\n"
"TIME CONDITIONS\n"
"\n"
" HTTP allows a client to specify a time condition for the document it\n"
" requests. It is If-Modified-Since or If-Unmodified-Since. Curl allow you to\n"
" specify them with the -z/--time-cond flag.\n"
"\n"
" For example, you can easily make a download that only gets performed if the\n"
" remote file is newer than a local copy. It would be made like:\n"
"\n"
);
 puts(
"        curl -z local.html http://remote.server.com/remote.html\n"
"\n"
" Or you can download a file only if the local file is newer than the remote\n"
" one. Do this by prepending the date string with a '-', as in:\n"
"\n"
"        curl -z -local.html http://remote.server.com/remote.html\n"
"\n"
" You can specify a \"free text\" date as condition. Tell curl to only download\n"
" the file if it was updated since yesterday:\n"
"\n"
"        curl -z yesterday http://remote.server.com/remote.html\n"
"\n"
);
 puts(
" Curl will then accept a wide range of date formats. You always make the date\n"
" check the other way around by prepending it with a dash '-'.\n"
"\n"
"DICT\n"
"\n"
"  For fun try\n"
"\n"
"        curl dict://dict.org/m:curl\n"
"        curl dict://dict.org/d:heisenbug:jargon\n"
"        curl dict://dict.org/d:daniel:web1913\n"
"\n"
"  Aliases for 'm' are 'match' and 'find', and aliases for 'd' are 'define'\n"
"  and 'lookup'. For example,\n"
"\n"
"        curl dict://dict.org/find:curl\n"
"\n"
);
 puts(
"  Commands that break the URL description of the RFC (but not the DICT\n"
"  protocol) are\n"
"\n"
"        curl dict://dict.org/show:db\n"
"        curl dict://dict.org/show:strat\n"
"\n"
"  Authentication is still missing (but this is not required by the RFC)\n"
"\n"
"LDAP\n"
"\n"
"  If you have installed the OpenLDAP library, curl can take advantage of it\n"
"  and offer ldap:// support.\n"
"\n"
"  LDAP is a complex thing and writing an LDAP query is not an easy task. I do\n"
);
 puts(
"  advice you to dig up the syntax description for that elsewhere. Two places\n"
"  that might suit you are:\n"
"\n"
"  Netscape's \"Netscape Directory SDK 3.0 for C Programmer's Guide Chapter 10:\n"
"  Working with LDAP URLs\":\n"
"  http://developer.netscape.com/docs/manuals/dirsdk/csdk30/url.htm\n"
"\n"
"  RFC 2255, \"The LDAP URL Format\" http://www.rfc-editor.org/rfc/rfc2255.txt\n"
"\n"
"  To show you an example, this is now I can get all people from my local LDAP\n"
"  server that has a certain sub-domain in their email address:\n"
"\n"
);
 puts(
"        curl -B \"ldap://ldap.frontec.se/o=frontec??sub?mail=*sth.frontec.se\"\n"
"\n"
"  If I want the same info in HTML format, I can get it by not using the -B\n"
"  (enforce ASCII) flag.\n"
"\n"
"ENVIRONMENT VARIABLES\n"
"\n"
"  Curl reads and understands the following environment variables:\n"
"\n"
"        http_proxy, HTTPS_PROXY, FTP_PROXY, GOPHER_PROXY\n"
"\n"
"  They should be set for protocol-specific proxies. General proxy should be\n"
"  set with\n"
"        \n"
"        ALL_PROXY\n"
"\n"
);
 puts(
"  A comma-separated list of host names that shouldn't go through any proxy is\n"
"  set in (only an asterisk, '*' matches all hosts)\n"
"\n"
"        NO_PROXY\n"
"\n"
"  If a tail substring of the domain-path for a host matches one of these\n"
"  strings, transactions with that node will not be proxied.\n"
"\n"
"\n"
"  The usage of the -x/--proxy flag overrides the environment variables.\n"
"\n"
"NETRC\n"
"\n"
"  Unix introduced the .netrc concept a long time ago. It is a way for a user\n"
);
 puts(
"  to specify name and password for commonly visited ftp sites in a file so\n"
"  that you don't have to type them in each time you visit those sites. You\n"
"  realize this is a big security risk if someone else gets hold of your\n"
"  passwords, so therefor most unix programs won't read this file unless it is\n"
"  only readable by yourself (curl doesn't care though).\n"
"\n"
"  Curl supports .netrc files if told so (using the -n/--netrc and\n"
"  --netrc-optional options). This is not restricted to only ftp,\n"
);
 puts(
"  but curl can use it for all protocols where authentication is used.\n"
"\n"
"  A very simple .netrc file could look something like:\n"
"\n"
"        machine curl.haxx.se login iamdaniel password mysecret\n"
"\n"
"CUSTOM OUTPUT\n"
"\n"
"  To better allow script programmers to get to know about the progress of\n"
"  curl, the -w/--write-out option was introduced. Using this, you can specify\n"
"  what information from the previous transfer you want to extract.\n"
"\n"
);
 puts(
"  To display the amount of bytes downloaded together with some text and an\n"
"  ending newline:\n"
"\n"
"        curl -w 'We downloaded %{size_download} bytes\\n' www.download.com\n"
"\n"
"KERBEROS4 FTP TRANSFER\n"
"\n"
"  Curl supports kerberos4 for FTP transfers. You need the kerberos package\n"
"  installed and used at curl build time for it to be used.\n"
"\n"
"  First, get the krb-ticket the normal way, like with the kauth tool. Then use\n"
"  curl in way similar to:\n"
"\n"
);
 puts(
"        curl --krb4 private ftp://krb4site.com -u username:fakepwd\n"
"\n"
"  There's no use for a password on the -u switch, but a blank one will make\n"
"  curl ask for one and you already entered the real password to kauth.\n"
"\n"
"TELNET\n"
"\n"
"  The curl telnet support is basic and very easy to use. Curl passes all data\n"
"  passed to it on stdin to the remote server. Connect to a remote telnet\n"
"  server using a command line similar to:\n"
"\n"
"        curl telnet://remote.server.com\n"
"\n"
);
 puts(
"  And enter the data to pass to the server on stdin. The result will be sent\n"
"  to stdout or to the file you specify with -o.\n"
"\n"
"  You might want the -N/--no-buffer option to switch off the buffered output\n"
"  for slow connections or similar.\n"
"\n"
"  Pass options to the telnet protocol negotiation, by using the -t option. To\n"
"  tell the server we use a vt100 terminal, try something like:\n"
"\n"
"        curl -tTTYPE=vt100 telnet://remote.server.com\n"
"\n"
"  Other interesting options for it -t include:\n"
"\n"
);
 puts(
"   - XDISPLOC=<X display> Sets the X display location.\n"
"\n"
"   - NEW_ENV=<var,val> Sets an environment variable.\n"
"\n"
"  NOTE: the telnet protocol does not specify any way to login with a specified\n"
"  user and password so curl can't do that automatically. To do that, you need\n"
"  to track when the login prompt is received and send the username and\n"
"  password accordingly.\n"
"\n"
"PERSISTANT CONNECTIONS\n"
"\n"
"  Specifying multiple files on a single command line will make curl transfer\n"
);
 puts(
"  all of them, one after the other in the specified order.\n"
"\n"
"  libcurl will attempt to use persistant connections for the transfers so that\n"
"  the second transfer to the same host can use the same connection that was\n"
"  already initiated and was left open in the previous transfer. This greatly\n"
"  decreases connection time for all but the first transfer and it makes a far\n"
"  better use of the network.\n"
"\n"
"  Note that curl cannot use persistant connections for transfers that are used\n"
);
 puts(
"  in subsequence curl invokes. Try to stuff as many URLs as possible on the\n"
"  same command line if they are using the same host, as that'll make the\n"
"  transfers faster. If you use a http proxy for file transfers, practicly\n"
"  all transfers will be persistant.\n"
"\n"
"  Persistant connections were introduced in curl 7.7.\n"
"\n"
"MAILING LISTS\n"
"\n"
"  For your convenience, we have several open mailing lists to discuss curl,\n"
"  its development and things relevant to this. Get all info at\n"
);
 puts(
"  http://curl.haxx.se/mail/. The lists available are:\n"
"\n"
"  curl-users\n"
"\n"
"    Users of the command line tool. How to use it, what doesn't work, new\n"
"    features, related tools, questions, news, installations, compilations,\n"

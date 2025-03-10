fputs(
" uploaded to a remote FTP server using a Blue Coat FTP proxy with the\n"
" options:\n"
"\n"
"   curl -u \"Remote-FTP-Username@remote.ftp.server Proxy-Username:Remote-Pass\" \\\n"
"    --ftp-account Proxy-Password --upload-file local-file \\\n"
"    ftp://my-ftp.proxy.server:21/remote/upload/path/\n"
"\n"
" See the manual for your FTP proxy to determine the form it expects to set up\n"
" transfers, and curl's -v option to see exactly what curl is sending.\n"
"\n"
"RANGES\n"
"\n"
, stdout);
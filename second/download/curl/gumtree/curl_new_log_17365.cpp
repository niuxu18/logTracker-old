puts(
"  Upload a local file to the remote site, and use the local file name remote\n"
"  too:\n"
" \n"
"        curl -T uploadfile -u user:passwd ftp://ftp.upload.com/\n"
"\n"
"  Upload a local file to get appended to the remote file using ftp:\n"
"\n"
"        curl -T localfile -a ftp://ftp.upload.com/remotefile\n"
"\n"
"  Curl also supports ftp upload through a proxy, but only if the proxy is\n"
"  configured to allow that kind of tunneling. If it does, you can run curl in\n"
"  a fashion similar to:\n"
"\n"
);
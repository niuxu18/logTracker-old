puts(
"  configured to allow that kind of tunneling. If it does, you can run curl in\n"
"  a fashion similar to:\n"
"\n"
"        curl --proxytunnel -x proxy:port -T localfile ftp.upload.com\n"
"\n"
" HTTP\n"
"\n"
"  Upload all data on stdin to a specified http site:\n"
"\n"
"        curl -t http://www.upload.com/myfile\n"
"\n"
"  Note that the http server must've been configured to accept PUT before this\n"
"  can be done successfully.\n"
"\n"
"  For other ways to do http data upload, see the POST section below.\n"
"\n"
"VERBOSE / DEBUG\n"
"\n"
);
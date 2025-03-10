fputs(
"       --disable-epsv\n"
"              (FTP)  Tell  curl  to  disable  the use of the EPSV command when\n"
"              doing passive FTP transfers. Curl  will  normally  always  first\n"
"              attempt  to  use EPSV before PASV, but with this option, it will\n"
"              not try using EPSV.\n"
"\n"
"              If this option is used several times, each occurrence will  tog-\n"
"              gle this on/off.\n"
"\n"
"       -D/--dump-header <file>\n"
"              Write the protocol headers to the specified file.\n"
, stdout);
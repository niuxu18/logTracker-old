fputs(
"              the verification fails.\n"
"\n"
"              This  is  currently  only implemented in the OpenSSL, GnuTLS and\n"
"              NSS backends.  (Added in 7.41.0)\n"
"\n"
"       --false-start\n"
"\n"
"              (SSL) Tells curl to use false start during  the  TLS  handshake.\n"
"              False  start  is  a  mode  where a TLS client will start sending\n"
"              application data before verifying the server's Finished message,\n"
"              thus saving a round trip when performing a full handshake.\n"
"\n"
, stdout);
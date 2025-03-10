 , stdout);
  fputs(
 "              independently.\n"
 "\n"
 "              If curl is built against the NSS SSL library  then  this  option\n"
 "              tells curl the nickname of the certificate to use within the NSS\n"
-"              database defined by --cacert.\n"
+"              database defined by the  environment  variable  SSL_DIR  (or  by\n"
+"              default  /etc/pki/nssdb).  If  the  NSS PEM PKCS#11 module (lib‐\n"
+"              nsspem.so) is available then PEM files may be loaded.\n"
 "\n"
 "              If this option is used several times, the last one will be used.\n"
 "\n"
+, stdout);
+ fputs(
 "       --cert-type <type>\n"
 "              (SSL)  Tells curl what certificate type the provided certificate\n"
 "              is in. PEM, DER and ENG are recognized types.  If not specified,\n"
-, stdout);
- fputs(
 "              PEM is assumed.\n"
 "\n"
 "              If this option is used several times, the last one will be used.\n"
 "\n"
 "       --cacert <CA certificate>\n"
 "              (SSL) Tells curl to use the specified certificate file to verify\n"
 "              the  peer.  The  file  may contain multiple CA certificates. The\n"
+, stdout);
+ fputs(
 "              certificate(s) must be in PEM format.\n"
 "\n"
 "              curl recognizes the environment variable named  ’CURL_CA_BUNDLE’\n"
-, stdout);
- fputs(
 "              if  that  is set, and uses the given path as a path to a CA cert\n"
 "              bundle. This option overrides that variable.\n"
 "\n"
 "              The windows version of curl will automatically  look  for  a  CA\n"
 "              certs file named ´curl-ca-bundle.crt´, either in the same direc‐\n"
+, stdout);
+ fputs(
 "              tory as curl.exe, or in the Current Working Directory, or in any\n"
 "              folder along your PATH.\n"
 "\n"
 "              If  curl  is  built against the NSS SSL library then this option\n"
+"              tells curl the nickname of the CA certificate to use within  the\n"
+"              NSS  database defined by the environment variable SSL_DIR (or by\n"
+"              default /etc/pki/nssdb).  If the NSS PEM  PKCS#11  module  (lib‐\n"
 , stdout);
  fputs(
-"              tells curl the  directory  that  the  NSS  certificate  database\n"
-"              resides in.\n"
+"              nsspem.so) is available then PEM files may be loaded.\n"
 "\n"
 "              If this option is used several times, the last one will be used.\n"
 "\n"
 "       --capath <CA certificate directory>\n"
 "              (SSL) Tells curl to use the specified certificate  directory  to\n"
 "              verify the peer. The certificates must be in PEM format, and the\n"

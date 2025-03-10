 "  Secure HTTP requires SSL libraries to be installed and used when curl is\n"
 "  built. If that is done, curl is capable of retrieving and posting documents\n"
 "  using the HTTPS protocol.\n"
 "\n"
 "  Example:\n"
 "\n"
+, stdout);
+ fputs(
 "        curl https://www.secure-site.com\n"
 "\n"
 "  Curl is also capable of using your personal certificates to get/post files\n"
-, stdout);
- fputs(
 "  from sites that require valid certificates. The only drawback is that the\n"
 "  certificate needs to be in PEM-format. PEM is a standard and open format to\n"
 "  store certificates with, but it is not used by the most commonly used\n"
 "  browsers (Netscape and MSIE both use the so called PKCS#12 format). If you\n"
-"  want curl to use the certificates you use with your (favourite) browser, you\n"
-"  may need to download/compile a converter that can convert your browser's\n"
 , stdout);
  fputs(
+"  want curl to use the certificates you use with your (favourite) browser, you\n"
+"  may need to download/compile a converter that can convert your browser's\n"
 "  formatted certificates to PEM formatted ones. This kind of converter is\n"
 "  included in recent versions of OpenSSL, and for older versions Dr Stephen\n"
 "  N. Henson has written a patch for SSLeay that adds this functionality. You\n"
 "  can get his patch (that requires an SSLeay installation) from his site at:\n"
 "  http://www.drh-consultancy.demon.co.uk/\n"
+, stdout);
+ fputs(
 "\n"
 "  Example on how to automatically retrieve a document using a certificate with\n"
 "  a personal password:\n"
 "\n"
-, stdout);
- fputs(
 "        curl -E /path/to/cert.pem:password https://secure.site.com/\n"
 "\n"
 "  If you neglect to specify the password on the command line, you will be\n"
 "  prompted for the correct password before any data can be received.\n"
 "\n"
 "  Many older SSL-servers have problems with SSLv3 or TLS, that newer versions\n"
 "  of OpenSSL etc is using, therefore it is sometimes useful to specify what\n"
-"  SSL-version curl should use. Use -3, -2 or -1 to specify that exact SSL\n"
-"  version to use (for SSLv3, SSLv2 or TLSv1 respectively):\n"
 , stdout);
  fputs(
+"  SSL-version curl should use. Use -3, -2 or -1 to specify that exact SSL\n"
+"  version to use (for SSLv3, SSLv2 or TLSv1 respectively):\n"
 "\n"
 "        curl -2 https://secure.site.com/\n"
 "\n"
 "  Otherwise, curl will first attempt to use v3 and then v2.\n"
 "\n"
 "  To use OpenSSL to convert your favourite browser's certificate into a PEM\n"
 "  formatted one that curl can use, do something like this (assuming netscape,\n"
 "  but IE is likely to work similarly):\n"
 "\n"
 "    You start with hitting the 'security' menu button in netscape.\n"
 "\n"
+, stdout);
+ fputs(
 "    Select 'certificates->yours' and then pick a certificate in the list\n"
 "\n"
 "    Press the 'export' button\n"
 "\n"
-, stdout);
- fputs(
 "    enter your PIN code for the certs\n"
 "\n"
 "    select a proper place to save it\n"
 "\n"
 "    Run the 'openssl' application to convert the certificate. If you cd to the\n"
 "    openssl installation, you can do it like:\n"

"\n"
"     -D/--dump-header <file>\n"
"          (HTTP/FTP) Write the HTTP headers to this file. Write\n"
"          the FTP file info to this file if -I/--head is used.\n"
"\n"
"          This option is handy to use when you want to store the\n"
);
 puts(
"          cookies that a HTTP site sends to you. The cookies\n"
"          could then be read in a second curl invoke by using the\n"
"          -b/--cookie option!\n"
"\n"
"          If this option is used serveral times, the last one\n"
"          will be used.\n"
"\n"
"     -e/--referer <URL>\n"
"          (HTTP) Sends the \"Referer Page\" information to the HTTP\n"
"          server. This can also be set with the -H/--header flag\n"
"          of course.  When used with -L/--location you can append\n"
);
 puts(
"          \";auto\" to the referer URL to make curl automatically\n"
"          set the previous URL when it follows a Location:\n"
"          header. The \";auto\" string can be used alone, even if\n"
"          you don't set an initial referer.\n"
"\n"
"          If this option is used serveral times, the last one\n"
"          will be used.\n"
"\n"
"     --egd-file <file>\n"
"          (HTTPS) Specify the path name to the Entropy Gathering\n"
"          Daemon socket. The socket is used to seed the random\n"
);
 puts(
"          engine for SSL connections. See also the --random-file\n"
"          option.\n"
"\n"
"     -E/--cert <certificate[:password]>\n"
"          (HTTPS) Tells curl to use the specified certificate\n"
"          file when getting a file with HTTPS. The certificate\n"
"          must be in PEM format.  If the optional password isn't\n"
"          specified, it will be queried for on the terminal. Note\n"
"          that this certificate is the private key and the pri�\n"
"          vate certificate concatenated!\n"
"\n"
);
 puts(
"          If this option is used serveral times, the last one\n"
"          will be used.\n"
"\n"
"     --cacert <CA certificate>\n"
"          (HTTPS) Tells curl to use the specified certificate\n"
"          file to verify the peer. The certificate must be in PEM\n"

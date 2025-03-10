    fprintf(stderr, "                    are a colon separated username and password.\n");
    fprintf(stderr, "    -X proxy:port   Proxyserver and port number to use\n");
    fprintf(stderr, "    -V              Print version number and exit\n");
    fprintf(stderr, "    -k              Use HTTP KeepAlive feature\n");
    fprintf(stderr, "    -d              Do not show percentiles served table.\n");
    fprintf(stderr, "    -S              Do not show confidence estimators and warnings.\n");
    fprintf(stderr, "    -g filename     Output collected data to gnuplot format file.\n");
    fprintf(stderr, "    -e filename     Output CSV file with percentages served\n");
    fprintf(stderr, "    -r              Don't exit on socket receive errors.\n");
    fprintf(stderr, "    -h              Display usage information (this message)\n");
#ifdef USE_SSL
    fprintf(stderr, "    -Z ciphersuite  Specify SSL/TLS cipher suite (See openssl ciphers)\n");
#ifndef OPENSSL_NO_SSL2
    fprintf(stderr, "    -f protocol     Specify SSL/TLS protocol (SSL2, SSL3, TLS1, or ALL)\n");
#else
    fprintf(stderr, "    -f protocol     Specify SSL/TLS protocol (SSL3, TLS1, or ALL)\n");
#endif
#endif
    exit(EINVAL);
}

/* ------------------------------------------------------- */


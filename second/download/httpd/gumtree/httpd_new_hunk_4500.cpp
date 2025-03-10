#ifdef HAVE_TLSV1_X
#define TLS1_X_HELP_MSG ", TLS1.1, TLS1.2"
#else
#define TLS1_X_HELP_MSG ""
#endif

#ifdef HAVE_TLSEXT
    fprintf(stderr, "    -I              Disable TLS Server Name Indication (SNI) extension\n");
#endif
    fprintf(stderr, "    -Z ciphersuite  Specify SSL/TLS cipher suite (See openssl ciphers)\n");
    fprintf(stderr, "    -f protocol     Specify SSL/TLS protocol\n");
    fprintf(stderr, "                    (" SSL2_HELP_MSG SSL3_HELP_MSG "TLS1" TLS1_X_HELP_MSG " or ALL)\n");
#endif
    exit(EINVAL);
}

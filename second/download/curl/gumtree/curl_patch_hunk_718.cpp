 "  decreases connection time for all but the first transfer and it makes a far\n"
 "  better use of the network.\n"
 "\n"
 "  Note that curl cannot use persistent connections for transfers that are used\n"
 "  in subsequence curl invokes. Try to stuff as many URLs as possible on the\n"
 "  same command line if they are using the same host, as that'll make the\n"
-"  transfers faster. If you use a http proxy for file transfers, practically\n"
+"  transfers faster. If you use an HTTP proxy for file transfers, practically\n"
 , stdout);
  fputs(
 "  all transfers will be persistent.\n"
 "\n"
 "MULTIPLE TRANSFERS WITH A SINGLE COMMAND LINE\n"
 "\n"

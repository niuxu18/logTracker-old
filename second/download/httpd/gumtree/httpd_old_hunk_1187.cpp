    fprintf(stderr, "Options are:\n");
    fprintf(stderr, "    -n requests     Number of requests to perform\n");
    fprintf(stderr, "    -c concurrency  Number of multiple requests to make\n");
    fprintf(stderr, "    -t timelimit    Seconds to max. wait for responses\n");
    fprintf(stderr, "    -b windowsize   Size of TCP send/receive buffer, in bytes\n");
    fprintf(stderr, "    -p postfile     File containing data to POST. Remember also to set -T\n");
    fprintf(stderr, "    -T content-type Content-type header for POSTing, eg.\n");
    fprintf(stderr, "                    'application/x-www-form-urlencoded'\n");
    fprintf(stderr, "                    Default is 'text/plain'\n");
    fprintf(stderr, "    -v verbosity    How much troubleshooting info to print\n");
    fprintf(stderr, "    -w              Print out results in HTML tables\n");
    fprintf(stderr, "    -i              Use HEAD instead of GET\n");

fputs(
"              second and then for all forthcoming retries it will  double  the\n"
"              waiting  time until it reaches 10 minutes which then will be the\n"
"              delay between the rest of the retries.  By  using  --retry-delay\n"
"              you   disable  this  exponential  backoff  algorithm.  See  also\n"
"              --retry-max-time to limit the total time allowed for retries.\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
, stdout);
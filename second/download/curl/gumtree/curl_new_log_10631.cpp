fputs(
"              all  transfers are done using the cache. Note that while nothing\n"
"              should ever get hurt by attempting  to  reuse  SSL  session-IDs,\n"
"              there seem to be broken SSL implementations in the wild that may\n"
"              require you to disable this in order for you to succeed.  (Added\n"
"              in 7.16.0)\n"
"\n"
"              Note  that  this  is the negated option name documented. You can\n"
"              thus use --sessionid to enforce session-ID caching.\n"
"\n"
, stdout);
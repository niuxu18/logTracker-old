fputs(
"        curl -m 1800 -Y 3000 -y 60 www.far-away-site.com\n"
"\n"
"  Forcing curl not to transfer data faster than a given rate is also possible,\n"
"  which might be useful if you're using a limited bandwidth connection and you\n"
"  don't want your transfer to use all of it (sometimes referred to as\n"
"  \"bandwith throttle\").\n"
"\n"
"  Make curl transfer data no faster than 10 kilobytes per second:\n"
"\n"
"        curl --limit-rate 10K www.far-away-site.com\n"
"\n"
"    or\n"
"\n"
"        curl --limit-rate 10240 www.far-away-site.com\n"
"\n"
, stdout);
fputs(
"              see --url for details.\n"
"\n"
"              Added in 7.45.0.\n"
"\n"
"       --proto-redir <protocols>\n"
"              Tells curl to limit what protocols it may use on redirect.  Pro-\n"
"              tocols  denied by --proto are not overridden by this option. See\n"
"              --proto for how protocols are represented.\n"
"\n"
"              Example, allow only HTTP and HTTPS on redirect:\n"
"\n"
"               curl --proto-redir -all,http,https http://example.com\n"
"\n"
, stdout);
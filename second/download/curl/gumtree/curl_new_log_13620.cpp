fputs(
"              (HTTP) Tells curl to respect RFC 7230/6.4.4 and not convert POST\n"
"              requests into GET requests when following a 303 redirection. The\n"
"              non-RFC  behaviour  is  ubiquitous in web browsers, so curl does\n"
"              the conversion by default to maintain  consistency.  However,  a\n"
"              server  may  require  a POST to remain a POST after such a redi-\n"
"              rection. This option is meaningful only when using  -L,  --loca-\n"
, stdout);
fputs(
"  the kerberos package installed and used at curl build time for it to be\n"
"  available.\n"
"\n"
"  First, get the krb-ticket the normal way, like with the kinit/kauth tool.\n"
"  Then use curl in way similar to:\n"
"\n"
"        curl --krb private ftp://krb4site.com -u username:fakepwd\n"
"\n"
"  There's no use for a password on the -u switch, but a blank one will make\n"
"  curl ask for one and you already entered the real password to kinit/kauth.\n"
"\n"
"TELNET\n"
"\n"
, stdout);
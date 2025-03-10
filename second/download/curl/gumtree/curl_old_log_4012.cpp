fputs(
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
"  The curl telnet support is basic and very easy to use. Curl passes all data\n"
"  passed to it on stdin to the remote server. Connect to a remote telnet\n"
, stdout);
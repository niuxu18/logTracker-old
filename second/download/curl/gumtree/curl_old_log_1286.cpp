fputs(
"  curl, the -w/--write-out option was introduced. Using this, you can specify\n"
"  what information from the previous transfer you want to extract.\n"
"\n"
"  To display the amount of bytes downloaded together with some text and an\n"
"  ending newline:\n"
"\n"
"        curl -w 'We downloaded %{size_download} bytes\\n' www.download.com\n"
"\n"
"KERBEROS4 FTP TRANSFER\n"
"\n"
"  Curl supports kerberos4 for FTP transfers. You need the kerberos package\n"
"  installed and used at curl build time for it to be used.\n"
"\n"
, stdout);
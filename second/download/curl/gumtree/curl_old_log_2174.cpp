fputs(
"       20     FTP write error. The transfer was reported bad by the server.\n"
"\n"
"       21     FTP quote error. A quote command returned error from the server.\n"
"       22     HTTP page not retrieved. The requested  url  was  not  found  or\n"
"              returned  another  error  with  the HTTP error code being 400 or\n"
"              above. This return code only appears if -f/--fail is used.\n"
"\n"
"       23     Write error. Curl couldn't write data to a local  filesystem  or\n"
"              similar.\n"
"\n"
, stdout);
"          prevent that header from appearing.\n"
"\n"
"     -i/--include\n"
"          (HTTP) Include the HTTP-header in the output. The HTTP-\n"
"          header  includes  things  like server-name, date of the\n"
"          document, HTTP-version and more...\n"
"     -I/--head\n"
"          (HTTP/FTP) Fetch  the  HTTP-header  only!  HTTP-servers\n"
"          feature the command HEAD which this uses to get nothing\n"
"          but the header of a document. When used on a FTP  file,\n"
"          curl displays the file size only.\n"
"\n"
"     -K/--config <config file>\n"
"          Specify  which config file to read curl arguments from.\n"
"          The config file is a text file in  which  command  line\n"
"          arguments  can be written which then will be used as if\n"
"          they were written on the actual command  line.  If  the\n"
"          first  column  of a config line is a '#' character, the\n"
);
 puts(
"          rest of the line will be treated as a comment.\n"
"\n"
"          Specify the filename as '-' to make curl read the  file\n"
"          from stdin.\n"
"\n"
"     -l/--list-only\n"
"          (FTP) When listing an FTP directory, this switch forces\n"
"          a name-only view.  Especially useful  if  you  want  to\n"
"          machine-parse  the  contents  of an FTP directory since\n"
"          the normal directory view doesn't use a  standard  look\n"
"          or format.\n"
"\n"
"     -L/--location\n"
"          (HTTP/HTTPS)  If  the server reports that the requested\n"
"          page has  a  different  location  (indicated  with  the\n"
"          header  line Location:) this flag will let curl attempt\n"
"          to reattempt the get on the new place. If used together\n"
"          with -i or -I, headers from all requested pages will be\n"
"          shown. If this flag is used when making  a  HTTP  POST,\n"
"          curl will automatically switch to GET after the initial\n"
"          POST has been done.\n"
"\n"

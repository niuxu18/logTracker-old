"              submit  button.  Note that the data is sent exactly\n"
"              as specified with no  extra  processing  (with  all\n"
"              newlines  cut  off).   The  data  is expected to be\n"
"              \"url-encoded\". This will cause  curl  to  pass  the\n"
, stdout);
 fputs(
"              data  to the server using the content-type applica�\n"
"              tion/x-www-form-urlencoded. Compare  to  -F/--form.\n"
"              If  this  option is used more than once on the same\n"
"              command line, the data  pieces  specified  will  be\n"
"              merged  together  with a separating &-letter. Thus,\n"
"              using '-d name=daniel -d skill=lousy' would  gener�\n"
"              ate     a    post    chunk    that    looks    like\n"
, stdout);
 fputs(
"              'name=daniel&skill=lousy'.\n"
"\n"
"              If you start the data with the letter @,  the  rest\n"
"              should  be  a file name to read the data from, or -\n"

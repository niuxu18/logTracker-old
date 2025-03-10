fputs(
"       -o/--output <file>\n"
"              Write output to <file> instead of stdout. If you are using {} or\n"
"              []  to  fetch  multiple documents, you can use '#' followed by a\n"
"              number in the <file> specifier. That variable will  be  replaced\n"
"              with the current string for the URL being fetched. Like in:\n"
"\n"
"                curl http://{one,two}.site.com -o \"file_#1.txt\"\n"
"\n"
"              or use several variables like:\n"
"\n"
, stdout);
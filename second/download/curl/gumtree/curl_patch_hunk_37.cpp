 "\n"
 "        machine curl.haxx.se login iamdaniel password mysecret\n"
 "\n"
 "CUSTOM OUTPUT\n"
 "\n"
 "  To better allow script programmers to get to know about the progress of\n"
-);
- puts(
+, stdout);
+ fputs(
 "  curl, the -w/--write-out option was introduced. Using this, you can specify\n"
 "  what information from the previous transfer you want to extract.\n"
 "\n"
 "  To display the amount of bytes downloaded together with some text and an\n"
 "  ending newline:\n"
 "\n"

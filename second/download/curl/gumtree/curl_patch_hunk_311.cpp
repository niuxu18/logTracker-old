 "  Get the first 100 bytes of a document using FTP:\n"
 "\n"
 "        curl -r 0-99 ftp://www.get.this/README  \n"
 "\n"
 "UPLOADING\n"
 "\n"
-" FTP\n"
+" FTP / FTPS / SFTP / SCP\n"
 "\n"
-, stdout);
- fputs(
-"  Upload all data on stdin to a specified ftp site:\n"
+"  Upload all data on stdin to a specified server:\n"
 "\n"
 "        curl -T - ftp://ftp.upload.com/myfile\n"
 "\n"
 "  Upload data from a specified file, login with user and password:\n"
+, stdout);
+ fputs(
 "\n"
 "        curl -T uploadfile -u user:passwd ftp://ftp.upload.com/myfile\n"
 "\n"
 "  Upload a local file to the remote site, and use the local file name remote\n"
 "  too:\n"
 " \n"
 "        curl -T uploadfile -u user:passwd ftp://ftp.upload.com/\n"
 "\n"
-"  Upload a local file to get appended to the remote file using ftp:\n"
+"  Upload a local file to get appended to the remote file:\n"
 "\n"
-, stdout);
- fputs(
 "        curl -T localfile -a ftp://ftp.upload.com/remotefile\n"
 "\n"
 "  Curl also supports ftp upload through a proxy, but only if the proxy is\n"
 "  configured to allow that kind of tunneling. If it does, you can run curl in\n"
+, stdout);
+ fputs(
 "  a fashion similar to:\n"
 "\n"
 "        curl --proxytunnel -x proxy:port -T localfile ftp.upload.com\n"
 "\n"
 " HTTP\n"
 "\n"
 "  Upload all data on stdin to a specified http site:\n"
 "\n"
 "        curl -T - http://www.upload.com/myfile\n"
 "\n"
 "  Note that the http server must have been configured to accept PUT before\n"
-, stdout);
- fputs(
 "  this can be done successfully.\n"
 "\n"
 "  For other ways to do http data upload, see the POST section below.\n"
 "\n"
 "VERBOSE / DEBUG\n"
 "\n"
 "  If curl fails where it isn't supposed to, if the servers don't let you in,\n"
+, stdout);
+ fputs(
 "  if you can't understand the responses: use the -v flag to get verbose\n"
 "  fetching. Curl will output lots of info and what it sends and receives in\n"
 "  order to let the user see all client-server interaction (but it won't show\n"
 "  you the actual data).\n"
 "\n"
 "        curl -v ftp://ftp.upload.com/\n"
 "\n"
-, stdout);
- fputs(
 "  To get even more details and information on what curl does, try using the\n"
 "  --trace or --trace-ascii options with a given file name to log to, like\n"
 "  this:\n"
 "\n"
 "        curl --trace trace.txt www.haxx.se\n"
 " \n"
 "\n"
+, stdout);
+ fputs(
 "DETAILED INFORMATION\n"
 "\n"
 "  Different protocols provide different ways of getting detailed information\n"
 "  about specific files/documents. To get curl to show detailed information\n"
 "  about a single file, you should use -I/--head option. It displays all\n"
-, stdout);
- fputs(
 "  available info on a single file for HTTP and FTP. The HTTP information is a\n"
 "  lot more extensive.\n"
 "\n"
 "  For HTTP, you can get the header information (the same as -I would show)\n"
 "  shown before the data by using -i/--include. Curl understands the\n"
+, stdout);
+ fputs(
 "  -D/--dump-header option when getting files from both FTP and HTTP, and it\n"
 "  will then store the headers in the specified file.\n"
 "\n"
 "  Store the HTTP headers in a separate file (headers.txt in the example):\n"
 "\n"
-, stdout);
- fputs(
 "        curl --dump-header headers.txt curl.haxx.se\n"
 "\n"
 "  Note that headers stored in a separate file can be very useful at a later\n"
 "  time if you want curl to use cookies sent by the server. More about that in\n"
 "  the cookies section.\n"
 "\n"
 "POST (HTTP)\n"
 "\n"
+, stdout);
+ fputs(
 "  It's easy to post data using curl. This is done using the -d <data>\n"
 "  option.  The post data must be urlencoded.\n"
 "\n"
 "  Post a simple \"name\" and \"phone\" guestbook.\n"
 "\n"
 "        curl -d \"name=Rafael%20Sagula&phone=3320780\" \\\n"
-, stdout);
- fputs(
 "                http://www.where.com/guest.cgi\n"
 "\n"
 "  How to post a form with curl, lesson #1:\n"
 "\n"
 "  Dig out all the <input> tags in the form that you want to fill in. (There's\n"
 "  a perl program called formfind.pl on the curl site that helps with this).\n"
 "\n"
+, stdout);
+ fputs(
 "  If there's a \"normal\" post, you use -d to post. -d takes a full \"post\n"
 "  string\", which is in the format\n"
 "\n"
 "        <variable1>=<data1>&<variable2>=<data2>&...\n"
 "\n"
 "  The 'variable' names are the names set with \"name=\" in the <input> tags, and\n"
-, stdout);
- fputs(
 "  the data is the contents you want to fill in for the inputs. The data *must*\n"
 "  be properly URL encoded. That means you replace space with + and that you\n"
 "  write weird letters with %XX where XX is the hexadecimal representation of\n"
+, stdout);
+ fputs(
 "  the letter's ASCII code.\n"
 "\n"
 "  Example:\n"
 "\n"
 "  (page located at http://www.formpost.com/getthis/\n"
 "\n"
 "        <form action=\"post.cgi\" method=\"post\">\n"
 "        <input name=user size=10>\n"
 "        <input name=pass type=password size=10>\n"
-, stdout);
- fputs(
 "        <input name=id type=hidden value=\"blablabla\">\n"
 "        <input name=ding value=\"submit\">\n"
 "        </form>\n"
 "\n"
 "  We want to enter user 'foobar' with password '12345'.\n"
 "\n"
 "  To post to this, you enter a curl command line like:\n"
 "\n"
+, stdout);
+ fputs(
 "        curl -d \"user=foobar&pass=12345&id=blablabla&ding=submit\"  (continues)\n"
 "          http://www.formpost.com/getthis/post.cgi\n"
 "\n"
 "\n"
 "  While -d uses the application/x-www-form-urlencoded mime-type, generally\n"
-, stdout);
- fputs(
 "  understood by CGI's and similar, curl also supports the more capable\n"
 "  multipart/form-data type. This latter type supports things like file upload.\n"
 "\n"
 "  -F accepts parameters like -F \"name=contents\". If you want the contents to\n"
+, stdout);
+ fputs(
 "  be read from a file, use <@filename> as contents. When specifying a file,\n"
 "  you can also specify the file content type by appending ';type=<mime type>'\n"
 "  to the file name. You can also post the contents of several files in one\n"
-, stdout);
- fputs(
 "  field.  For example, the field name 'coolfiles' is used to send three files,\n"
 "  with different content types using the following syntax:\n"
 "\n"
 "        curl -F \"coolfiles=@fil1.gif;type=image/gif,fil2.txt,fil3.html\" \\\n"
 "        http://www.post.com/postit.cgi\n"
 "\n"
+, stdout);
+ fputs(
 "  If the content-type is not specified, curl will try to guess from the file\n"
 "  extension (it only knows a few), or use the previously specified type (from\n"
 "  an earlier file if several files are specified in a list) or else it will\n"
-, stdout);
- fputs(
 "  using the default type 'text/plain'.\n"
 "\n"
 "  Emulate a fill-in form with -F. Let's say you fill in three fields in a\n"
 "  form. One field is a file name which to post, one field is your name and one\n"
+, stdout);
+ fputs(
 "  field is a file description. We want to post the file we have written named\n"
 "  \"cooltext.txt\". To let curl do the posting of this data instead of your\n"
 "  favourite browser, you have to read the HTML source of the form page and\n"
 "  find the names of the input fields. In our example, the input field names\n"
-, stdout);
- fputs(
 "  are 'file', 'yourname' and 'filedescription'.\n"
 "\n"
 "        curl -F \"file=@cooltext.txt\" -F \"yourname=Daniel\" \\\n"
 "             -F \"filedescription=Cool text file with cool text inside\" \\\n"
+, stdout);
+ fputs(
 "             http://www.post.com/postit.cgi\n"
 "\n"
 "  To send two files in one post you can do it in two ways:\n"
 "\n"
 "  1. Send multiple files in a single \"field\" with a single field name:\n"
 " \n"
 "        curl -F \"pictures=@dog.gif,cat.gif\" \n"
 " \n"
 "  2. Send two fields with two field names: \n"
 "\n"
-, stdout);
- fputs(
 "        curl -F \"docpicture=@dog.gif\" -F \"catpicture=@cat.gif\" \n"
 "\n"
 "  To send a field value literally without interpreting a leading '@'\n"
 "  or '<', or an embedded ';type=', use --form-string instead of\n"
+, stdout);
+ fputs(
 "  -F. This is recommended when the value is obtained from a user or\n"
 "  some other unpredictable source. Under these circumstances, using\n"
 "  -F instead of --form-string would allow a user to trick curl into\n"
 "  uploading a file.\n"
 "\n"
 "REFERRER\n"
 "\n"
-, stdout);
- fputs(
 "  A HTTP request has the option to include information about which address\n"
 "  that referred to actual page.  Curl allows you to specify the\n"
 "  referrer to be used on the command line. It is especially useful to\n"
+, stdout);
+ fputs(
 "  fool or trick stupid servers or CGI scripts that rely on that information\n"
 "  being available or contain certain data.\n"
 "\n"
 "        curl -e www.coolsite.com http://www.showme.com/\n"
 "\n"
-"  NOTE: The referer field is defined in the HTTP spec to be a full URL.\n"
+"  NOTE: The Referer: [sic] field is defined in the HTTP spec to be a full URL.\n"
 "\n"
 "USER AGENT\n"
 "\n"
-, stdout);
- fputs(
 "  A HTTP request has the option to include information about the browser\n"
 "  that generated the request. Curl allows it to be specified on the command\n"
 "  line. It is especially useful to fool or trick stupid servers or CGI\n"
+, stdout);
+ fputs(
 "  scripts that only accept certain browsers.\n"
 "\n"
 "  Example:\n"
 "\n"
 "  curl -A 'Mozilla/3.0 (Win95; I)' http://www.nationsbank.com/\n"
 "\n"
 "  Other common strings:\n"
 "    'Mozilla/3.0 (Win95; I)'     Netscape Version 3 for Windows 95\n"
 "    'Mozilla/3.04 (Win95; U)'    Netscape Version 3 for Windows 95\n"
-, stdout);
- fputs(
 "    'Mozilla/2.02 (OS/2; U)'     Netscape Version 2 for OS/2\n"
 "    'Mozilla/4.04 [en] (X11; U; AIX 4.2; Nav)'           NS for AIX\n"
 "    'Mozilla/4.05 [en] (X11; U; Linux 2.0.32 i586)'      NS for Linux\n"
 "\n"
+, stdout);
+ fputs(
 "  Note that Internet Explorer tries hard to be compatible in every way:\n"
 "    'Mozilla/4.0 (compatible; MSIE 4.01; Windows 95)'    MSIE for W95\n"
 "\n"
 "  Mozilla is not the only possible User-Agent name:\n"
 "    'Konqueror/1.0'             KDE File Manager desktop client\n"
-, stdout);
- fputs(
 "    'Lynx/2.7.1 libwww-FM/2.14' Lynx command line browser\n"
 "\n"
 "COOKIES\n"
 "\n"
 "  Cookies are generally used by web servers to keep state information at the\n"
 "  client's side. The server sets cookies by sending a response line in the\n"
+, stdout);
+ fputs(
 "  headers that looks like 'Set-Cookie: <data>' where the data part then\n"
 "  typically contains a set of NAME=VALUE pairs (separated by semicolons ';'\n"
 "  like \"NAME1=VALUE1; NAME2=VALUE2;\"). The server can also specify for what\n"
-, stdout);
- fputs(
 "  path the \"cookie\" should be used for (by specifying \"path=value\"), when the\n"
 "  cookie should expire (\"expire=DATE\"), for what domain to use it\n"
 "  (\"domain=NAME\") and if it should be used on secure connections only\n"
 "  (\"secure\").\n"
 "\n"
+, stdout);
+ fputs(
 "  If you've received a page from a server that contains a header like:\n"
 "        Set-Cookie: sessionid=boo123; path=\"/foo\";\n"
 "\n"
 "  it means the server wants that first pair passed on when we get anything in\n"
 "  a path beginning with \"/foo\".\n"
 "\n"
-, stdout);
- fputs(
 "  Example, get a page that wants my name passed in a cookie:\n"
 "\n"
 "        curl -b \"name=Daniel\" www.sillypage.com\n"
 "\n"
 "  Curl also has the ability to use previously received cookies in following\n"
 "  sessions. If you get cookies from a server and store them in a file in a\n"
+, stdout);
+ fputs(
 "  manner similar to:\n"
 "\n"
 "        curl --dump-header headers www.example.com\n"
 "\n"
 "  ... you can then in a second connect to that (or another) site, use the\n"
 "  cookies from the 'headers' file like:\n"
 "\n"
 "        curl -b headers www.example.com\n"
 "\n"
-, stdout);
- fputs(
 "  While saving headers to a file is a working way to store cookies, it is\n"
 "  however error-prone and not the preferred way to do this. Instead, make curl\n"
 "  save the incoming cookies using the well-known netscape cookie format like\n"
 "  this:\n"
 "\n"
+, stdout);
+ fputs(
 "        curl -c cookies.txt www.example.com\n"
 "\n"
 "  Note that by specifying -b you enable the \"cookie awareness\" and with -L\n"
 "  you can make curl follow a location: (which often is used in combination\n"
-, stdout);
- fputs(
 "  with cookies). So that if a site sends cookies and a location, you can\n"
 "  use a non-existing file to trigger the cookie awareness like:\n"
 "\n"
 "        curl -L -b empty.txt www.example.com\n"
 "\n"
 "  The file to read cookies from must be formatted using plain HTTP headers OR\n"
+, stdout);
+ fputs(
 "  as netscape's cookie file. Curl will determine what kind it is based on the\n"
 "  file contents.  In the above command, curl will parse the header and store\n"
 "  the cookies received from www.example.com.  curl will send to the server the\n"
-, stdout);
- fputs(
 "  stored cookies which match the request as it follows the location.  The\n"
 "  file \"empty.txt\" may be a nonexistent file.\n"
 "\n"
 "  Alas, to both read and write cookies from a netscape cookie file, you can\n"
 "  set both -b and -c to use the same file:\n"
 "\n"
+, stdout);
+ fputs(
 "        curl -b cookies.txt -c cookies.txt www.example.com\n"
 "\n"
 "PROGRESS METER\n"
 "\n"
 "  The progress meter exists to show a user that something actually is\n"
 "  happening. The different fields in the output have the following meaning:\n"
 "\n"
-, stdout);
- fputs(
 "  % Total    % Received % Xferd  Average Speed          Time             Curr.\n"
 "                                 Dload  Upload Total    Current  Left    Speed\n"
 "  0  151M    0 38608    0     0   9406      0  4:41:43  0:00:04  4:41:39  9287\n"
 "\n"
 "  From left-to-right:\n"
+, stdout);
+ fputs(
 "   %             - percentage completed of the whole transfer\n"
 "   Total         - total size of the whole expected transfer\n"
 "   %             - percentage completed of the download\n"
 "   Received      - currently downloaded amount of bytes\n"
-, stdout);
- fputs(
 "   %             - percentage completed of the upload\n"
 "   Xferd         - currently uploaded amount of bytes\n"
 "   Average Speed\n"
 "   Dload         - the average transfer speed of the download\n"
 "   Average Speed\n"
 "   Upload        - the average transfer speed of the upload\n"
+, stdout);
+ fputs(
 "   Time Total    - expected time to complete the operation\n"
 "   Time Current  - time passed since the invoke\n"
 "   Time Left     - expected time left to completion\n"
 "   Curr.Speed    - the average transfer speed the last 5 seconds (the first\n"
-, stdout);
- fputs(
 "                   5 seconds of a transfer is based on less time of course.)\n"
 "\n"
 "  The -# option will display a totally different progress bar that doesn't\n"
 "  need much explanation!\n"
 "\n"
 "SPEED LIMIT\n"
 "\n"
+, stdout);
+ fputs(
 "  Curl allows the user to set the transfer speed conditions that must be met\n"
 "  to let the transfer keep going. By using the switch -y and -Y you\n"
 "  can make curl abort transfers if the transfer speed is below the specified\n"
 "  lowest limit for a specified time.\n"
 "\n"
-, stdout);
- fputs(
 "  To have curl abort the download if the speed is slower than 3000 bytes per\n"
 "  second for 1 minute, run:\n"
 "\n"
 "        curl -Y 3000 -y 60 www.far-away-site.com\n"
 "\n"
 "  This can very well be used in combination with the overall time limit, so\n"
+, stdout);
+ fputs(
 "  that the above operation must be completed in whole within 30 minutes:\n"
 "\n"
 "        curl -m 1800 -Y 3000 -y 60 www.far-away-site.com\n"
 "\n"
 "  Forcing curl not to transfer data faster than a given rate is also possible,\n"
-, stdout);
- fputs(
 "  which might be useful if you're using a limited bandwidth connection and you\n"
 "  don't want your transfer to use all of it (sometimes referred to as\n"
 "  \"bandwidth throttle\").\n"
 "\n"
 "  Make curl transfer data no faster than 10 kilobytes per second:\n"
 "\n"
+, stdout);
+ fputs(
 "        curl --limit-rate 10K www.far-away-site.com\n"
 "\n"
 "    or\n"
 "\n"
 "        curl --limit-rate 10240 www.far-away-site.com\n"
 "\n"
 "  Or prevent curl from uploading data faster than 1 megabyte per second:\n"
 "\n"
 "        curl -T upload --limit-rate 1M ftp://uploadshereplease.com\n"
 "\n"
-, stdout);
- fputs(
 "  When using the --limit-rate option, the transfer rate is regulated on a\n"
 "  per-second basis, which will cause the total transfer speed to become lower\n"
 "  than the given number. Sometimes of course substantially lower, if your\n"
+, stdout);
+ fputs(
 "  transfer stalls during periods.\n"
 "\n"
 "CONFIG FILE\n"
 "\n"
 "  Curl automatically tries to read the .curlrc file (or _curlrc file on win32\n"
 "  systems) from the user's home dir on startup.\n"
 "\n"
 "  The config file could be made up with normal command line switches, but you\n"
-, stdout);
- fputs(
 "  can also specify the long options without the dashes to make it more\n"
 "  readable. You can separate the options and the parameter with spaces, or\n"
 "  with = or :. Comments can be used within the file. If the first letter on a\n"
+, stdout);
+ fputs(
 "  line is a '#'-letter the rest of the line is treated as a comment.\n"
 "\n"
-"  If you want the parameter to contain spaces, you must inclose the entire\n"
+"  If you want the parameter to contain spaces, you must enclose the entire\n"
 "  parameter within double quotes (\"). Within those quotes, you specify a\n"
 "  quote as \\\".\n"
 "\n"
-, stdout);
- fputs(
 "  NOTE: You must specify options and their arguments on the same line.\n"
 "\n"
 "  Example, set default time out and proxy in a config file:\n"
 "\n"
 "        # We want a 30 minute timeout:\n"
 "        -m 1800\n"
 "        # ... and we use a proxy for all accesses:\n"
+, stdout);
+ fputs(
 "        proxy = proxy.our.domain.com:8080\n"
 "\n"
 "  White spaces ARE significant at the end of lines, but all white spaces\n"
 "  leading up to the first characters of each line are ignored.\n"
 "\n"
 "  Prevent curl from reading the default file by using -q as the first command\n"
-, stdout);
- fputs(
 "  line parameter, like:\n"
 "\n"
 "        curl -q www.thatsite.com\n"
 "\n"
 "  Force curl to get and display a local help page in case it is invoked\n"
 "  without URL by making a config file similar to:\n"
 "\n"
 "        # default url to get\n"
+, stdout);
+ fputs(
 "        url = \"http://help.with.curl.com/curlhelp.html\"\n"
 "\n"
 "  You can specify another config file to be read by using the -K/--config\n"
 "  flag. If you set config file name to \"-\" it'll read the config from stdin,\n"
 "  which can be handy if you want to hide options from being visible in process\n"
-, stdout);
- fputs(
 "  tables etc:\n"
 "\n"
 "        echo \"user = user:passwd\" | curl -K - http://that.secret.site.com\n"
 "\n"
 "EXTRA HEADERS\n"
 "\n"
 "  When using curl in your own very special programs, you may end up needing\n"
+, stdout);
+ fputs(
 "  to pass on your own custom headers when getting a web page. You can do\n"
 "  this by using the -H flag.\n"
 "\n"
 "  Example, send the header \"X-you-and-me: yes\" to the server when getting a\n"
 "  page:\n"
 "\n"
 "        curl -H \"X-you-and-me: yes\" www.love.com\n"
 "\n"
 "  This can also be useful in case you want curl to send a different text in a\n"
-, stdout);
- fputs(
 "  header than it normally does. The -H header you specify then replaces the\n"
 "  header curl would normally send. If you replace an internal header with an\n"
+, stdout);
+ fputs(
 "  empty one, you prevent that header from being sent. To prevent the Host:\n"
 "  header from being used:\n"
 "\n"
 "        curl -H \"Host:\" www.server.com\n"
 "\n"
 "FTP and PATH NAMES\n"
 "\n"
 "  Do note that when getting files with the ftp:// URL, the given path is\n"
 "  relative the directory you enter. To get the file 'README' from your home\n"
 "  directory at your ftp site, do:\n"
 "\n"
-, stdout);
- fputs(
 "        curl ftp://user:passwd@my.site.com/README\n"
 "\n"
 "  But if you want the README file from the root directory of that very same\n"
+, stdout);
+ fputs(
 "  site, you need to specify the absolute file name:\n"
 "\n"
 "        curl ftp://user:passwd@my.site.com//README\n"
 "\n"
 "  (I.e with an extra slash in front of the file name.)\n"
 "\n"
+"SFTP and SCP and PATH NAMES\n"
+"\n"
+"  With sftp: and scp: URLs, the path name given is the absolute name on the\n"
+"  server. To access a file relative to the remote user's home directory,\n"
+"  prefix the file with /~/ , such as:\n"
+"\n"
+"        curl -u $USER sftp://home.example.com/~/.bashrc\n"
+"\n"
 "FTP and firewalls\n"
 "\n"
+, stdout);
+ fputs(
 "  The FTP protocol requires one of the involved parties to open a second\n"
-"  connction as soon as data is about to get transfered. There are two ways to\n"
+"  connection as soon as data is about to get transfered. There are two ways to\n"
 "  do this.\n"
 "\n"
-, stdout);
- fputs(
 "  The default way for curl is to issue the PASV command which causes the\n"
 "  server to open another port and await another connection performed by the\n"
 "  client. This is good if the client is behind a firewall that don't allow\n"
 "  incoming connections.\n"
 "\n"
 "        curl ftp.download.com\n"
 "\n"
-"  If the server for example, is behind a firewall that don't allow connections\n"
-"  on other ports than 21 (or if it just doesn't support the PASV command), the\n"
 , stdout);
  fputs(
+"  If the server for example, is behind a firewall that don't allow connections\n"
+"  on other ports than 21 (or if it just doesn't support the PASV command), the\n"
 "  other way to do it is to use the PORT command and instruct the server to\n"
 "  connect to the client on the given (as parameters to the PORT command) IP\n"
 "  number and port.\n"
 "\n"
 "  The -P flag to curl supports a few different options. Your machine may have\n"
 "  several IP-addresses and/or network interfaces and curl allows you to select\n"
+, stdout);
+ fputs(
 "  which of them to use. Default address can also be used:\n"
 "\n"
 "        curl -P - ftp.download.com\n"
 "\n"
-, stdout);
- fputs(
 "  Download with PORT but use the IP address of our 'le0' interface (this does\n"
 "  not work on windows):\n"
 "\n"
 "        curl -P le0 ftp.download.com\n"
 "\n"
 "  Download with PORT but use 192.168.0.10 as our IP address to use:\n"

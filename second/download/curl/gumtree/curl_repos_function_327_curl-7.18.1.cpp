static void help(void)
{
  int i;
  /* A few of these source lines are >80 columns wide, but that's only because
     breaking the strings narrower makes this chunk look even worse!

     Starting with 7.18.0, this list of command line options is sorted based
     on the long option name. It is not done automatically, although a command
     line like the following can help out:

     curl --help | cut -c5- | grep "^-" | sort
  */
  static const char * const helptext[]={
    "Usage: curl [options...] <url>",
    "Options: (H) means HTTP/HTTPS only, (F) means FTP only",
    "    --anyauth       Pick \"any\" authentication method (H)",
    " -a/--append        Append to target file when uploading (F)",
    "    --basic         Use HTTP Basic Authentication (H)",
    "    --cacert <file> CA certificate to verify peer against (SSL)",
    "    --capath <directory> CA directory to verify peer against (SSL)",
    " -E/--cert <cert[:passwd]> Client certificate file and password (SSL)",
    "    --cert-type <type> Certificate file type (DER/PEM/ENG) (SSL)",
    "    --ciphers <list> SSL ciphers to use (SSL)",
    "    --compressed    Request compressed response (using deflate or gzip)",
    " -K/--config        Specify which config file to read",
    "    --connect-timeout <seconds> Maximum time allowed for connection",
    " -C/--continue-at <offset> Resumed transfer offset",
    " -b/--cookie <name=string/file> Cookie string or file to read cookies from (H)",
    " -c/--cookie-jar <file> Write cookies to this file after operation (H)",
    "    --create-dirs   Create necessary local directory hierarchy",
    "    --crlf          Convert LF to CRLF in upload",
    " -d/--data <data>   HTTP POST data (H)",
    "    --data-ascii <data>  HTTP POST ASCII data (H)",
    "    --data-binary <data> HTTP POST binary data (H)",
    "    --data-urlencode <name=data/name@filename> HTTP POST data url encoded (H)",
    "    --digest        Use HTTP Digest Authentication (H)",
    "    --disable-eprt  Inhibit using EPRT or LPRT (F)",
    "    --disable-epsv  Inhibit using EPSV (F)",
    " -D/--dump-header <file> Write the headers to this file",
    "    --egd-file <file> EGD socket path for random data (SSL)",
    "    --engine <eng>  Crypto engine to use (SSL). \"--engine list\" for list",
#ifdef USE_ENVIRONMENT
    "    --environment   Write results to environment variables (RISC OS)",
#endif
    " -f/--fail          Fail silently (no output at all) on HTTP errors (H)",
    " -F/--form <name=content> Specify HTTP multipart POST data (H)",
    "    --form-string <name=string> Specify HTTP multipart POST data (H)",
    "    --ftp-account <data> Account data to send when requested by server (F)",
    "    --ftp-alternative-to-user String to replace \"USER [name]\" (F)",
    "    --ftp-create-dirs Create the remote dirs if not present (F)",
    "    --ftp-method [multicwd/nocwd/singlecwd] Control CWD usage (F)",
    "    --ftp-pasv      Use PASV/EPSV instead of PORT (F)",
    " -P/--ftp-port <address> Use PORT with address instead of PASV (F)",
    "    --ftp-skip-pasv-ip Skip the IP address for PASV (F)\n"
    "    --ftp-ssl       Try SSL/TLS for ftp transfer (F)",
    "    --ftp-ssl-ccc   Send CCC after authenticating (F)",
    "    --ftp-ssl-ccc-mode [active/passive] Set CCC mode (F)",
    "    --ftp-ssl-control Require SSL/TLS for ftp login, clear for transfer (F)",
    "    --ftp-ssl-reqd  Require SSL/TLS for ftp transfer (F)",
    " -G/--get           Send the -d data with a HTTP GET (H)",
    " -g/--globoff       Disable URL sequences and ranges using {} and []",
    " -H/--header <line> Custom header to pass to server (H)",
    " -I/--head          Show document info only",
    " -h/--help          This help text",
    "    --hostpubmd5 <md5> Hex encoded MD5 string of the host public key. (SSH)",
    " -0/--http1.0       Use HTTP 1.0 (H)",
    "    --ignore-content-length  Ignore the HTTP Content-Length header",
    " -i/--include       Include protocol headers in the output (H/F)",
    " -k/--insecure      Allow connections to SSL sites without certs (H)",
    "    --interface <interface> Specify network interface/address to use",
    " -4/--ipv4          Resolve name to IPv4 address",
    " -6/--ipv6          Resolve name to IPv6 address",
    " -j/--junk-session-cookies Ignore session cookies read from file (H)",
    "    --keepalive-time <seconds> Interval between keepalive probes",
    "    --key <key>     Private key file name (SSL/SSH)",
    "    --key-type <type> Private key file type (DER/PEM/ENG) (SSL)",
    "    --krb <level>   Enable kerberos with specified security level (F)",
    "    --libcurl <file> Dump libcurl equivalent code of this command line",
    "    --limit-rate <rate> Limit transfer speed to this rate",
    " -l/--list-only     List only names of an FTP directory (F)",
    "    --local-port <num>[-num] Force use of these local port numbers",
    " -L/--location      Follow Location: hints (H)",
    "    --location-trusted Follow Location: and send auth to other hosts (H)",
    " -M/--manual        Display the full manual",
    "    --max-filesize <bytes> Maximum file size to download (H/F)",
    "    --max-redirs <num> Maximum number of redirects allowed (H)",
    " -m/--max-time <seconds> Maximum time allowed for the transfer",
    "    --negotiate     Use HTTP Negotiate Authentication (H)",
    " -n/--netrc         Must read .netrc for user name and password",
    "    --netrc-optional Use either .netrc or URL; overrides -n",
    " -N/--no-buffer     Disable buffering of the output stream",
    "    --no-keepalive  Disable keepalive use on the connection",
    "    --no-sessionid  Disable SSL session-ID reusing (SSL)",
    "    --ntlm          Use HTTP NTLM authentication (H)",
    " -o/--output <file> Write output to <file> instead of stdout",
    "    --pass  <pass>  Pass phrase for the private key (SSL/SSH)",
    "    --post301       Do not switch to GET after following a 301 redirect (H)",
    " -#/--progress-bar  Display transfer progress as a progress bar",
    " -x/--proxy <host[:port]> Use HTTP proxy on given port",
    "    --proxy-anyauth Pick \"any\" proxy authentication method (H)",
    "    --proxy-basic   Use Basic authentication on the proxy (H)",
    "    --proxy-digest  Use Digest authentication on the proxy (H)",
    "    --proxy-negotiate Use Negotiate authentication on the proxy (H)",
    "    --proxy-ntlm    Use NTLM authentication on the proxy (H)",
    " -U/--proxy-user <user[:password]> Set proxy user and password",
    " -p/--proxytunnel   Operate through a HTTP proxy tunnel (using CONNECT)",
    "    --pubkey <key>  Public key file name (SSH)",
    " -Q/--quote <cmd>   Send command(s) to server before file transfer (F/SFTP)",
    "    --random-file <file> File for reading random data from (SSL)",
    " -r/--range <range> Retrieve a byte range from a HTTP/1.1 or FTP server",
    "    --raw           Pass HTTP \"raw\", without any transfer decoding (H)",
    " -e/--referer       Referer URL (H)",
    " -O/--remote-name   Write output to a file named as the remote file",
    " -R/--remote-time   Set the remote file's time on the local output",
    " -X/--request <command> Specify request command to use",
    "    --retry <num>   Retry request <num> times if transient problems occur",
    "    --retry-delay <seconds> When retrying, wait this many seconds between each",
    "    --retry-max-time <seconds> Retry only within this period",
    " -S/--show-error    Show error. With -s, make curl show errors when they occur",
    " -s/--silent        Silent mode. Don't output anything",
    "    --socks4 <host[:port]> SOCKS4 proxy on given host + port",
    "    --socks4a <host[:port]> SOCKS4a proxy on given host + port",
    "    --socks5 <host[:port]> SOCKS5 proxy on given host + port",
    "    --socks5-hostname <host[:port]> SOCKS5 proxy, pass host name to proxy",
    " -Y/--speed-limit   Stop transfer if below speed-limit for 'speed-time' secs",
    " -y/--speed-time    Time needed to trig speed-limit abort. Defaults to 30",
    " -2/--sslv2         Use SSLv2 (SSL)",
    " -3/--sslv3         Use SSLv3 (SSL)",
    "    --stderr <file> Where to redirect stderr. - means stdout",
    "    --tcp-nodelay   Use the TCP_NODELAY option",
    " -t/--telnet-option <OPT=val> Set telnet option",
    " -z/--time-cond <time> Transfer based on a time condition",
    " -1/--tlsv1         Use TLSv1 (SSL)",
    "    --trace <file>  Write a debug trace to the given file",
    "    --trace-ascii <file> Like --trace but without the hex output",
    "    --trace-time    Add time stamps to trace/verbose output",
    " -T/--upload-file <file> Transfer <file> to remote site",
    "    --url <URL>     Set URL to work with",
    " -B/--use-ascii     Use ASCII/text transfer",
    " -u/--user <user[:password]> Set server user and password",
    " -A/--user-agent <string> User-Agent to send to server (H)",
    " -v/--verbose       Make the operation more talkative",
    " -V/--version       Show version number and quit",

#ifdef MSDOS
    "    --wdebug        Turn on Watt-32 debugging under DJGPP",
#endif
    " -w/--write-out [format] What to output after completion",
    " -q                 If used as the first parameter disables .curlrc",
    NULL
  };
  for(i=0; helptext[i]; i++) {
    puts(helptext[i]);
#ifdef NETWARE
    if (i && ((i % 23) == 0))
      pressanykey();
#endif
  }
}
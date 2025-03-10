fputs(
"  Get a file off an FTPS server:\n"
"\n"
"        curl ftps://files.are.secure.com/secrets.txt\n"
"\n"
"  or use the more appropriate FTPS way to get the same file:\n"
"\n"
"        curl --ftp-ssl ftp://files.are.secure.com/secrets.txt\n"
"\n"
"  Get a file from an SSH server using SFTP:\n"
"\n"
"        curl -u username sftp://example.com/etc/issue\n"
"\n"
"  Get a file from an SSH server using SCP using a private key\n"
"  (not password-protected) to authenticate:\n"
"\n"
"        curl -u username: --key ~/.ssh/id_rsa \\\n"
, stdout);
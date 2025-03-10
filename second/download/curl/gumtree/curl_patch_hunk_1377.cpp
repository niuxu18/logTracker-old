 "     4    URL  user malformatted. The user-part of the URL syntax\n"
 "          was not correct.\n"
 "\n"
 "     5    Couldn't resolve proxy. The given proxy host could  not\n"
 "          be resolved.\n"
 "\n"
-);
- puts(
 "     6    Couldn't  resolve  host.  The given remote host was not\n"
 "          resolved.\n"
 "\n"
 "     7    Failed to connect to host.\n"
 "\n"
+);
+ puts(
 "     8    FTP weird server  reply.  The  server  sent  data  curl\n"
 "          couldn't parse.\n"
 "\n"
 "     9    FTP access denied. The server denied login.\n"
 "\n"
 "     10   FTP  user/password  incorrect.  Either one or both were\n"
 "          not accepted by the server.\n"
 "\n"
 "     11   FTP weird PASS reply. Curl  couldn't  parse  the  reply\n"
 "          sent to the PASS request.\n"
 "\n"
-);
- puts(
 "     12   FTP  weird  USER  reply.  Curl couldn't parse the reply\n"
 "          sent to the USER request.\n"
+);
+ puts(
 "     13   FTP weird PASV reply, Curl  couldn't  parse  the  reply\n"
 "          sent to the PASV request.\n"
 "\n"
 "     14   FTP  weird 227 format. Curl couldn't parse the 227-line\n"
 "          the server sent.\n"
 "\n"
 "     15   FTP can't get host. Couldn't resolve the host IP we got\n"
 "          in the 227-line.\n"
 "\n"
 "     16   FTP  can't  reconnect.  Couldn't connect to the host we\n"
 "          got in the 227-line.\n"
 "\n"
-);
- puts(
 "     17   FTP  couldn't  set  binary.  Couldn't  change  transfer\n"
 "          method to binary.\n"
 "\n"
+);
+ puts(
 "     18   Partial file. Only a part of the file was transfered.\n"
 "\n"
 "     19   FTP couldn't RETR file. The RETR command failed.\n"
 "\n"
 "     20   FTP  write  error. The transfer was reported bad by the\n"
 "          server.\n"
 "\n"
 "     21   FTP quote error. A quote command  returned  error  from\n"
 "          the server.\n"
 "\n"
 "     22   HTTP  not found. The requested page was not found. This\n"
-);
- puts(
 "          return code only appears if --fail is used.\n"
 "\n"
 "     23   Write error.  Curl  couldn't  write  data  to  a  local\n"
+);
+ puts(
 "          filesystem or similar.\n"
 "\n"
 "     24   Malformat user. User name badly specified.\n"
 "\n"
 "     25   FTP  couldn't  STOR  file.  The  server denied the STOR\n"
 "          operation.\n"
 "\n"
 "     26   Read error. Various reading problems.\n"
 "\n"
 "     27   Out of memory. A memory allocation request failed.\n"
 "\n"
 "     28   Operation timeout. The specified  time-out  period  was\n"
-);
- puts(
 "          reached according to the conditions.\n"
 "\n"
 "     29   FTP  couldn't set ASCII. The server returned an unknown\n"
 "          reply.\n"
 "\n"
+);
+ puts(
 "     30   FTP PORT failed. The PORT command failed.\n"
 "\n"
 "     31   FTP couldn't use REST. The REST command failed.\n"
 "\n"
 "     32   FTP couldn't use SIZE. The  SIZE  command  failed.  The\n"
 "          command  is  an  extension to the original FTP spec RFC\n"
 "          959.\n"
 "\n"
 "     33   HTTP range error. The range \"command\" didn't work.\n"
 "\n"
-);
- puts(
 "     34   HTTP  post  error.  Internal  post-request   generation\n"
 "          error.\n"
 "\n"
 "     35   SSL connect error. The SSL handshaking failed.\n"
 "\n"
+);
+ puts(
 "     36   FTP  bad  download resume. Couldn't continue an earlier\n"
 "          aborted download.\n"
 "\n"
 "     37   FILE couldn't read file. Failed to open the file.  Per�\n"
 "          missions?\n"
 "\n"
 "     38   LDAP cannot bind. LDAP bind operation failed.\n"
 "\n"
 "     39   LDAP search failed.\n"
 "\n"
 "     40   Library not found. The LDAP library was not found.\n"
 "\n"
-);
- puts(
 "     41   Function  not  found.  A required LDAP function was not\n"
 "          found.\n"
 "\n"
 "     42   Aborted by callback. An application told curl to  abort\n"
+);
+ puts(
 "          the operation.\n"
 "\n"
 "     43   Internal error. A function was called with a bad param�\n"
 "          eter.\n"
 "\n"
 "     44   Internal error. A function was called in a bad order.\n"
 "\n"
 "     45   Interface error. A specified outgoing  interface  could\n"
 "          not be used.\n"
 "\n"
 "     46   Bad  password  entered. An error was signalled when the\n"
-);
- puts(
 "          password was entered.\n"
 "\n"
 "     47   Too many redirects. When following redirects, curl  hit\n"
 "          the maximum amount.\n"
 "\n"
+);
+ puts(
 "     XX   There  will  appear  more  error  codes  here in future\n"
 "          releases. The existing ones are meant to never  change.\n"
 "\n"
 "BUGS\n"
 "     If you do find bugs, mail them to curl-bug@haxx.se.\n"
 "\n"
 "AUTHORS / CONTRIBUTORS\n"
 "      - Daniel Stenberg <Daniel.Stenberg@haxx.se>\n"
 "      - Rafael Sagula <sagula@inf.ufrgs.br>\n"
 "      - Sampo Kellomaki <sampo@iki.fi>\n"
-);
- puts(
 "      - Linas Vepstas <linas@linas.org>\n"
 "      - Bjorn Reese <breese@mail1.stofanet.dk>\n"
 "      - Johan Anderson <johan@homemail.com>\n"
+);
+ puts(
 "      - Kjell Ericson <Kjell.Ericson@haxx.se>\n"
 "      - Troy Engel <tengel@sonic.net>\n"
 "      - Ryan Nelson <ryan@inch.com>\n"
 "      - Bj�rn Stenberg <Bjorn.Stenberg@haxx.se>\n"
 "      - Angus Mackay <amackay@gus.ml.org>\n"
 "      - Eric Young <eay@cryptsoft.com>\n"
 "      - Simon Dick <simond@totally.irrelevant.org>\n"
 "      - Oren Tirosh <oren@monty.hishome.net>\n"
-);
- puts(
 "      - Steven G. Johnson <stevenj@alum.mit.edu>\n"
 "      - Gilbert Ramirez Jr. <gram@verdict.uthscsa.edu>\n"
 "      - Andr�s Garc�a <ornalux@redestb.es>\n"
+);
+ puts(
 "      - Douglas E. Wegscheid <wegscd@whirlpool.com>\n"
 "      - Mark Butler <butlerm@xmission.com>\n"
 "      - Eric Thelin <eric@generation-i.com>\n"
 "      - Marc Boucher <marc@mbsi.ca>\n"
 "      - Greg Onufer <Greg.Onufer@Eng.Sun.COM>\n"
 "      - Doug Kaufman <dkaufman@rahul.net>\n"
 "      - David Eriksson <david@2good.com>\n"
 "      - Ralph Beckmann <rabe@uni-paderborn.de>\n"
-);
- puts(
 "      - T. Yamada <tai@imasy.or.jp>\n"
 "      - Lars J. Aas <larsa@sim.no>\n"
 "      - J�rn Hartroth <Joern.Hartroth@computer.org>\n"
+);
+ puts(
 "      - Matthew Clarke <clamat@van.maves.ca>\n"
 "      - Linus Nielsen Feltzing <linus@haxx.se>\n"
 "      - Felix von Leitner <felix@convergence.de>\n"
 "      - Dan Zitter <dzitter@zitter.net>\n"
 "      - Jongki Suwandi <Jongki.Suwandi@eng.sun.com>\n"
 "      - Chris Maltby <chris@aurema.com>\n"
 "      - Ron Zapp <rzapper@yahoo.com>\n"
 "      - Paul Marquis <pmarquis@iname.com>\n"
-);
- puts(
 "      - Ellis Pritchard <ellis@citria.com>\n"
 "      - Damien Adant <dams@usa.net>\n"
 "      - Chris <cbayliss@csc.come>\n"
+);
+ puts(
 "      - Marco G. Salvagno <mgs@whiz.cjb.net>\n"
 "      - Paul Marquis <pmarquis@iname.com>\n"
 "      - David LeBlanc <dleblanc@qnx.com>\n"
 "      - Rich Gray at Plus Technologies\n"
 "      - Luong Dinh Dung <u8luong@lhsystems.hu>\n"
 "      - Torsten Foertsch <torsten.foertsch@gmx.net>\n"
 "      - Kristian K�hntopp <kris@koehntopp.de>\n"
 "      - Fred Noz <FNoz@siac.com>\n"
-);
- puts(
 "      - Caolan McNamara <caolan@csn.ul.ie>\n"
 "      - Albert Chin-A-Young <china@thewrittenword.com>\n"
 "      - Stephen Kick <skick@epicrealm.com>\n"
+);
+ puts(
 "      - Martin Hedenfalk <mhe@stacken.kth.se>\n"
 "      - Richard Prescott\n"
 "      - Jason S. Priebe <priebe@wral-tv.com>\n"
 "      - T. Bharath <TBharath@responsenetworks.com>\n"
 "      - Alexander Kourakos <awk@users.sourceforge.net>\n"
 "      - James Griffiths <griffiths_james@yahoo.com>\n"
 "      - Loic Dachary <loic@senga.org>\n"
-);
- puts(
 "      - Robert Weaver <robert.weaver@sabre.com>\n"
 "      - Ingo Ralf Blum <ingoralfblum@ingoralfblum.com>\n"
 "      - Jun-ichiro itojun Hagino <itojun@iijlab.net>\n"
+);
+ puts(
+"      - Frederic Lepied <flepied@mandrakesoft.com>\n"
+"      - Georg Horn <horn@koblenz-net.de>\n"
+"      - Cris Bailiff <c.bailiff@awayweb.com>\n"
 "\n"
 "WWW\n"
 "     http://curl.haxx.se\n"
 "\n"
 "FTP\n"
 "     ftp://ftp.sunet.se/pub/www/utilities/curl/\n"

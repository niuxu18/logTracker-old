  if(!p) {
    Curl_failf(conn->data, "Bad reply from server");
    Curl_set_command_prot(conn, save);
    return CURLE_FTP_WEIRD_SERVER_REPLY;
  }

  p += 2;
  tmp = Curl_base64_decode(p, &tkt.dat);
  if(tmp < 0) {
    Curl_failf(conn->data, "Failed to decode base64 in reply.\n");
    Curl_set_command_prot(conn, save);
    return CURLE_FTP_WEIRD_SERVER_REPLY;
  }
  tkt.length = tmp;
  tktcopy.length = tkt.length;

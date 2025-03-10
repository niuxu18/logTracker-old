static
CURLcode _ftp_getfiletime(struct connectdata *conn, char *file)
{
  CURLcode result=CURLE_OK;
  int ftpcode; /* for ftp status */
  ssize_t nread;
  char *buf = conn->data->state.buffer;

  /* we have requested to get the modified-time of the file, this is yet
     again a grey area as the MDTM is not kosher RFC959 */
  ftpsendf(conn->firstsocket, conn, "MDTM %s", file);

  nread = Curl_GetFTPResponse(conn->firstsocket, buf, conn, &ftpcode);
  if(nread < 0)
    return CURLE_OPERATION_TIMEOUTED;

  if(ftpcode == 213) {
    /* we got a time. Format should be: "YYYYMMDDHHMMSS[.sss]" where the
       last .sss part is optional and means fractions of a second */
    int year, month, day, hour, minute, second;
    if(6 == sscanf(buf+4, "%04d%02d%02d%02d%02d%02d",
                   &year, &month, &day, &hour, &minute, &second)) {
      /* we have a time, reformat it */
      time_t secs=time(NULL);
      sprintf(buf, "%04d%02d%02d %02d:%02d:%02d",
              year, month, day, hour, minute, second);
      /* now, convert this into a time() value: */
      conn->data->info.filetime = curl_getdate(buf, &secs);
    }
    else {
      infof(conn->data, "unsupported MDTM reply format\n");
    }
  }
  return  result;
}
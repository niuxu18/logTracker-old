static int sock_cb(CURL *e, curl_socket_t s, int what, void *cbp, void *sockp)
{
  GlobalInfo *g = (GlobalInfo*) cbp;
  SockInfo *fdp = (SockInfo*) sockp;
  const char *whatstr[]={ "none", "IN", "OUT", "INOUT", "REMOVE" };

  fprintf(MSG_OUT,
          "socket callback: s=%d e=%p what=%s ", s, e, whatstr[what]);
  if (what == CURL_POLL_REMOVE) {
    fprintf(MSG_OUT, "\n");
    remsock(fdp);
  } else {
    if (!fdp) {
      fprintf(MSG_OUT, "Adding data: %s%s\n",
             what&CURL_POLL_IN?"READ":"",
             what&CURL_POLL_OUT?"WRITE":"" );
      addsock(s, e, what, g);
    }
    else {
      fprintf(MSG_OUT,
        "Changing action from %d to %d\n", fdp->action, what);
      setsock(fdp, s, e, what, g);
    }
  }
  return 0;
}
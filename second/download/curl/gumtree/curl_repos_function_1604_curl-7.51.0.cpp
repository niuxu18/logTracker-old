static void state(struct connectdata *conn, pop3state newstate)
{
  struct pop3_conn *pop3c = &conn->proto.pop3c;
#if defined(DEBUGBUILD) && !defined(CURL_DISABLE_VERBOSE_STRINGS)
  /* for debug purposes */
  static const char * const names[] = {
    "STOP",
    "SERVERGREET",
    "CAPA",
    "STARTTLS",
    "UPGRADETLS",
    "AUTH",
    "APOP",
    "USER",
    "PASS",
    "COMMAND",
    "QUIT",
    /* LAST */
  };

  if(pop3c->state != newstate)
    infof(conn->data, "POP3 %p state change from %s to %s\n",
          (void *)pop3c, names[pop3c->state], names[newstate]);
#endif

  pop3c->state = newstate;
}
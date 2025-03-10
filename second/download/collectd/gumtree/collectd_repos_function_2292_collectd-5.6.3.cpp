static int lcc_sendreceive(lcc_connection_t *c, /* {{{ */
                           const char *command, lcc_response_t *ret_res) {
  lcc_response_t res = {0};
  int status;

  if (c->fh == NULL) {
    lcc_set_errno(c, EBADF);
    return (-1);
  }

  status = lcc_send(c, command);
  if (status != 0)
    return (status);

  status = lcc_receive(c, &res);
  if (status == 0)
    memcpy(ret_res, &res, sizeof(*ret_res));

  return (status);
}
void
Ident::ReadReply(const Comm::ConnectionPointer &conn, char *buf, size_t len, comm_err_t flag, int xerrno, void *data)
{
    IdentStateData *state = (IdentStateData *)data;
    char *ident = NULL;
    char *t = NULL;

    assert(buf == state->buf);
    assert(conn->fd == state->conn->fd);

    if (flag != COMM_OK || len <= 0) {
        state->conn->close();
        return;
    }

    /*
     * XXX This isn't really very tolerant. It should read until EOL
     * or EOF and then decode the answer... If the reply is fragmented
     * then this will fail
     */
    buf[len] = '\0';

    if ((t = strchr(buf, '\r')))
        *t = '\0';

    if ((t = strchr(buf, '\n')))
        *t = '\0';

    debugs(30, 5, HERE << conn << ": Read '" << buf << "'");

    if (strstr(buf, "USERID")) {
        if ((ident = strrchr(buf, ':'))) {
            while (xisspace(*++ident));
            Ident::identCallback(state, ident);
        }
    }

    state->conn->close();
}
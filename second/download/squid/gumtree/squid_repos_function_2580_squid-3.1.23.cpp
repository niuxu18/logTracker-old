static void
ftpSendQuit(FtpStateData * ftpState)
{
    /* check the server control channel is still available */
    if (!ftpState || !ftpState->haveControlChannel("ftpSendQuit"))
        return;

    snprintf(cbuf, 1024, "QUIT\r\n");
    ftpState->writeCommand(cbuf);
    ftpState->state = SENT_QUIT;
}
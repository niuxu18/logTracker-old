static void
ftpSendReply(FtpStateData * ftpState)
{
    int code = ftpState->ctrl.replycode;
    http_status http_code;
    err_type err_code = ERR_NONE;

    debugs(9, 3, HERE << ftpState->entry->url() << ", code " << code);

    if (cbdataReferenceValid(ftpState))
        debugs(9, 5, HERE << "ftpState (" << ftpState << ") is valid!");

    if (code == 226 || code == 250) {
        err_code = (ftpState->mdtm > 0) ? ERR_FTP_PUT_MODIFIED : ERR_FTP_PUT_CREATED;
        http_code = (ftpState->mdtm > 0) ? HTTP_ACCEPTED : HTTP_CREATED;
    } else if (code == 227) {
        err_code = ERR_FTP_PUT_CREATED;
        http_code = HTTP_CREATED;
    } else {
        err_code = ERR_FTP_PUT_ERROR;
        http_code = HTTP_INTERNAL_SERVER_ERROR;
    }

    ErrorState err(err_code, http_code, ftpState->request);

    if (ftpState->old_request)
        err.ftp.request = xstrdup(ftpState->old_request);
    else
        err.ftp.request = xstrdup(ftpState->ctrl.last_command);

    if (ftpState->old_reply)
        err.ftp.reply = xstrdup(ftpState->old_reply);
    else if (ftpState->ctrl.last_reply)
        err.ftp.reply = xstrdup(ftpState->ctrl.last_reply);
    else
        err.ftp.reply = xstrdup("");

    // TODO: interpret as FTP-specific error code
    err.detailError(code);

    ftpState->entry->replaceHttpReply( err.BuildHttpReply() );

    ftpSendQuit(ftpState);
}
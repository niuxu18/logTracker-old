void
Comm::ConnOpener::doConnect()
{
    Must(conn_ != NULL);
    Must(temporaryFd_ >= 0);

    ++ totalTries_;

    switch (comm_connect_addr(temporaryFd_, conn_->remote) ) {

    case Comm::INPROGRESS:
        debugs(5, 5, HERE << conn_ << ": Comm::INPROGRESS");
        Comm::SetSelect(temporaryFd_, COMM_SELECT_WRITE, Comm::ConnOpener::InProgressConnectRetry, new Pointer(this), 0);
        break;

    case Comm::OK:
        debugs(5, 5, HERE << conn_ << ": Comm::OK - connected");
        connected();
        break;

    default: {
        const int xerrno = errno;

        ++failRetries_;
        debugs(5, 7, conn_ << ": failure #" << failRetries_ << " <= " <<
               Config.connect_retries << ": " << xstrerr(xerrno));

        if (failRetries_ < Config.connect_retries) {
            debugs(5, 5, HERE << conn_ << ": * - try again");
            retrySleep();
            return;
        } else {
            // send ERROR back to the upper layer.
            debugs(5, 5, HERE << conn_ << ": * - ERR tried too many times already.");
            sendAnswer(Comm::ERR_CONNECT, xerrno, "Comm::ConnOpener::doConnect");
        }
    }
    }
}
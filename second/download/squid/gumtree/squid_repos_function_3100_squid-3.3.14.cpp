bool
CommIoCbParams::syncWithComm()
{
    // change parameters if the call was scheduled before comm_close but
    // is being fired after comm_close
    if ((conn->fd < 0 || fd_table[conn->fd].closing()) && flag != COMM_ERR_CLOSING) {
        debugs(5, 3, HERE << "converting late call to COMM_ERR_CLOSING: " << conn);
        flag = COMM_ERR_CLOSING;
    }
    return true; // now we are in sync and can handle the call
}
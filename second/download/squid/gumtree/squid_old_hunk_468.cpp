    if (getsockname(theWccpConnection, (struct sockaddr*)&local, &slen))
        fatal("Unable to getsockname on WCCP out socket");

    local_ip = local;
}


void
wccpConnectionClose(void)
{
    if (theWccpConnection > -1) {
        debugs(80, 1, "FD " << theWccpConnection << " Closing WCCPv1 socket");
        comm_close(theWccpConnection);
        theWccpConnection = -1;
    }
}

/*

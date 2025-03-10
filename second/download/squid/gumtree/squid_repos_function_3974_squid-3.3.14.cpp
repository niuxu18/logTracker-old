void
Adaptation::Ecap::XactionRep::start()
{
    Must(theMaster);

    if (!theVirginRep.raw().body_pipe)
        makingVb = opNever; // there is nothing to deliver

    const HttpRequest *request = dynamic_cast<const HttpRequest*> (theCauseRep ?
                                 theCauseRep->raw().header : theVirginRep.raw().header);
    Must(request);
    Adaptation::History::Pointer ah = request->adaptLogHistory();
    if (ah != NULL) {
        // retrying=false because ecap never retries transactions
        adaptHistoryId = ah->recordXactStart(service().cfg().key, current_time, false);
    }

    theMaster->start();
}
HttpReply *
ServerStateData::setFinalReply(HttpReply *rep)
{
    debugs(11,5, HERE << this << " setting final reply to " << rep);

    assert(!theFinalReply);
    assert(rep);
    theFinalReply = rep;
    HTTPMSGLOCK(theFinalReply);

    // give entry the reply because haveParsedReplyHeaders() expects it there
    entry->replaceHttpReply(theFinalReply, false); // but do not write yet
    haveParsedReplyHeaders(); // update the entry/reply (e.g., set timestamps)
    entry->startWriting(); // write the updated entry to store

    return theFinalReply;
}
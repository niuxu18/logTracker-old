void
ConnStateData::httpsPeeked(PinnedIdleContext pic)
{
    Must(sslServerBump != NULL);
    Must(sslServerBump->request == pic.request);
    Must(pipeline.empty() || pipeline.front()->http == nullptr || pipeline.front()->http->request == pic.request.getRaw());

    if (Comm::IsConnOpen(pic.connection)) {
        notePinnedConnectionBecameIdle(pic);
        debugs(33, 5, HERE << "bumped HTTPS server: " << sslConnectHostOrIp);
    } else
        debugs(33, 5, HERE << "Error while bumping: " << sslConnectHostOrIp);

    getSslContextStart();
}
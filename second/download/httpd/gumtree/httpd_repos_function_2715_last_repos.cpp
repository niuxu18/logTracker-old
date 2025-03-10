static void ev_ngh2_done(h2_proxy_session *session, int arg, const char *msg)
{
    switch (session->state) {
        case H2_PROXYS_ST_DONE:
            /* nop */
            break;
        default:
            transit(session, "nghttp2 done", H2_PROXYS_ST_DONE);
            break;
    }
}
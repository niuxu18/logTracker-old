void
HttpMsg::putCc(const HttpHdrCc *otherCc)
{
    // get rid of the old CC, if any
    if (cache_control) {
        delete cache_control;
        cache_control = nullptr;
        if (!otherCc)
            header.delById(Http::HdrType::CACHE_CONTROL);
        // else it will be deleted inside putCc() below
    }

    // add new CC, if any
    if (otherCc) {
        cache_control = new HttpHdrCc(*otherCc);
        header.putCc(cache_control);
    }
}
int64_t
ClientSocketContext::getNextRangeOffset() const
{
    if (http->request->range) {
        /* offset in range specs does not count the prefix of an http msg */
        debugs (33, 5, "ClientSocketContext::getNextRangeOffset: http offset " << http->out.offset);
        /* check: reply was parsed and range iterator was initialized */
        assert(http->range_iter.valid);
        /* filter out data according to range specs */
        assert (canPackMoreRanges());
        {
            int64_t start;		/* offset of still missing data */
            assert(http->range_iter.currentSpec());
            start = http->range_iter.currentSpec()->offset + http->range_iter.currentSpec()->length - http->range_iter.debt();
            debugs(33, 3, "clientPackMoreRanges: in:  offset: " << http->out.offset);
            debugs(33, 3, "clientPackMoreRanges: out:"
                   " start: " << start <<
                   " spec[" << http->range_iter.pos - http->request->range->begin() << "]:" <<
                   " [" << http->range_iter.currentSpec()->offset <<
                   ", " << http->range_iter.currentSpec()->offset + http->range_iter.currentSpec()->length << "),"
                   " len: " << http->range_iter.currentSpec()->length <<
                   " debt: " << http->range_iter.debt());
            if (http->range_iter.currentSpec()->length != -1)
                assert(http->out.offset <= start);	/* we did not miss it */

            return start;
        }

    } else if (reply && reply->content_range) {
        /* request does not have ranges, but reply does */
        /** \todo FIXME: should use range_iter_pos on reply, as soon as reply->content_range
         *        becomes HttpHdrRange rather than HttpHdrRangeSpec.
         */
        return http->out.offset + reply->content_range->spec.offset;
    }

    return http->out.offset;
}
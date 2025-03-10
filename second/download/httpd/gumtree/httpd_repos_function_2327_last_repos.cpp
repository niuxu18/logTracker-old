apr_status_t h2_brigade_concat_length(apr_bucket_brigade *dest, 
                                      apr_bucket_brigade *src,
                                      apr_off_t length)
{
    apr_bucket *b;
    apr_off_t remain = length;
    apr_status_t status = APR_SUCCESS;
    
    while (!APR_BRIGADE_EMPTY(src)) {
        b = APR_BRIGADE_FIRST(src); 
        
        if (APR_BUCKET_IS_METADATA(b)) {
            APR_BUCKET_REMOVE(b);
            APR_BRIGADE_INSERT_TAIL(dest, b);
        }
        else {
            if (remain == b->length) {
                /* fall through */
            }
            else if (remain <= 0) {
                return status;
            }
            else {
                if (b->length == ((apr_size_t)-1)) {
                    const char *ign;
                    apr_size_t ilen;
                    status = apr_bucket_read(b, &ign, &ilen, APR_BLOCK_READ);
                    if (status != APR_SUCCESS) {
                        return status;
                    }
                }
            
                if (remain < b->length) {
                    apr_bucket_split(b, remain);
                }
            }
            APR_BUCKET_REMOVE(b);
            APR_BRIGADE_INSERT_TAIL(dest, b);
            remain -= b->length;
        }
    }
    return status;
}
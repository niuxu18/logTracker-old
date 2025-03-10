                         tmp_heap = apr_bucket_heap_create((char *)ctx->buffer, len,
                                                           NULL, f->c->bucket_alloc);
                         APR_BRIGADE_INSERT_TAIL(ctx->proc_bb, tmp_heap);
                         ctx->stream.avail_out = c->bufferSize;
                     }
 
-                    len = ctx->stream.avail_out;
+                    ctx->inflate_total += ctx->stream.avail_out;
                     zRC = inflate(&ctx->stream, Z_NO_FLUSH);
-
+                    ctx->inflate_total -= ctx->stream.avail_out;
                     if (zRC != Z_OK && zRC != Z_STREAM_END) {
                         inflateEnd(&ctx->stream);
                         ap_log_rerror(APLOG_MARK, APLOG_WARNING, 0, r, APLOGNO(01392)
                                       "Zlib error %d inflating data (%s)", zRC,
                                       ctx->stream.msg);
                         return APR_EGENERAL;
                     }
 
-                    ctx->inflate_total += len - ctx->stream.avail_out;
                     if (inflate_limit && ctx->inflate_total > inflate_limit) { 
                         inflateEnd(&ctx->stream);
                         ap_log_rerror(APLOG_MARK, APLOG_WARNING, 0, r, APLOGNO(02648)
                                 "Inflated content length of %" APR_OFF_T_FMT
                                 " is larger than the configured limit"
                                 " of %" APR_OFF_T_FMT, 

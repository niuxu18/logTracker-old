  * limitations under the License.
  */
 
 #include <assert.h>
 #include <stddef.h>
 
-#define APR_POOL_DEBUG  7
-
 #include <httpd.h>
 #include <http_core.h>
 #include <http_connection.h>
 #include <http_log.h>
 
 #include <nghttp2/nghttp2.h>
 
 #include "h2_private.h"
 #include "h2_conn.h"
+#include "h2_config.h"
+#include "h2_h2.h"
 #include "h2_mplx.h"
+#include "h2_push.h"
 #include "h2_request.h"
 #include "h2_response.h"
+#include "h2_session.h"
 #include "h2_stream.h"
 #include "h2_task.h"
 #include "h2_ctx.h"
 #include "h2_task_input.h"
 #include "h2_task.h"
 #include "h2_util.h"
 
 
-static void set_state(h2_stream *stream, h2_stream_state_t state)
+#define H2_STREAM_OUT(lvl,s,msg) \
+    do { \
+        if (APLOG_C_IS_LEVEL((s)->session->c,lvl)) \
+        h2_util_bb_log((s)->session->c,(s)->id,lvl,msg,(s)->bbout); \
+    } while(0)
+#define H2_STREAM_IN(lvl,s,msg) \
+    do { \
+        if (APLOG_C_IS_LEVEL((s)->session->c,lvl)) \
+        h2_util_bb_log((s)->session->c,(s)->id,lvl,msg,(s)->bbin); \
+    } while(0)
+    
+
+static int state_transition[][7] = {
+    /*  ID OP RL RR CI CO CL */
+/*ID*/{  1, 0, 0, 0, 0, 0, 0 },
+/*OP*/{  1, 1, 0, 0, 0, 0, 0 },
+/*RL*/{  0, 0, 1, 0, 0, 0, 0 },
+/*RR*/{  0, 0, 0, 1, 0, 0, 0 },
+/*CI*/{  1, 1, 0, 0, 1, 0, 0 },
+/*CO*/{  1, 1, 0, 0, 0, 1, 0 },
+/*CL*/{  1, 1, 0, 0, 1, 1, 1 },
+};
+
+static int set_state(h2_stream *stream, h2_stream_state_t state)
 {
-    AP_DEBUG_ASSERT(stream);
-    if (stream->state != state) {
+    int allowed = state_transition[state][stream->state];
+    if (allowed) {
         stream->state = state;
+        return 1;
     }
+    
+    ap_log_cerror(APLOG_MARK, APLOG_WARNING, 0, stream->session->c,
+                  "h2_stream(%ld-%d): invalid state transition from %d to %d", 
+                  stream->session->id, stream->id, stream->state, state);
+    return 0;
 }
 
-h2_stream *h2_stream_create(int id, apr_pool_t *pool, struct h2_mplx *m)
+static int close_input(h2_stream *stream) 
 {
-    h2_stream *stream = apr_pcalloc(pool, sizeof(h2_stream));
-    if (stream != NULL) {
-        stream->id = id;
-        stream->state = H2_STREAM_ST_IDLE;
-        stream->pool = pool;
-        stream->m = m;
-        stream->request = h2_request_create(id, pool, m->c->bucket_alloc);
-        ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, m->c,
-                      "h2_stream(%ld-%d): created", m->id, stream->id);
+    switch (stream->state) {
+        case H2_STREAM_ST_CLOSED_INPUT:
+        case H2_STREAM_ST_CLOSED:
+            return 0; /* ignore, idempotent */
+        case H2_STREAM_ST_CLOSED_OUTPUT:
+            /* both closed now */
+            set_state(stream, H2_STREAM_ST_CLOSED);
+            break;
+        default:
+            /* everything else we jump to here */
+            set_state(stream, H2_STREAM_ST_CLOSED_INPUT);
+            break;
     }
-    return stream;
+    return 1;
 }
 
-static void h2_stream_cleanup(h2_stream *stream)
+static int input_closed(h2_stream *stream) 
 {
-    if (stream->request) {
-        h2_request_destroy(stream->request);
-        stream->request = NULL;
+    switch (stream->state) {
+        case H2_STREAM_ST_OPEN:
+        case H2_STREAM_ST_CLOSED_OUTPUT:
+            return 0;
+        default:
+            return 1;
+    }
+}
+
+static int close_output(h2_stream *stream) 
+{
+    switch (stream->state) {
+        case H2_STREAM_ST_CLOSED_OUTPUT:
+        case H2_STREAM_ST_CLOSED:
+            return 0; /* ignore, idempotent */
+        case H2_STREAM_ST_CLOSED_INPUT:
+            /* both closed now */
+            set_state(stream, H2_STREAM_ST_CLOSED);
+            break;
+        default:
+            /* everything else we jump to here */
+            set_state(stream, H2_STREAM_ST_CLOSED_OUTPUT);
+            break;
+    }
+    return 1;
+}
+
+static int input_open(h2_stream *stream) 
+{
+    switch (stream->state) {
+        case H2_STREAM_ST_OPEN:
+        case H2_STREAM_ST_CLOSED_OUTPUT:
+            return 1;
+        default:
+            return 0;
+    }
+}
+
+static int output_open(h2_stream *stream) 
+{
+    switch (stream->state) {
+        case H2_STREAM_ST_OPEN:
+        case H2_STREAM_ST_CLOSED_INPUT:
+            return 1;
+        default:
+            return 0;
     }
 }
 
+h2_stream *h2_stream_create(int id, apr_pool_t *pool, h2_session *session)
+{
+    h2_stream *stream = apr_pcalloc(pool, sizeof(h2_stream));
+    stream->id        = id;
+    stream->state     = H2_STREAM_ST_IDLE;
+    stream->pool      = pool;
+    stream->session   = session;
+    return stream;
+}
+
+h2_stream *h2_stream_open(int id, apr_pool_t *pool, h2_session *session)
+{
+    h2_stream *stream = h2_stream_create(id, pool, session);
+    set_state(stream, H2_STREAM_ST_OPEN);
+    stream->request   = h2_request_create(id, pool, session->config);
+    stream->bbout     = apr_brigade_create(stream->pool, 
+                                           stream->session->c->bucket_alloc);
+    
+    ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, session->c,
+                  "h2_stream(%ld-%d): opened", session->id, stream->id);
+    return stream;
+}
+
 apr_status_t h2_stream_destroy(h2_stream *stream)
 {
     AP_DEBUG_ASSERT(stream);
-    ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, stream->m->c,
-                  "h2_stream(%ld-%d): destroy", stream->m->id, stream->id);
-    h2_stream_cleanup(stream);
-    
-    if (stream->task) {
-        h2_task_destroy(stream->task);
-        stream->task = NULL;
+    if (stream->request) {
+        h2_request_destroy(stream->request);
+        stream->request = NULL;
     }
+    
     if (stream->pool) {
         apr_pool_destroy(stream->pool);
     }
     return APR_SUCCESS;
 }
 
-void h2_stream_attach_pool(h2_stream *stream, apr_pool_t *pool)
+void h2_stream_cleanup(h2_stream *stream)
 {
-    stream->pool = pool;
+    h2_session_stream_destroy(stream->session, stream);
+    /* stream is gone */
 }
 
 apr_pool_t *h2_stream_detach_pool(h2_stream *stream)
 {
     apr_pool_t *pool = stream->pool;
     stream->pool = NULL;
     return pool;
 }
 
-void h2_stream_abort(h2_stream *stream)
+void h2_stream_rst(h2_stream *stream, int error_code)
 {
-    AP_DEBUG_ASSERT(stream);
-    stream->aborted = 1;
+    stream->rst_error = error_code;
+    close_input(stream);
+    close_output(stream);
+    ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, stream->session->c,
+                  "h2_stream(%ld-%d): reset, error=%d", 
+                  stream->session->id, stream->id, error_code);
 }
 
 apr_status_t h2_stream_set_response(h2_stream *stream, h2_response *response,
                                     apr_bucket_brigade *bb)
 {
+    apr_status_t status = APR_SUCCESS;
+    if (!output_open(stream)) {
+        ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, stream->session->c,
+                      "h2_stream(%ld-%d): output closed", 
+                      stream->session->id, stream->id);
+        return APR_ECONNRESET;
+    }
+    
     stream->response = response;
     if (bb && !APR_BRIGADE_EMPTY(bb)) {
-        if (!stream->bbout) {
-            stream->bbout = apr_brigade_create(stream->pool, 
-                                               stream->m->c->bucket_alloc);
-        }
-        return h2_util_move(stream->bbout, bb, 16 * 1024, NULL,  
-                            "h2_stream_set_response");
+        int move_all = INT_MAX;
+        /* we can move file handles from h2_mplx into this h2_stream as many
+         * as we want, since the lifetimes are the same and we are not freeing
+         * the ones in h2_mplx->io before this stream is done. */
+        H2_STREAM_OUT(APLOG_TRACE2, stream, "h2_stream set_response_pre");
+        status = h2_util_move(stream->bbout, bb, 16 * 1024, &move_all,  
+                              "h2_stream_set_response");
+        H2_STREAM_OUT(APLOG_TRACE2, stream, "h2_stream set_response_post");
     }
-    return APR_SUCCESS;
+    
+    ap_log_cerror(APLOG_MARK, APLOG_DEBUG, status, stream->session->c,
+                  "h2_stream(%ld-%d): set_response(%d)", 
+                  stream->session->id, stream->id, response->http_status);
+    return status;
 }
 
-static int set_closed(h2_stream *stream) 
+apr_status_t h2_stream_set_request(h2_stream *stream, request_rec *r)
 {
-    switch (stream->state) {
-        case H2_STREAM_ST_CLOSED_INPUT:
-        case H2_STREAM_ST_CLOSED:
-            return 0; /* ignore, idempotent */
-        case H2_STREAM_ST_CLOSED_OUTPUT:
-            /* both closed now */
-            set_state(stream, H2_STREAM_ST_CLOSED);
-            break;
-        default:
-            /* everything else we jump to here */
-            set_state(stream, H2_STREAM_ST_CLOSED_INPUT);
-            break;
+    apr_status_t status;
+    AP_DEBUG_ASSERT(stream);
+    if (stream->rst_error) {
+        return APR_ECONNRESET;
     }
-    return 1;
+    set_state(stream, H2_STREAM_ST_OPEN);
+    status = h2_request_rwrite(stream->request, r);
+    return status;
 }
 
-apr_status_t h2_stream_rwrite(h2_stream *stream, request_rec *r)
+void h2_stream_set_h2_request(h2_stream *stream, int initiated_on,
+                              const h2_request *req)
+{
+    h2_request_copy(stream->pool, stream->request, req);
+    stream->initiated_on = initiated_on;
+    stream->request->eoh = 0;
+}
+
+apr_status_t h2_stream_add_header(h2_stream *stream,
+                                  const char *name, size_t nlen,
+                                  const char *value, size_t vlen)
 {
-    apr_status_t status;
     AP_DEBUG_ASSERT(stream);
-    set_state(stream, H2_STREAM_ST_OPEN);
-    status = h2_request_rwrite(stream->request, r, stream->m);
-    return status;
+    if (h2_stream_is_scheduled(stream)) {
+        return h2_request_add_trailer(stream->request, stream->pool,
+                                      name, nlen, value, vlen);
+    }
+    else {
+        if (!input_open(stream)) {
+            return APR_ECONNRESET;
+        }
+        return h2_request_add_header(stream->request, stream->pool,
+                                     name, nlen, value, vlen);
+    }
 }
 
-apr_status_t h2_stream_write_eoh(h2_stream *stream, int eos)
+apr_status_t h2_stream_schedule(h2_stream *stream, int eos,
+                                h2_stream_pri_cmp *cmp, void *ctx)
 {
     apr_status_t status;
     AP_DEBUG_ASSERT(stream);
+    AP_DEBUG_ASSERT(stream->session);
+    AP_DEBUG_ASSERT(stream->session->mplx);
+    
+    if (!output_open(stream)) {
+        return APR_ECONNRESET;
+    }
+    if (stream->scheduled) {
+        return APR_EINVAL;
+    }
+    if (eos) {
+        close_input(stream);
+    }
     
     /* Seeing the end-of-headers, we have everything we need to 
      * start processing it.
      */
-    status = h2_mplx_create_task(stream->m, stream);
+    status = h2_request_end_headers(stream->request, stream->pool, eos);
     if (status == APR_SUCCESS) {
-        status = h2_request_end_headers(stream->request, 
-                                        stream->m, stream->task, eos);
-        if (status == APR_SUCCESS) {
-            status = h2_mplx_do_task(stream->m, stream->task);
-        }
-        if (eos) {
-            status = h2_stream_write_eos(stream);
+        if (!eos) {
+            stream->bbin = apr_brigade_create(stream->pool, 
+                                              stream->session->c->bucket_alloc);
         }
-        ap_log_cerror(APLOG_MARK, APLOG_DEBUG, status, stream->m->c,
-                      "h2_mplx(%ld-%d): start stream, task %s %s (%s)",
-                      stream->m->id, stream->id,
-                      stream->request->method, stream->request->path,
-                      stream->request->authority);
+        stream->input_remaining = stream->request->content_length;
         
+        status = h2_mplx_process(stream->session->mplx, stream->id, 
+                                 stream->request, eos, cmp, ctx);
+        stream->scheduled = 1;
+        
+        ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, stream->session->c,
+                      "h2_stream(%ld-%d): scheduled %s %s://%s%s",
+                      stream->session->id, stream->id,
+                      stream->request->method, stream->request->scheme,
+                      stream->request->authority, stream->request->path);
+    }
+    else {
+        h2_stream_rst(stream, H2_ERR_INTERNAL_ERROR);
+        ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, stream->session->c,
+                      "h2_stream(%ld-%d): RST=2 (internal err) %s %s://%s%s",
+                      stream->session->id, stream->id,
+                      stream->request->method, stream->request->scheme,
+                      stream->request->authority, stream->request->path);
     }
+    
     return status;
 }
 
-apr_status_t h2_stream_write_eos(h2_stream *stream)
+int h2_stream_is_scheduled(h2_stream *stream)
 {
-    AP_DEBUG_ASSERT(stream);
-    ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, stream->m->c,
-                  "h2_stream(%ld-%d): closing input",
-                  stream->m->id, stream->id);
-    if (set_closed(stream)) {
-        return h2_request_close(stream->request);
+    return stream->scheduled;
+}
+
+static apr_status_t h2_stream_input_flush(h2_stream *stream)
+{
+    apr_status_t status = APR_SUCCESS;
+    if (stream->bbin && !APR_BRIGADE_EMPTY(stream->bbin)) {
+
+        status = h2_mplx_in_write(stream->session->mplx, stream->id, stream->bbin);
+        if (status != APR_SUCCESS) {
+            ap_log_cerror(APLOG_MARK, APLOG_DEBUG, status, stream->session->mplx->c,
+                          "h2_stream(%ld-%d): flushing input data",
+                          stream->session->id, stream->id);
+        }
     }
-    return APR_SUCCESS;
+    return status;
+}
+
+static apr_status_t input_flush(apr_bucket_brigade *bb, void *ctx) 
+{
+    (void)bb;
+    return h2_stream_input_flush(ctx);
+}
+
+static apr_status_t input_add_data(h2_stream *stream,
+                                   const char *data, size_t len, int chunked)
+{
+    apr_status_t status = APR_SUCCESS;
+    
+    if (chunked) {
+        status = apr_brigade_printf(stream->bbin, input_flush, stream,
+                                    "%lx\r\n", (unsigned long)len);
+        if (status == APR_SUCCESS) {
+            status = apr_brigade_write(stream->bbin, input_flush, stream, data, len);
+            if (status == APR_SUCCESS) {
+                status = apr_brigade_puts(stream->bbin, input_flush, stream, "\r\n");
+            }
+        }
+    }
+    else {
+        status = apr_brigade_write(stream->bbin, input_flush, stream, data, len);
+    }
+    return status;
+}
+
+static int input_add_header(void *str, const char *key, const char *value)
+{
+    h2_stream *stream = str;
+    apr_status_t status = input_add_data(stream, key, strlen(key), 0);
+    if (status == APR_SUCCESS) {
+        status = input_add_data(stream, ": ", 2, 0);
+        if (status == APR_SUCCESS) {
+            status = input_add_data(stream, value, strlen(value), 0);
+            if (status == APR_SUCCESS) {
+                status = input_add_data(stream, "\r\n", 2, 0);
+            }
+        }
+    }
+    return (status == APR_SUCCESS);
 }
 
-apr_status_t h2_stream_write_header(h2_stream *stream,
-                                    const char *name, size_t nlen,
-                                    const char *value, size_t vlen)
+apr_status_t h2_stream_close_input(h2_stream *stream)
 {
+    apr_status_t status = APR_SUCCESS;
+    
     AP_DEBUG_ASSERT(stream);
-    switch (stream->state) {
-        case H2_STREAM_ST_IDLE:
-            set_state(stream, H2_STREAM_ST_OPEN);
-            break;
-        case H2_STREAM_ST_OPEN:
-            break;
-        default:
-            return APR_EINVAL;
+    ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, stream->session->c,
+                  "h2_stream(%ld-%d): closing input",
+                  stream->session->id, stream->id);
+                  
+    if (stream->rst_error) {
+        return APR_ECONNRESET;
     }
-    return h2_request_write_header(stream->request, name, nlen,
-                                   value, vlen, stream->m);
+    
+    H2_STREAM_IN(APLOG_TRACE2, stream, "close_pre");
+    if (close_input(stream) && stream->bbin) {
+        if (stream->request->chunked) {
+            apr_table_t *trailers = stream->request->trailers;
+            if (trailers && !apr_is_empty_table(trailers)) {
+                status = input_add_data(stream, "0\r\n", 3, 0);
+                apr_table_do(input_add_header, stream, trailers, NULL);
+                status = input_add_data(stream, "\r\n", 2, 0);
+            }
+            else {
+                status = input_add_data(stream, "0\r\n\r\n", 5, 0);
+            }
+        }
+        
+        if (status == APR_SUCCESS) {
+            status = h2_stream_input_flush(stream);
+        }
+        if (status == APR_SUCCESS) {
+            status = h2_mplx_in_close(stream->session->mplx, stream->id);
+        }
+    }
+    H2_STREAM_IN(APLOG_TRACE2, stream, "close_post");
+    return status;
 }
 
 apr_status_t h2_stream_write_data(h2_stream *stream,
                                   const char *data, size_t len)
 {
+    apr_status_t status = APR_SUCCESS;
+    
     AP_DEBUG_ASSERT(stream);
-    AP_DEBUG_ASSERT(stream);
-    switch (stream->state) {
-        case H2_STREAM_ST_OPEN:
-            break;
-        default:
-            return APR_EINVAL;
+    if (input_closed(stream) || !stream->request->eoh || !stream->bbin) {
+        ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, stream->session->c,
+                      "h2_stream(%ld-%d): writing denied, closed=%d, eoh=%d, bbin=%d", 
+                      stream->session->id, stream->id, input_closed(stream),
+                      stream->request->eoh, !!stream->bbin);
+        return APR_EINVAL;
+    }
+
+    ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, stream->session->c,
+                  "h2_stream(%ld-%d): add %ld input bytes", 
+                  stream->session->id, stream->id, (long)len);
+
+    H2_STREAM_IN(APLOG_TRACE2, stream, "write_data_pre");
+    if (stream->request->chunked) {
+        /* if input may have a body and we have not seen any
+         * content-length header, we need to chunk the input data.
+         */
+        status = input_add_data(stream, data, len, 1);
     }
-    return h2_request_write_data(stream->request, data, len);
+    else {
+        stream->input_remaining -= len;
+        if (stream->input_remaining < 0) {
+            ap_log_cerror(APLOG_MARK, APLOG_WARNING, 0, stream->session->c,
+                          APLOGNO(02961) 
+                          "h2_stream(%ld-%d): got %ld more content bytes than announced "
+                          "in content-length header: %ld", 
+                          stream->session->id, stream->id,
+                          (long)stream->request->content_length, 
+                          -(long)stream->input_remaining);
+            h2_stream_rst(stream, H2_ERR_PROTOCOL_ERROR);
+            return APR_ECONNABORTED;
+        }
+        status = input_add_data(stream, data, len, 0);
+    }
+    if (status == APR_SUCCESS) {
+        status = h2_stream_input_flush(stream);
+    }
+    H2_STREAM_IN(APLOG_TRACE2, stream, "write_data_post");
+    return status;
 }
 
 apr_status_t h2_stream_prep_read(h2_stream *stream, 
-                                 apr_size_t *plen, int *peos)
+                                 apr_off_t *plen, int *peos)
 {
     apr_status_t status = APR_SUCCESS;
     const char *src;
+    apr_table_t *trailers = NULL;
+    int test_read = (*plen == 0);
     
-    if (stream->bbout && !APR_BRIGADE_EMPTY(stream->bbout)) {
+    if (stream->rst_error) {
+        return APR_ECONNRESET;
+    }
+
+    H2_STREAM_OUT(APLOG_TRACE2, stream, "h2_stream prep_read_pre");
+    if (!APR_BRIGADE_EMPTY(stream->bbout)) {
         src = "stream";
         status = h2_util_bb_avail(stream->bbout, plen, peos);
-        if (status == APR_SUCCESS && !*peos && !*plen) {
+        if (!test_read && status == APR_SUCCESS && !*peos && !*plen) {
             apr_brigade_cleanup(stream->bbout);
             return h2_stream_prep_read(stream, plen, peos);
         }
+        trailers = stream->response? stream->response->trailers : NULL;
     }
     else {
         src = "mplx";
-        status = h2_mplx_out_readx(stream->m, stream->id, 
-                                   NULL, NULL, plen, peos);
+        status = h2_mplx_out_readx(stream->session->mplx, stream->id, 
+                                   NULL, NULL, plen, peos, &trailers);
+        if (trailers && stream->response) {
+            h2_response_set_trailers(stream->response, trailers);
+        }    
     }
-    if (status == APR_SUCCESS && !*peos && !*plen) {
+    
+    if (!test_read && status == APR_SUCCESS && !*peos && !*plen) {
         status = APR_EAGAIN;
     }
-    ap_log_cerror(APLOG_MARK, APLOG_TRACE1, status, stream->m->c,
-                  "h2_stream(%ld-%d): prep_read %s, len=%ld eos=%d",
-                  stream->m->id, stream->id, 
-                  src, (long)*plen, *peos);
+    
+    H2_STREAM_OUT(APLOG_TRACE2, stream, "h2_stream prep_read_post");
+    ap_log_cerror(APLOG_MARK, APLOG_TRACE1, status, stream->session->c,
+                  "h2_stream(%ld-%d): prep_read %s, len=%ld eos=%d, trailers=%s",
+                  stream->session->id, stream->id, src, (long)*plen, *peos,
+                  trailers? "yes" : "no");
     return status;
 }
 
 apr_status_t h2_stream_readx(h2_stream *stream, 
                              h2_io_data_cb *cb, void *ctx,
-                             apr_size_t *plen, int *peos)
+                             apr_off_t *plen, int *peos)
 {
-    if (stream->bbout && !APR_BRIGADE_EMPTY(stream->bbout)) {
-        return h2_util_bb_readx(stream->bbout, cb, ctx, plen, peos);
+    apr_status_t status = APR_SUCCESS;
+    apr_table_t *trailers = NULL;
+    const char *src;
+    
+    H2_STREAM_OUT(APLOG_TRACE2, stream, "h2_stream readx_pre");
+    if (stream->rst_error) {
+        return APR_ECONNRESET;
+    }
+    *peos = 0;
+    if (!APR_BRIGADE_EMPTY(stream->bbout)) {
+        apr_off_t origlen = *plen;
+        
+        src = "stream";
+        status = h2_util_bb_readx(stream->bbout, cb, ctx, plen, peos);
+        if (status == APR_SUCCESS && !*peos && !*plen) {
+            apr_brigade_cleanup(stream->bbout);
+            *plen = origlen;
+            return h2_stream_readx(stream, cb, ctx, plen, peos);
+        }
+    }
+    else {
+        src = "mplx";
+        status = h2_mplx_out_readx(stream->session->mplx, stream->id, 
+                                   cb, ctx, plen, peos, &trailers);
+    }
+    
+    if (trailers && stream->response) {
+        ap_log_cerror(APLOG_MARK, APLOG_TRACE1, status, stream->session->c,
+                      "h2_stream(%ld-%d): readx, saving trailers",
+                      stream->session->id, stream->id);
+        h2_response_set_trailers(stream->response, trailers);
+    }
+    
+    if (status == APR_SUCCESS && !*peos && !*plen) {
+        status = APR_EAGAIN;
     }
-    return h2_mplx_out_readx(stream->m, stream->id, 
-                             cb, ctx, plen, peos);
+    
+    H2_STREAM_OUT(APLOG_TRACE2, stream, "h2_stream readx_post");
+    ap_log_cerror(APLOG_MARK, APLOG_TRACE1, status, stream->session->c,
+                  "h2_stream(%ld-%d): readx %s, len=%ld eos=%d",
+                  stream->session->id, stream->id, src, (long)*plen, *peos);
+    H2_STREAM_OUT(APLOG_TRACE2, stream, "h2_stream readx_post");
+    
+    return status;
 }
 
+apr_status_t h2_stream_read_to(h2_stream *stream, apr_bucket_brigade *bb, 
+                               apr_off_t *plen, int *peos)
+{
+    apr_status_t status = APR_SUCCESS;
+    apr_table_t *trailers = NULL;
+
+    H2_STREAM_OUT(APLOG_TRACE2, stream, "h2_stream read_to_pre");
+    if (stream->rst_error) {
+        return APR_ECONNRESET;
+    }
+    
+    if (APR_BRIGADE_EMPTY(stream->bbout)) {
+        apr_off_t tlen = *plen;
+        int eos;
+        status = h2_mplx_out_read_to(stream->session->mplx, stream->id, 
+                                     stream->bbout, &tlen, &eos, &trailers);
+    }
+    
+    if (status == APR_SUCCESS && !APR_BRIGADE_EMPTY(stream->bbout)) {
+        status = h2_transfer_brigade(bb, stream->bbout, stream->pool, 
+                                     plen, peos);
+    }
+    else {
+        *plen = 0;
+        *peos = 0;
+    }
+
+    if (trailers && stream->response) {
+        ap_log_cerror(APLOG_MARK, APLOG_TRACE1, status, stream->session->c,
+                      "h2_stream(%ld-%d): read_to, saving trailers",
+                      stream->session->id, stream->id);
+        h2_response_set_trailers(stream->response, trailers);
+    }
+    
+    if (status == APR_SUCCESS && !*peos && !*plen) {
+        status = APR_EAGAIN;
+    }
+    H2_STREAM_OUT(APLOG_TRACE2, stream, "h2_stream read_to_post");
+    ap_log_cerror(APLOG_MARK, APLOG_TRACE1, status, stream->session->c,
+                  "h2_stream(%ld-%d): read_to, len=%ld eos=%d",
+                  stream->session->id, stream->id, (long)*plen, *peos);
+    return status;
+}
 
 void h2_stream_set_suspended(h2_stream *stream, int suspended)
 {
     AP_DEBUG_ASSERT(stream);
     stream->suspended = !!suspended;
+    ap_log_cerror(APLOG_MARK, APLOG_TRACE2, 0, stream->session->c,
+                  "h2_stream(%ld-%d): suspended=%d",
+                  stream->session->id, stream->id, stream->suspended);
 }
 
 int h2_stream_is_suspended(h2_stream *stream)
 {
     AP_DEBUG_ASSERT(stream);
     return stream->suspended;
 }
 
+int h2_stream_input_is_open(h2_stream *stream) 
+{
+    return input_open(stream);
+}
+
+int h2_stream_needs_submit(h2_stream *stream)
+{
+    switch (stream->state) {
+        case H2_STREAM_ST_OPEN:
+        case H2_STREAM_ST_CLOSED_INPUT:
+        case H2_STREAM_ST_CLOSED_OUTPUT:
+        case H2_STREAM_ST_CLOSED:
+            return !stream->submitted;
+        default:
+            return 0;
+    }
+}
+
+apr_status_t h2_stream_submit_pushes(h2_stream *stream)
+{
+    apr_status_t status = APR_SUCCESS;
+    apr_array_header_t *pushes;
+    int i;
+    
+    pushes = h2_push_collect(stream->pool, stream->request, stream->response);
+    if (pushes && !apr_is_empty_array(pushes)) {
+        ap_log_cerror(APLOG_MARK, APLOG_TRACE1, 0, stream->session->c,
+                      "h2_stream(%ld-%d): found %d push candidates",
+                      stream->session->id, stream->id, pushes->nelts);
+        for (i = 0; i < pushes->nelts; ++i) {
+            h2_push *push = APR_ARRAY_IDX(pushes, i, h2_push*);
+            h2_stream *s = h2_session_push(stream->session, stream, push);
+            if (!s) {
+                status = APR_ECONNRESET;
+                break;
+            }
+        }
+    }
+    return status;
+}
+
+apr_table_t *h2_stream_get_trailers(h2_stream *stream)
+{
+    return stream->response? stream->response->trailers : NULL;
+}
+
+const h2_priority *h2_stream_get_priority(h2_stream *stream)
+{
+    if (stream->initiated_on && stream->response) {
+        const char *ctype = apr_table_get(stream->response->headers, "content-type");
+        if (ctype) {
+            /* FIXME: Not good enough, config needs to come from request->server */
+            return h2_config_get_priority(stream->session->config, ctype);
+        }
+    }
+    return NULL;
+}
+

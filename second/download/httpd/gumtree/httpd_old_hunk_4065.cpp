#include "h2_response.h"
#include "h2_task_output.h"
#include "h2_task.h"
#include "h2_util.h"


h2_task_output *h2_task_output_create(h2_task *task, apr_pool_t *pool)
{
    h2_task_output *output = apr_pcalloc(pool, sizeof(h2_task_output));
    
    if (output) {
        output->task = task;
        output->state = H2_TASK_OUT_INIT;
        output->from_h1 = h2_from_h1_create(task->stream_id, pool);
        if (!output->from_h1) {
            return NULL;
        }
    }
    return output;
}

void h2_task_output_destroy(h2_task_output *output)
{
    if (output->from_h1) {
        h2_from_h1_destroy(output->from_h1);
        output->from_h1 = NULL;
    }
}

static apr_status_t open_if_needed(h2_task_output *output, ap_filter_t *f,
                                   apr_bucket_brigade *bb)
{
    if (output->state == H2_TASK_OUT_INIT) {
        h2_response *response;
        output->state = H2_TASK_OUT_STARTED;
        response = h2_from_h1_get_response(output->from_h1);
        if (!response) {
            if (f) {
                /* This happens currently when ap_die(status, r) is invoked
                 * by a read request filter.
                 */
                ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, f->c,
                              "h2_task_output(%s): write without response "
                              "for %s %s %s",
                              output->task->id, output->task->request->method, 
                              output->task->request->authority, 
                              output->task->request->path);
                f->c->aborted = 1;
            }
            if (output->task->io) {
                apr_thread_cond_broadcast(output->task->io);
            }
            return APR_ECONNABORTED;
        }
        
        output->trailers_passed = !!response->trailers;
        return h2_mplx_out_open(output->task->mplx, output->task->stream_id, 
                                response, f, bb, output->task->io);
    }
    return APR_EOF;
}

static apr_table_t *get_trailers(h2_task_output *output)
{
    if (!output->trailers_passed) {
        h2_response *response = h2_from_h1_get_response(output->from_h1);
        if (response && response->trailers) {
            output->trailers_passed = 1;
            return response->trailers;
        }
    }
    return NULL;
}

void h2_task_output_close(h2_task_output *output)
{
    open_if_needed(output, NULL, NULL);
    if (output->state != H2_TASK_OUT_DONE) {
        h2_mplx_out_close(output->task->mplx, output->task->stream_id, 
                          get_trailers(output));
        output->state = H2_TASK_OUT_DONE;
    }
}

int h2_task_output_has_started(h2_task_output *output)
{
    return output->state >= H2_TASK_OUT_STARTED;
}

/* Bring the data from the brigade (which represents the result of the
 * request_rec out filter chain) into the h2_mplx for further sending
 * on the master connection. 
 */
apr_status_t h2_task_output_write(h2_task_output *output,
                                  ap_filter_t* f, apr_bucket_brigade* bb)
{
    apr_status_t status;
    
    if (APR_BRIGADE_EMPTY(bb)) {
        ap_log_cerror(APLOG_MARK, APLOG_TRACE1, 0, f->c,
                      "h2_task_output(%s): empty write", output->task->id);
        return APR_SUCCESS;
    }
    
    status = open_if_needed(output, f, bb);
    if (status != APR_EOF) {
        ap_log_cerror(APLOG_MARK, APLOG_TRACE1, status, f->c,
                      "h2_task_output(%s): opened and passed brigade", 
                      output->task->id);
        return status;
    }
    
    ap_log_cerror(APLOG_MARK, APLOG_TRACE1, 0, f->c,
                  "h2_task_output(%s): write brigade", output->task->id);
    return h2_mplx_out_write(output->task->mplx, output->task->stream_id, 
                             f, bb, get_trailers(output), output->task->io);
}


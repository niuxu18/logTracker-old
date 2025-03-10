     if (!srv->flags.reserved)
         return;
 
     ++ srv->stats.releases;
 
     srv->flags.reserved = false;
-    if (srv->parent->OnEmptyQueue != NULL && srv->data)
-        srv->parent->OnEmptyQueue(srv->data);
 
     helperStatefulServerDone(srv);
 }
 
 /** return a pointer to the stateful routines data area */
 void *
 helperStatefulServerGetData(helper_stateful_server * srv)
 {
     return srv->data;
 }
 
-/**
- * Dump some stats about the helper states to a StoreEntry
- */
-void
-helperStats(StoreEntry * sentry, helper * hlp, const char *label)
-{
-    if (!helperStartStats(sentry, hlp, label))
-        return;
-
-    storeAppendPrintf(sentry, "program: %s\n",
-                      hlp->cmdline->key);
-    storeAppendPrintf(sentry, "number active: %d of %d (%d shutting down)\n",
-                      hlp->childs.n_active, hlp->childs.n_max, (hlp->childs.n_running - hlp->childs.n_active) );
-    storeAppendPrintf(sentry, "requests sent: %d\n",
-                      hlp->stats.requests);
-    storeAppendPrintf(sentry, "replies received: %d\n",
-                      hlp->stats.replies);
-    storeAppendPrintf(sentry, "queue length: %d\n",
-                      hlp->stats.queue_size);
-    storeAppendPrintf(sentry, "avg service time: %d msec\n",
-                      hlp->stats.avg_svc_time);
-    storeAppendPrintf(sentry, "\n");
-    storeAppendPrintf(sentry, "%7s\t%7s\t%7s\t%11s\t%11s\t%s\t%7s\t%7s\t%7s\n",
-                      "ID #",
-                      "FD",
-                      "PID",
-                      "# Requests",
-                      "# Replies",
-                      "Flags",
-                      "Time",
-                      "Offset",
-                      "Request");
-
-    for (dlink_node *link = hlp->servers.head; link; link = link->next) {
-        helper_server *srv = (helper_server*)link->data;
-        double tt = 0.001 * (srv->requests[0] ? tvSubMsec(srv->requests[0]->dispatch_time, current_time) : tvSubMsec(srv->dispatch_time, srv->answer_time));
-        storeAppendPrintf(sentry, "%7u\t%7d\t%7d\t%11" PRIu64 "\t%11" PRIu64 "\t%c%c%c%c\t%7.3f\t%7d\t%s\n",
-                          srv->index.value,
-                          srv->readPipe->fd,
-                          srv->pid,
-                          srv->stats.uses,
-                          srv->stats.replies,
-                          srv->stats.pending ? 'B' : ' ',
-                          srv->flags.writing ? 'W' : ' ',
-                          srv->flags.closing ? 'C' : ' ',
-                          srv->flags.shutdown ? 'S' : ' ',
-                          tt < 0.0 ? 0.0 : tt,
-                          (int) srv->roffset,
-                          srv->requests[0] ? Format::QuoteMimeBlob(srv->requests[0]->buf) : "(none)");
-    }
-
-    storeAppendPrintf(sentry, "\nFlags key:\n\n");
-    storeAppendPrintf(sentry, "   B = BUSY\n");
-    storeAppendPrintf(sentry, "   W = WRITING\n");
-    storeAppendPrintf(sentry, "   C = CLOSING\n");
-    storeAppendPrintf(sentry, "   S = SHUTDOWN PENDING\n");
-}
-
 void
-helperStatefulStats(StoreEntry * sentry, statefulhelper * hlp, const char *label)
+helper::packStatsInto(Packable *p, const char *label) const
 {
-    if (!helperStartStats(sentry, hlp, label))
-        return;
+    if (label)
+        p->appendf("%s:\n", label);
 
-    storeAppendPrintf(sentry, "program: %s\n",
-                      hlp->cmdline->key);
-    storeAppendPrintf(sentry, "number active: %d of %d (%d shutting down)\n",
-                      hlp->childs.n_active, hlp->childs.n_max, (hlp->childs.n_running - hlp->childs.n_active) );
-    storeAppendPrintf(sentry, "requests sent: %d\n",
-                      hlp->stats.requests);
-    storeAppendPrintf(sentry, "replies received: %d\n",
-                      hlp->stats.replies);
-    storeAppendPrintf(sentry, "queue length: %d\n",
-                      hlp->stats.queue_size);
-    storeAppendPrintf(sentry, "avg service time: %d msec\n",
-                      hlp->stats.avg_svc_time);
-    storeAppendPrintf(sentry, "\n");
-    storeAppendPrintf(sentry, "%7s\t%7s\t%7s\t%11s\t%11s\t%6s\t%7s\t%7s\t%7s\n",
-                      "ID #",
-                      "FD",
-                      "PID",
-                      "# Requests",
-                      "# Replies",
-                      "Flags",
-                      "Time",
-                      "Offset",
-                      "Request");
-
-    for (dlink_node *link = hlp->servers.head; link; link = link->next) {
-        helper_stateful_server *srv = (helper_stateful_server *)link->data;
-        double tt = 0.001 * tvSubMsec(srv->dispatch_time, srv->stats.pending ? current_time : srv->answer_time);
-        storeAppendPrintf(sentry, "%7u\t%7d\t%7d\t%11" PRIu64 "\t%11" PRIu64 "\t%c%c%c%c%c\t%7.3f\t%7d\t%s\n",
-                          srv->index.value,
-                          srv->readPipe->fd,
-                          srv->pid,
-                          srv->stats.uses,
-                          srv->stats.replies,
-                          srv->stats.pending ? 'B' : ' ',
-                          srv->flags.closing ? 'C' : ' ',
-                          srv->flags.reserved ? 'R' : ' ',
-                          srv->flags.shutdown ? 'S' : ' ',
-                          srv->request ? (srv->request->placeholder ? 'P' : ' ') : ' ',
-                          tt < 0.0 ? 0.0 : tt,
-                          (int) srv->roffset,
-                          srv->request ? Format::QuoteMimeBlob(srv->request->buf) : "(none)");
-    }
-
-    storeAppendPrintf(sentry, "\nFlags key:\n\n");
-    storeAppendPrintf(sentry, "   B = BUSY\n");
-    storeAppendPrintf(sentry, "   C = CLOSING\n");
-    storeAppendPrintf(sentry, "   R = RESERVED\n");
-    storeAppendPrintf(sentry, "   S = SHUTDOWN PENDING\n");
-    storeAppendPrintf(sentry, "   P = PLACEHOLDER\n");
+    p->appendf("  program: %s\n", cmdline->key);
+    p->appendf("  number active: %d of %d (%d shutting down)\n", childs.n_active, childs.n_max, (childs.n_running - childs.n_active));
+    p->appendf("  requests sent: %d\n", stats.requests);
+    p->appendf("  replies received: %d\n", stats.replies);
+    p->appendf("  requests timedout: %d\n", stats.timedout);
+    p->appendf("  queue length: %d\n", stats.queue_size);
+    p->appendf("  avg service time: %d msec\n", stats.avg_svc_time);
+    p->append("\n",1);
+    p->appendf("%7s\t%7s\t%7s\t%11s\t%11s\t%11s\t%6s\t%7s\t%7s\t%7s\n",
+               "ID #",
+               "FD",
+               "PID",
+               "# Requests",
+               "# Replies",
+               "# Timed-out",
+               "Flags",
+               "Time",
+               "Offset",
+               "Request");
+
+    for (dlink_node *link = servers.head; link; link = link->next) {
+        HelperServerBase *srv = static_cast<HelperServerBase *>(link->data);
+        assert(srv);
+        Helper::Xaction *xaction = srv->requests.empty() ? NULL : srv->requests.front();
+        double tt = 0.001 * (xaction ? tvSubMsec(xaction->request.dispatch_time, current_time) : tvSubMsec(srv->dispatch_time, srv->answer_time));
+        p->appendf("%7u\t%7d\t%7d\t%11" PRIu64 "\t%11" PRIu64 "\t%11" PRIu64 "\t%c%c%c%c%c%c\t%7.3f\t%7d\t%s\n",
+                   srv->index.value,
+                   srv->readPipe->fd,
+                   srv->pid,
+                   srv->stats.uses,
+                   srv->stats.replies,
+                   srv->stats.timedout,
+                   srv->stats.pending ? 'B' : ' ',
+                   srv->flags.writing ? 'W' : ' ',
+                   srv->flags.closing ? 'C' : ' ',
+                   srv->flags.reserved ? 'R' : ' ',
+                   srv->flags.shutdown ? 'S' : ' ',
+                   xaction && xaction->request.placeholder ? 'P' : ' ',
+                   tt < 0.0 ? 0.0 : tt,
+                   (int) srv->roffset,
+                   xaction ? Format::QuoteMimeBlob(xaction->request.buf) : "(none)");
+    }
+
+    p->append("\nFlags key:\n"
+              "   B\tBUSY\n"
+              "   W\tWRITING\n"
+              "   C\tCLOSING\n"
+              "   R\tRESERVED\n"
+              "   S\tSHUTDOWN PENDING\n"
+              "   P\tPLACEHOLDER\n", 101);
+}
+
+bool
+helper::willOverload() const {
+    return queueFull() && !(childs.needNew() || GetFirstAvailable(this));
 }
 
 void
 helperShutdown(helper * hlp)
 {
     dlink_node *link = hlp->servers.head;

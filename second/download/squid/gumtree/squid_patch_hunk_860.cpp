     if (ssl_crtd->stats.queue_size >= (int)(ssl_crtd->childs.n_running * 2)) {
         if (first_warn == 0)
             first_warn = squid_curtime;
         if (squid_curtime - first_warn > 3 * 60)
             fatal("SSL servers not responding for 3 minutes");
         debugs(34, DBG_IMPORTANT, HERE << "Queue overload, rejecting");
-        HelperReply failReply;
-        failReply.result = HelperReply::BrokenHelper;
+        ::Helper::Reply failReply;
+        failReply.result = ::Helper::BrokenHelper;
         failReply.notes.add("message", "error 45 Temporary network problem, please retry later");
         callback(data, failReply);
         return;
     }
 
     first_warn = 0;

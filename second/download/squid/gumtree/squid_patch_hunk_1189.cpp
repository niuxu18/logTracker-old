     delete ssl_crtd;
     ssl_crtd = NULL;
 }
 
 void Ssl::Helper::sslSubmit(CrtdMessage const & message, HLPCB * callback, void * data)
 {
-    static time_t first_warn = 0;
     assert(ssl_crtd);
 
-    if (ssl_crtd->stats.queue_size >= (int)(ssl_crtd->childs.n_running * 2)) {
-        if (first_warn == 0)
-            first_warn = squid_curtime;
-        if (squid_curtime - first_warn > 3 * 60)
-            fatal("SSL servers not responding for 3 minutes");
-        debugs(34, DBG_IMPORTANT, HERE << "Queue overload, rejecting");
-        ::Helper::Reply failReply;
-        failReply.result = ::Helper::BrokenHelper;
+    std::string msg = message.compose();
+    msg += '\n';
+    if (!ssl_crtd->trySubmit(msg.c_str(), callback, data)) {
+        ::Helper::Reply failReply(::Helper::BrokenHelper);
         failReply.notes.add("message", "error 45 Temporary network problem, please retry later");
         callback(data, failReply);
         return;
     }
-
-    first_warn = 0;
-    std::string msg = message.compose();
-    msg += '\n';
-    helperSubmit(ssl_crtd, msg.c_str(), callback, data);
 }
 #endif //USE_SSL_CRTD
 
 Ssl::CertValidationHelper * Ssl::CertValidationHelper::GetInstance()
 {
     static Ssl::CertValidationHelper sslHelper;

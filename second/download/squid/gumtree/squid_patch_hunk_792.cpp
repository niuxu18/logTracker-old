         if (checklist->keepMatching())
             checklist->markFinished(answer, "aclMatchExternal exception");
         return -1; // other
     }
 }
 
-wordlist *
+SBufList
 ACLExternal::dump() const
 {
     external_acl_data const *acl = data;
-    wordlist *result = NULL;
-    wordlist *arg;
-    MemBuf mb;
-    mb.init();
-    mb.Printf("%s", acl->def->name);
+    SBufList rv;
+    rv.push_back(SBuf(acl->def->name));
 
-    for (arg = acl->arguments; arg; arg = arg->next) {
-        mb.Printf(" %s", arg->key);
+    for (wordlist *arg = acl->arguments; arg; arg = arg->next) {
+        SBuf s;
+        s.Printf(" %s", arg->key);
+        rv.push_back(s);
     }
 
-    wordlistAdd(&result, mb.buf);
-    mb.clean();
-    return result;
+    return rv;
 }
 
 /******************************************************************
  * external_acl cache
  */
 
 static void
 external_acl_cache_touch(external_acl * def, const ExternalACLEntryPointer &entry)
 {
     // this must not be done when nothing is being cached.
-    if (def->cache_size <= 0 || (def->ttl <= 0 && entry->result == 1) || (def->negative_ttl <= 0 && entry->result != 1))
+    if (!def->maybeCacheable(entry->result))
         return;
 
     dlinkDelete(&entry->lru, &def->lru_list);
     ExternalACLEntry *e = const_cast<ExternalACLEntry *>(entry.getRaw()); // XXX: make hash a std::map of Pointer.
     dlinkAdd(e, &entry->lru, &def->lru_list);
 }
 
+#if USE_OPENSSL
+static const char *
+external_acl_ssl_get_user_attribute(const ACLFilledChecklist &ch, const char *attr)
+{
+    if (ch.conn() != NULL && Comm::IsConnOpen(ch.conn()->clientConnection)) {
+        if (SSL *ssl = fd_table[ch.conn()->clientConnection->fd].ssl)
+            return sslGetUserAttribute(ssl, attr);
+    }
+    return NULL;
+}
+#endif
+
 static char *
 makeExternalAclKey(ACLFilledChecklist * ch, external_acl_data * acl_data)
 {
     static MemBuf mb;
     char buf[256];
     int first = 1;
     wordlist *arg;
-    external_acl_format *format;
     HttpRequest *request = ch->request;
     HttpReply *reply = ch->reply;
     mb.reset();
     bool data_used = false;
 
-    for (format = acl_data->def->format; format; format = format->next) {
+    for (external_acl_format::Pointer format = acl_data->def->format; format != NULL; format = format->next) {
         const char *str = NULL;
         String sb;
 
         switch (format->type) {
 #if USE_AUTH
-        case _external_acl_format::EXT_ACL_LOGIN:
+        case Format::LFT_USER_LOGIN:
             // if this ACL line was the cause of credentials fetch
             // they may not already be in the checklist
             if (ch->auth_user_request == NULL && ch->request)
                 ch->auth_user_request = ch->request->auth_user_request;
 
             if (ch->auth_user_request != NULL)
                 str = ch->auth_user_request->username();
             break;
 #endif
 #if USE_IDENT
-        case _external_acl_format::EXT_ACL_IDENT:
+        case Format::LFT_USER_IDENT:
             str = ch->rfc931;
 
             if (!str || !*str) {
                 // if we fail to go async, we still return NULL and the caller
                 // will detect the failure in ACLExternal::match().
                 (void)ch->goAsync(IdentLookup::Instance());
                 return NULL;
             }
 
             break;
 #endif
 
-        case _external_acl_format::EXT_ACL_SRC:
+        case Format::LFT_CLIENT_IP_ADDRESS:
             str = ch->src_addr.toStr(buf,sizeof(buf));
             break;
 
-        case _external_acl_format::EXT_ACL_SRCPORT:
+        case Format::LFT_CLIENT_PORT:
             snprintf(buf, sizeof(buf), "%d", request->client_addr.port());
             str = buf;
             break;
 
 #if USE_SQUID_EUI
-        case _external_acl_format::EXT_ACL_SRCEUI48:
+        case Format::LFT_CLIENT_EUI:
+            // TODO make the ACL checklist have a direct link to any TCP details.
+            if (request->clientConnectionManager.valid() && request->clientConnectionManager->clientConnection != NULL)
+            {
+                if (request->clientConnectionManager->clientConnection->remote.isIPv4())
+                    request->clientConnectionManager->clientConnection->remoteEui48.encode(buf, sizeof(buf));
+                else
+                    request->clientConnectionManager->clientConnection->remoteEui64.encode(buf, sizeof(buf));
+                str = buf;
+            }
+            break;
+
+        case Format::LFT_EXT_ACL_CLIENT_EUI48:
             if (request->clientConnectionManager.valid() && request->clientConnectionManager->clientConnection != NULL &&
                     request->clientConnectionManager->clientConnection->remoteEui48.encode(buf, sizeof(buf)))
                 str = buf;
             break;
 
-        case _external_acl_format::EXT_ACL_SRCEUI64:
+        case Format::LFT_EXT_ACL_CLIENT_EUI64:
             if (request->clientConnectionManager.valid() && request->clientConnectionManager->clientConnection != NULL &&
                     request->clientConnectionManager->clientConnection->remoteEui64.encode(buf, sizeof(buf)))
                 str = buf;
             break;
 #endif
 
-        case _external_acl_format::EXT_ACL_MYADDR:
+        case Format::LFT_LOCAL_LISTENING_IP:
             str = request->my_addr.toStr(buf, sizeof(buf));
             break;
 
-        case _external_acl_format::EXT_ACL_MYPORT:
+        case Format::LFT_LOCAL_LISTENING_PORT:
             snprintf(buf, sizeof(buf), "%d", request->my_addr.port());
             str = buf;
             break;
 
-        case _external_acl_format::EXT_ACL_URI:
+        case Format::LFT_CLIENT_REQ_URI:
             str = urlCanonical(request);
             break;
 
-        case _external_acl_format::EXT_ACL_DST:
+        case Format::LFT_CLIENT_REQ_URLDOMAIN:
             str = request->GetHost();
             break;
 
-        case _external_acl_format::EXT_ACL_PROTO:
-            str = AnyP::ProtocolType_str[request->protocol];
+        case Format::LFT_CLIENT_REQ_URLSCHEME:
+            str = request->url.getScheme().c_str();
             break;
 
-        case _external_acl_format::EXT_ACL_PORT:
+        case Format::LFT_CLIENT_REQ_URLPORT:
             snprintf(buf, sizeof(buf), "%d", request->port);
             str = buf;
             break;
 
-        case _external_acl_format::EXT_ACL_PATH:
+        case Format::LFT_CLIENT_REQ_URLPATH:
             str = request->urlpath.termedBuf();
             break;
 
-        case _external_acl_format::EXT_ACL_METHOD:
-            str = RequestMethodStr(request->method);
-            break;
-
-        case _external_acl_format::EXT_ACL_HEADER_REQUEST:
-            sb = request->header.getByName(format->header);
-            str = sb.termedBuf();
-            break;
-
-        case _external_acl_format::EXT_ACL_HEADER_REQUEST_ID:
-            sb = request->header.getStrOrList(format->header_id);
-            str = sb.termedBuf();
-            break;
-
-        case _external_acl_format::EXT_ACL_HEADER_REQUEST_MEMBER:
-            sb = request->header.getByNameListMember(format->header, format->member, format->separator);
+        case Format::LFT_CLIENT_REQ_METHOD: {
+            const SBuf &s = request->method.image();
+            sb.append(s.rawContent(), s.length());
+        }
+        str = sb.termedBuf();
+        break;
+
+        case Format::LFT_ADAPTED_REQUEST_HEADER:
+            if (format->header_id == -1)
+                sb = request->header.getByName(format->header);
+            else
+                sb = request->header.getStrOrList(format->header_id);
             str = sb.termedBuf();
             break;
 
-        case _external_acl_format::EXT_ACL_HEADER_REQUEST_ID_MEMBER:
-            sb = request->header.getListMember(format->header_id, format->member, format->separator);
+        case Format::LFT_ADAPTED_REQUEST_HEADER_ELEM:
+            if (format->header_id == -1)
+                sb = request->header.getByNameListMember(format->header, format->member, format->separator);
+            else
+                sb = request->header.getListMember(format->header_id, format->member, format->separator);
             str = sb.termedBuf();
             break;
 
-        case _external_acl_format::EXT_ACL_HEADER_REPLY:
-            if (reply) {
-                sb = reply->header.getByName(format->header);
-                str = sb.termedBuf();
-            }
-            break;
-
-        case _external_acl_format::EXT_ACL_HEADER_REPLY_ID:
+        case Format::LFT_REPLY_HEADER:
             if (reply) {
-                sb = reply->header.getStrOrList(format->header_id);
+                if (format->header_id == -1)
+                    sb = reply->header.getByName(format->header);
+                else
+                    sb = reply->header.getStrOrList(format->header_id);
                 str = sb.termedBuf();
             }
             break;
 
-        case _external_acl_format::EXT_ACL_HEADER_REPLY_MEMBER:
+        case Format::LFT_REPLY_HEADER_ELEM:
             if (reply) {
-                sb = reply->header.getByNameListMember(format->header, format->member, format->separator);
+                if (format->header_id == -1)
+                    sb = reply->header.getByNameListMember(format->header, format->member, format->separator);
+                else
+                    sb = reply->header.getListMember(format->header_id, format->member, format->separator);
                 str = sb.termedBuf();
             }
             break;
 
-        case _external_acl_format::EXT_ACL_HEADER_REPLY_ID_MEMBER:
-            if (reply) {
-                sb = reply->header.getListMember(format->header_id, format->member, format->separator);
-                str = sb.termedBuf();
-            }
-            break;
-#if USE_SSL
+#if USE_OPENSSL
 
-        case _external_acl_format::EXT_ACL_USER_CERT_RAW:
+        case Format::LFT_EXT_ACL_USER_CERT_RAW:
 
             if (ch->conn() != NULL && Comm::IsConnOpen(ch->conn()->clientConnection)) {
                 SSL *ssl = fd_table[ch->conn()->clientConnection->fd].ssl;
 
                 if (ssl)
                     str = sslGetUserCertificatePEM(ssl);
             }
 
             break;
 
-        case _external_acl_format::EXT_ACL_USER_CERTCHAIN_RAW:
+        case Format::LFT_EXT_ACL_USER_CERTCHAIN_RAW:
 
             if (ch->conn() != NULL && Comm::IsConnOpen(ch->conn()->clientConnection)) {
                 SSL *ssl = fd_table[ch->conn()->clientConnection->fd].ssl;
 
                 if (ssl)
                     str = sslGetUserCertificateChainPEM(ssl);
             }
 
             break;
 
-        case _external_acl_format::EXT_ACL_USER_CERT:
+        case Format::LFT_EXT_ACL_USER_CERT:
+            str = external_acl_ssl_get_user_attribute(*ch, format->header);
+            break;
+
+        case Format::LFT_EXT_ACL_USER_CA_CERT:
 
             if (ch->conn() != NULL && Comm::IsConnOpen(ch->conn()->clientConnection)) {
                 SSL *ssl = fd_table[ch->conn()->clientConnection->fd].ssl;
 
                 if (ssl)
-                    str = sslGetUserAttribute(ssl, format->header);
+                    str = sslGetCAAttribute(ssl, format->header);
             }
 
             break;
 
-        case _external_acl_format::EXT_ACL_USER_CA_CERT:
-
-            if (ch->conn() != NULL && Comm::IsConnOpen(ch->conn()->clientConnection)) {
-                SSL *ssl = fd_table[ch->conn()->clientConnection->fd].ssl;
-
-                if (ssl)
-                    str = sslGetCAAttribute(ssl, format->header);
+        case Format::LFT_SSL_CLIENT_SNI:
+            if (ch->conn() != NULL) {
+                if (Ssl::ServerBump * srvBump = ch->conn()->serverBump()) {
+                    if (!srvBump->clientSni.isEmpty())
+                        str = srvBump->clientSni.c_str();
+                }
             }
+            break;
 
+        case Format::LFT_SSL_SERVER_CERT_SUBJECT:
+        case Format::LFT_SSL_SERVER_CERT_ISSUER: {
+            X509 *serverCert = NULL;
+            if (ch->serverCert.get())
+                serverCert = ch->serverCert.get();
+            else if (ch->conn() && ch->conn()->serverBump())
+                serverCert = ch->conn()->serverBump()->serverCert.get();
+
+            if (serverCert) {
+                if (format->type == Format::LFT_SSL_SERVER_CERT_SUBJECT)
+                    str = Ssl::GetX509UserAttribute(serverCert, "DN");
+                else
+                    str = Ssl::GetX509CAAttribute(serverCert, "DN");
+            }
             break;
+        }
+
 #endif
 #if USE_AUTH
-        case _external_acl_format::EXT_ACL_EXT_USER:
+        case Format::LFT_USER_EXTERNAL:
             str = request->extacl_user.termedBuf();
             break;
 #endif
-        case _external_acl_format::EXT_ACL_EXT_LOG:
+        case Format::LFT_USER_NAME:
+            /* find the first available name from various sources */
+#if USE_AUTH
+            // if this ACL line was the cause of credentials fetch
+            // they may not already be in the checklist
+            if (!ch->auth_user_request && ch->request)
+                ch->auth_user_request = ch->request->auth_user_request;
+
+            if (ch->auth_user_request != NULL)
+                str = ch->auth_user_request->username();
+
+            if ((!str || !*str) &&
+                    (request->extacl_user.size() > 0 && request->extacl_user[0] != '-'))
+                str = request->extacl_user.termedBuf();
+#endif
+#if USE_OPENSSL
+            if (!str || !*str)
+                str = external_acl_ssl_get_user_attribute(*ch, "CN");
+#endif
+#if USE_IDENT
+            if (!str || !*str)
+                str = ch->rfc931;
+#endif
+            break;
+        case Format::LFT_EXT_LOG:
             str = request->extacl_log.termedBuf();
             break;
-        case _external_acl_format::EXT_ACL_TAG:
+        case Format::LFT_TAG:
             str = request->tag.termedBuf();
             break;
-        case _external_acl_format::EXT_ACL_ACLNAME:
+        case Format::LFT_EXT_ACL_NAME:
             str = acl_data->name;
             break;
-        case _external_acl_format::EXT_ACL_ACLDATA:
+        case Format::LFT_EXT_ACL_DATA:
             data_used = true;
             for (arg = acl_data->arguments; arg; arg = arg->next) {
                 if (!first)
                     sb.append(" ", 1);
 
                 if (acl_data->def->quote == external_acl::QUOTE_METHOD_URL) {

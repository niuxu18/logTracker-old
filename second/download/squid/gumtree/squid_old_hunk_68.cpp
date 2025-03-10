
/* The functions in this file handle authentication.
 * They DO NOT perform access control or auditing.
 * See acl.c for access control and client_side.c for auditing */


#include "squid.h"
#include "auth_ntlm.h"
#include "auth/Gadgets.h"
#include "CacheManager.h"
#include "Store.h"
#include "client_side.h"
#include "HttpReply.h"
#include "HttpRequest.h"
/* TODO remove this include */
#include "ntlmScheme.h"
#include "wordlist.h"
#include "SquidTime.h"


static void
authenticateStateFree(authenticateStateData * r)
{
    AUTHUSERREQUESTUNLOCK(r->auth_user_request, "r");
    cbdataFree(r);
}

/* NTLM Scheme */
static HLPSCB authenticateNTLMHandleReply;
static AUTHSSTATS authenticateNTLMStats;

static statefulhelper *ntlmauthenticators = NULL;

CBDATA_TYPE(authenticateStateData);

static int authntlm_initialised = 0;

static auth_ntlm_config ntlmConfig;

static hash_table *proxy_auth_cache = NULL;

/*
 *
 * Private Functions
 *
 */

/* move to ntlmScheme.cc */
void
ntlmScheme::done()
{
    /* TODO: this should be a Config call. */
    debugs(29, 2, "ntlmScheme::done: shutting down NTLM authentication.");

    if (ntlmauthenticators)
        helperStatefulShutdown(ntlmauthenticators);

    authntlm_initialised = 0;

    if (!shutting_down)
        return;

    if (ntlmauthenticators)
        helperStatefulFree(ntlmauthenticators);

    ntlmauthenticators = NULL;

    debugs(29, 2, "ntlmScheme::done: NTLM authentication Shutdown.");
}

/* free any allocated configuration details */
void
AuthNTLMConfig::done()
{
    if (authenticate)
        wordlistDestroy(&authenticate);
}

void
AuthNTLMConfig::dump(StoreEntry * entry, const char *name, AuthConfig * scheme)
{
    wordlist *list = authenticate;
    storeAppendPrintf(entry, "%s %s", name, "ntlm");

    while (list != NULL) {
        storeAppendPrintf(entry, " %s", list->key);
        list = list->next;
    }

    storeAppendPrintf(entry, "\n%s ntlm children %d\n",
                      name, authenticateChildren);
    storeAppendPrintf(entry, "%s %s keep_alive %s\n", name, "ntlm", keep_alive ? "on" : "off");

}

AuthNTLMConfig::AuthNTLMConfig() : authenticateChildren(5), keep_alive(0)
{ }

void
AuthNTLMConfig::parse(AuthConfig * scheme, int n_configured, char *param_str)
{
    if (strcasecmp(param_str, "program") == 0) {
        if (authenticate)
            wordlistDestroy(&authenticate);

        parse_wordlist(&authenticate);

        requirePathnameExists("auth_param ntlm program", authenticate->key);
    } else if (strcasecmp(param_str, "children") == 0) {
        parse_int(&authenticateChildren);
    } else if (strcasecmp(param_str, "keep_alive") == 0) {
        parse_onoff(&keep_alive);
    } else {
        debugs(29, 0, "AuthNTLMConfig::parse: unrecognised ntlm auth scheme parameter '" << param_str << "'");
    }
}

const char *
AuthNTLMConfig::type() const
{
    return ntlmScheme::GetInstance().type();
}

/* Initialize helpers and the like for this auth scheme. Called AFTER parsing the
 * config file */
void
AuthNTLMConfig::init(AuthConfig * scheme)
{
    if (authenticate) {

        authntlm_initialised = 1;

        if (ntlmauthenticators == NULL)
            ntlmauthenticators = helperStatefulCreate("ntlmauthenticator");

        if (!proxy_auth_cache)
            proxy_auth_cache = hash_create((HASHCMP *) strcmp, 7921, hash_string);

        assert(proxy_auth_cache);

        ntlmauthenticators->cmdline = authenticate;

        ntlmauthenticators->n_to_start = authenticateChildren;

        ntlmauthenticators->ipc_type = IPC_STREAM;

        helperStatefulOpenServers(ntlmauthenticators);

        CBDATA_INIT_TYPE(authenticateStateData);
    }
}

void
AuthNTLMConfig::registerWithCacheManager(void)
{
    CacheManager::GetInstance()->
    registerAction("ntlmauthenticator",
                   "NTLM User Authenticator Stats",
                   authenticateNTLMStats, 0, 1);
}

bool
AuthNTLMConfig::active() const
{
    return authntlm_initialised == 1;
}

bool
AuthNTLMConfig::configured() const
{
    if ((authenticate != NULL) && (authenticateChildren != 0)) {
        debugs(29, 9, "AuthNTLMConfig::configured: returning configured");
        return true;
    }

    debugs(29, 9, "AuthNTLMConfig::configured: returning unconfigured");
    return false;
}

/* NTLM Scheme */
/* See AuthUserRequest.cc::authenticateDirection for return values */
int
AuthNTLMUserRequest::module_direction()
{
    /* null auth_user is checked for by authenticateDirection */

    if (waiting || client_blob)
        return -1; /* need helper response to continue */

    switch (auth_state) {

        /* no progress at all. */

    case AUTHENTICATE_STATE_NONE:
        debugs(29, 1, "AuthNTLMUserRequest::direction: called before NTLM Authenticate for request " << this << "!. Report a bug to squid-dev.");
        return -2; /* error */

    case AUTHENTICATE_STATE_FAILED:
        return -2; /* error */


    case AUTHENTICATE_STATE_IN_PROGRESS:
        assert(server_blob);
        return 1; /* send to client */

    case AUTHENTICATE_STATE_DONE:
        return 0; /* do nothing */

    case AUTHENTICATE_STATE_INITIAL:
        debugs(29, 1, "AuthNTLMUserRequest::direction: Unexpected AUTHENTICATE_STATE_INITIAL");
        return -2;
    }

    return -2;
}

void
AuthNTLMConfig::fixHeader(AuthUserRequest *auth_user_request, HttpReply *rep, http_hdr_type hdrType, HttpRequest * request)
{
    AuthNTLMUserRequest *ntlm_request;

    if (!authenticate)
        return;

    /* Need keep-alive */
    if (!request->flags.proxy_keepalive && request->flags.must_keepalive)
        return;

    /* New request, no user details */
    if (auth_user_request == NULL) {
        debugs(29, 9, "AuthNTLMConfig::fixHeader: Sending type:" << hdrType << " header: 'NTLM'");
        httpHeaderPutStrf(&rep->header, hdrType, "NTLM");

        if (!keep_alive) {
            /* drop the connection */
            request->flags.proxy_keepalive = 0;
        }
    } else {
        ntlm_request = dynamic_cast<AuthNTLMUserRequest *>(auth_user_request);

        assert(ntlm_request != NULL);

        switch (ntlm_request->auth_state) {

        case AUTHENTICATE_STATE_FAILED:
            /* here it makes sense to drop the connection, as auth is
             * tied to it, even if MAYBE the client could handle it - Kinkie */
            request->flags.proxy_keepalive = 0;
            /* fall through */

        case AUTHENTICATE_STATE_DONE:
            /* Special case: authentication finished OK but disallowed by ACL.
             * Need to start over to give the client another chance.
             */
            /* fall through */

        case AUTHENTICATE_STATE_NONE:
            /* semantic change: do not drop the connection.
             * 2.5 implementation used to keep it open - Kinkie */
            debugs(29, 9, "AuthNTLMConfig::fixHeader: Sending type:" << hdrType << " header: 'NTLM'");
            httpHeaderPutStrf(&rep->header, hdrType, "NTLM");
            break;

        case AUTHENTICATE_STATE_IN_PROGRESS:
            /* we're waiting for a response from the client. Pass it the blob */
            debugs(29, 9, "AuthNTLMConfig::fixHeader: Sending type:" << hdrType << " header: 'NTLM " << ntlm_request->server_blob << "'");
            httpHeaderPutStrf(&rep->header, hdrType, "NTLM %s", ntlm_request->server_blob);
            safe_free(ntlm_request->server_blob);
            break;


        default:
            debugs(29, 0, "AuthNTLMConfig::fixHeader: state " << ntlm_request->auth_state << ".");
            fatal("unexpected state in AuthenticateNTLMFixErrorHeader.\n");
        }
    }
}

NTLMUser::~NTLMUser()
{
    debugs(29, 5, "NTLMUser::~NTLMUser: doing nothing to clearNTLM scheme data for '" << this << "'");
}

static void
authenticateNTLMHandleReply(void *data, void *lastserver, char *reply)
{
    authenticateStateData *r = static_cast<authenticateStateData *>(data);

    int valid;
    char *blob;

    AuthUserRequest *auth_user_request;
    AuthUser *auth_user;
    NTLMUser *ntlm_user;
    AuthNTLMUserRequest *ntlm_request;

    debugs(29, 8, "authenticateNTLMHandleReply: helper: '" << lastserver << "' sent us '" << (reply ? reply : "<NULL>") << "'");
    valid = cbdataReferenceValid(r->data);

    if (!valid) {
        debugs(29, 1, "authenticateNTLMHandleReply: invalid callback data. helper '" << lastserver << "'.");
        cbdataReferenceDone(r->data);
        authenticateStateFree(r);
        return;
    }

    if (!reply) {
        debugs(29, 1, "authenticateNTLMHandleReply: Helper '" << lastserver << "' crashed!.");
        reply = (char *)"BH Internal error";
    }

    auth_user_request = r->auth_user_request;
    assert(auth_user_request != NULL);
    ntlm_request = dynamic_cast<AuthNTLMUserRequest *>(auth_user_request);

    assert(ntlm_request != NULL);
    assert(ntlm_request->waiting);
    ntlm_request->waiting = 0;
    safe_free(ntlm_request->client_blob);

    auth_user = ntlm_request->user();
    assert(auth_user != NULL);
    assert(auth_user->auth_type == AUTH_NTLM);
    ntlm_user = dynamic_cast<ntlm_user_t *>(auth_user_request->user());

    assert(ntlm_user != NULL);

    if (ntlm_request->authserver == NULL)
        ntlm_request->authserver = static_cast<helper_stateful_server*>(lastserver);
    else
        assert(ntlm_request->authserver == lastserver);

    /* seperate out the useful data */
    blob = strchr(reply, ' ');

    if (blob)
        blob++;

    if (strncasecmp(reply, "TT ", 3) == 0) {
        /* we have been given a blob to send to the client */
        safe_free(ntlm_request->server_blob);
        ntlm_request->request->flags.must_keepalive = 1;
        if (ntlm_request->request->flags.proxy_keepalive) {
            ntlm_request->server_blob = xstrdup(blob);
            ntlm_request->auth_state = AUTHENTICATE_STATE_IN_PROGRESS;
            auth_user_request->denyMessage("Authentication in progress");
            debugs(29, 4, "authenticateNTLMHandleReply: Need to challenge the client with a server blob '" << blob << "'");
        } else {
            ntlm_request->auth_state = AUTHENTICATE_STATE_FAILED;
            auth_user_request->denyMessage("NTLM authentication requires a persistent connection");
        }
    } else if (strncasecmp(reply, "AF ", 3) == 0) {
        /* we're finished, release the helper */
        ntlm_user->username(blob);
        auth_user_request->denyMessage("Login successful");
        safe_free(ntlm_request->server_blob);

        debugs(29, 4, "authenticateNTLMHandleReply: Successfully validated user via NTLM. Username '" << blob << "'");
        /* connection is authenticated */
        debugs(29, 4, "AuthNTLMUserRequest::authenticate: authenticated user " << ntlm_user->username());
        /* see if this is an existing user with a different proxy_auth
         * string */
        AuthUserHashPointer *usernamehash = static_cast<AuthUserHashPointer *>(hash_lookup(proxy_auth_username_cache, ntlm_user->username()));
        AuthUser *local_auth_user = ntlm_request->user();
        while (usernamehash && (usernamehash->user()->auth_type != AUTH_NTLM || strcmp(usernamehash->user()->username(), ntlm_user->username()) != 0))
            usernamehash = static_cast<AuthUserHashPointer *>(usernamehash->next);
        if (usernamehash) {
            /* we can't seamlessly recheck the username due to the
             * challenge-response nature of the protocol.
             * Just free the temporary auth_user */
            usernamehash->user()->absorb(local_auth_user);
            //authenticateAuthUserMerge(local_auth_user, usernamehash->user());
            local_auth_user = usernamehash->user();
            ntlm_request->_auth_user = local_auth_user;
        } else {
            /* store user in hash's */
            local_auth_user->addToNameCache();
            // authenticateUserNameCacheAdd(local_auth_user);
        }
        /* set these to now because this is either a new login from an
         * existing user or a new user */
        local_auth_user->expiretime = current_time.tv_sec;
        ntlm_request->releaseAuthServer();
        ntlm_request->auth_state = AUTHENTICATE_STATE_DONE;
    } else if (strncasecmp(reply, "NA ", 3) == 0) {
        /* authentication failure (wrong password, etc.) */
        auth_user_request->denyMessage(blob);
        ntlm_request->auth_state = AUTHENTICATE_STATE_FAILED;
        safe_free(ntlm_request->server_blob);
        ntlm_request->releaseAuthServer();
        debugs(29, 4, "authenticateNTLMHandleReply: Failed validating user via NTLM. Error returned '" << blob << "'");
    } else if (strncasecmp(reply, "BH ", 3) == 0) {
        /* TODO kick off a refresh process. This can occur after a YR or after
         * a KK. If after a YR release the helper and resubmit the request via
         * Authenticate NTLM start.
         * If after a KK deny the user's request w/ 407 and mark the helper as
         * Needing YR. */
        auth_user_request->denyMessage(blob);
        ntlm_request->auth_state = AUTHENTICATE_STATE_FAILED;
        safe_free(ntlm_request->server_blob);
        ntlm_request->releaseAuthServer();
        debugs(29, 1, "authenticateNTLMHandleReply: Error validating user via NTLM. Error returned '" << reply << "'");
    } else {
        /* protocol error */
        fatalf("authenticateNTLMHandleReply: *** Unsupported helper response ***, '%s'\n", reply);
    }

    if (ntlm_request->request) {
        HTTPMSGUNLOCK(ntlm_request->request);
        ntlm_request->request = NULL;
    }
    r->handler(r->data, NULL);
    cbdataReferenceDone(r->data);
    authenticateStateFree(r);
}

static void
authenticateNTLMStats(StoreEntry * sentry)
{
    helperStatefulStats(sentry, ntlmauthenticators, "NTLM Authenticator Statistics");
}


/* send the initial data to a stateful ntlm authenticator module */
void
AuthNTLMUserRequest::module_start(RH * handler, void *data)
{
    authenticateStateData *r = NULL;
    static char buf[8192];
    AuthUser *auth_user = user();

    assert(data);
    assert(handler);
    assert(auth_user);
    assert(auth_user->auth_type == AUTH_NTLM);

    debugs(29, 8, "AuthNTLMUserRequest::module_start: auth state is '" << auth_state << "'");

    if (ntlmConfig.authenticate == NULL) {
        debugs(29, 0, "AuthNTLMUserRequest::module_start: no NTLM program specified.");
        handler(data, NULL);
        return;
    }

    r = cbdataAlloc(authenticateStateData);
    r->handler = handler;
    r->data = cbdataReference(data);
    r->auth_user_request = this;
    AUTHUSERREQUESTLOCK(r->auth_user_request, "r");

    if (auth_state == AUTHENTICATE_STATE_INITIAL) {
        snprintf(buf, 8192, "YR %s\n", client_blob); //CHECKME: can ever client_blob be 0 here?
    } else {
        snprintf(buf, 8192, "KK %s\n", client_blob);
    }

    waiting = 1;

    safe_free(client_blob);
    helperStatefulSubmit(ntlmauthenticators, buf, authenticateNTLMHandleReply, r, authserver);
}

/**
 * Atomic action: properly release the NTLM auth helpers which may have been reserved
 * for this request connections use.
 */
void
AuthNTLMUserRequest::releaseAuthServer()
{
    if (authserver) {
        debugs(29, 6, HERE << "releasing NTLM auth server '" << authserver << "'");
        helperStatefulReleaseServer(authserver);
        authserver = NULL;
    } else
        debugs(29, 6, HERE << "No NTLM auth server to release.");
}

/* clear any connection related authentication details */
void
AuthNTLMUserRequest::onConnectionClose(ConnStateData *conn)
{
    assert(conn != NULL);

    debugs(29, 8, "AuthNTLMUserRequest::onConnectionClose: closing connection '" << conn << "' (this is '" << this << "')");

    if (conn->auth_user_request == NULL) {
        debugs(29, 8, "AuthNTLMUserRequest::onConnectionClose: no auth_user_request");
        return;
    }

    // unlock / un-reserve the helpers
    releaseAuthServer();

    /* unlock the connection based lock */
    debugs(29, 9, "AuthNTLMUserRequest::onConnectionClose: Unlocking auth_user from the connection '" << conn << "'.");

    AUTHUSERREQUESTUNLOCK(conn->auth_user_request, "conn");
}

/*
 * Decode a NTLM [Proxy-]Auth string, placing the results in the passed
 * Auth_user structure.
 */
AuthUserRequest *
AuthNTLMConfig::decode(char const *proxy_auth)
{
    NTLMUser *newUser = new NTLMUser(&ntlmConfig);
    AuthNTLMUserRequest *auth_user_request = new AuthNTLMUserRequest ();
    assert(auth_user_request->user() == NULL);
    auth_user_request->user(newUser);
    auth_user_request->user()->auth_type = AUTH_NTLM;
    auth_user_request->user()->addRequest(auth_user_request);

    /* all we have to do is identify that it's NTLM - the helper does the rest */
    debugs(29, 9, "AuthNTLMConfig::decode: NTLM authentication");
    return auth_user_request;
}

int
AuthNTLMUserRequest::authenticated() const
{
    if (auth_state == AUTHENTICATE_STATE_DONE) {
        debugs(29, 9, "AuthNTLMUserRequest::authenticated: user authenticated.");
        return 1;
    }

    debugs(29, 9, "AuthNTLMUserRequest::authenticated: user not fully authenticated.");

    return 0;
}

void
AuthNTLMUserRequest::authenticate(HttpRequest * aRequest, ConnStateData * conn, http_hdr_type type)
{
    const char *proxy_auth, *blob;

    /* TODO: rename this!! */
    AuthUser *local_auth_user;

    local_auth_user = user();
    assert(local_auth_user);
    assert(local_auth_user->auth_type == AUTH_NTLM);
    assert (this);

    /* Check that we are in the client side, where we can generate
     * auth challenges */

    if (conn == NULL || !cbdataReferenceValid(conn)) {
        auth_state = AUTHENTICATE_STATE_FAILED;
        debugs(29, 1, "AuthNTLMUserRequest::authenticate: attempt to perform authentication without a connection!");
        return;
    }

    if (waiting) {
        debugs(29, 1, "AuthNTLMUserRequest::authenticate: waiting for helper reply!");
        return;
    }

    if (server_blob) {
        debugs(29, 2, "AuthNTLMUserRequest::authenticate: need to challenge client '" << server_blob << "'!");
        return;
    }

    /* get header */
    proxy_auth = aRequest->header.getStr(type);

    /* locate second word */
    blob = proxy_auth;

    /* if proxy_auth is actually NULL, we'd better not manipulate it. */
    if (blob) {
        while (xisspace(*blob) && *blob)
            blob++;

        while (!xisspace(*blob) && *blob)
            blob++;

        while (xisspace(*blob) && *blob)
            blob++;
    }

    switch (auth_state) {

    case AUTHENTICATE_STATE_NONE:
        /* we've received a ntlm request. pass to a helper */
        debugs(29, 9, "AuthNTLMUserRequest::authenticate: auth state ntlm none. Received blob: '" << proxy_auth << "'");
        auth_state = AUTHENTICATE_STATE_INITIAL;
        safe_free(client_blob);
        client_blob=xstrdup(blob);
        conn->auth_type = AUTH_NTLM;
        assert(conn->auth_user_request == NULL);
        conn->auth_user_request = this;
        AUTHUSERREQUESTLOCK(conn->auth_user_request, "conn");
        request = aRequest;
        HTTPMSGLOCK(request);
        return;

        break;

    case AUTHENTICATE_STATE_INITIAL:
        debugs(29, 1, "AuthNTLMUserRequest::authenticate: need to ask helper");

        return;

        break;


    case AUTHENTICATE_STATE_IN_PROGRESS:
        /* we should have received a blob from the client. Hand it off to
         * some helper */
        safe_free(client_blob);

        client_blob = xstrdup (blob);

        if (request)
            HTTPMSGUNLOCK(request);
        request = aRequest;
        HTTPMSGLOCK(request);
        return;

        break;

    case AUTHENTICATE_STATE_DONE:
        fatal("AuthNTLMUserRequest::authenticate: unexpect auth state DONE! Report a bug to the squid developers.\n");

        break;

    case AUTHENTICATE_STATE_FAILED:
        /* we've failed somewhere in authentication */
        debugs(29, 9, "AuthNTLMUserRequest::authenticate: auth state ntlm failed. " << proxy_auth);

        return;

        break;
    }

    return;
}

AuthNTLMUserRequest::AuthNTLMUserRequest() :
        /*conn(NULL),*/ auth_state(AUTHENTICATE_STATE_NONE)
{
    waiting=0;
    client_blob=0;
    server_blob=0;
    authserver=NULL;
    request = NULL;
}

AuthNTLMUserRequest::~AuthNTLMUserRequest()
{
    safe_free(server_blob);
    safe_free(client_blob);

    releaseAuthServer();

    if (request) {
        HTTPMSGUNLOCK(request);
        request = NULL;
    }
}

void
NTLMUser::deleteSelf() const
{
    delete this;
}

NTLMUser::NTLMUser (AuthConfig *aConfig) : AuthUser (aConfig)
{
    proxy_auth_list.head = proxy_auth_list.tail = NULL;
}

AuthConfig *
ntlmScheme::createConfig()
{
    return &ntlmConfig;
}

const char *
AuthNTLMUserRequest::connLastHeader()
{
    return NULL;
}

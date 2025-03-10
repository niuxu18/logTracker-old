    plaintext = ap_lua_ssl_is_https(r->connection) ? 0 : 1;

    
    mask_bytes = apr_pcalloc(r->pool, 4);
    sock = ap_get_conn_socket(r->connection);

    while (do_read) {
        do_read = 0;
        /* Get opcode and FIN bit */
        if (plaintext) {
            rv = apr_socket_recv(sock, &byte, &len);
        }
        else {
            rv = lua_websocket_readbytes(r->connection, &byte, 1);
        }
        if (rv == APR_SUCCESS) {
            unsigned char ubyte, fin, opcode, mask, payload;
            ubyte = (unsigned char)byte;
            /* fin bit is the first bit */
            fin = ubyte >> (CHAR_BIT - 1);
            /* opcode is the last four bits (there's 3 reserved bits we don't care about) */
            opcode = ubyte & 0xf;

            /* Get the payload length and mask bit */
            if (plaintext) {
                rv = apr_socket_recv(sock, &byte, &len);
            }
            else {
                rv = lua_websocket_readbytes(r->connection, &byte, 1);
            }
            if (rv == APR_SUCCESS) {
                ubyte = (unsigned char)byte;
                /* Mask is the first bit */
                mask = ubyte >> (CHAR_BIT - 1);
                /* Payload is the last 7 bits */
                payload = ubyte & 0x7f;
                plen = payload;

                /* Extended payload? */
                if (payload == 126) {
                    len = 2;
                    if (plaintext) {
                        /* XXX: apr_socket_recv does not receive len bits, only up to len bits! */
                        rv = apr_socket_recv(sock, (char*) &payload_short, &len);
                    }
                    else {
                        rv = lua_websocket_readbytes(r->connection, 
                                (char*) &payload_short, 2);
                    }
                    payload_short = ntohs(payload_short);

                    if (rv == APR_SUCCESS) {
                        plen = payload_short;
                    }
                    else {
                        return 0;
                    }
                }
                /* Super duper extended payload? */
                if (payload == 127) {
                    len = 8;
                    if (plaintext) {
                        rv = apr_socket_recv(sock, (char*) &payload_long, &len);
                    }
                    else {
                        rv = lua_websocket_readbytes(r->connection, 
                                (char*) &payload_long, 8);
                    }
                    if (rv == APR_SUCCESS) {
                        plen = ap_ntoh64(&payload_long);
                    }
                    else {
                        return 0;
                    }
                }
                ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, APLOGNO(03210)
                              "Websocket: Reading %" APR_SIZE_T_FMT " (%s) bytes, masking is %s. %s", 
                              plen,
                              (payload >= 126) ? "extra payload" : "no extra payload", 
                              mask ? "on" : "off", 
                              fin ? "This is a final frame" : "more to follow");
                if (mask) {
                    len = 4;
                    if (plaintext) {
                        rv = apr_socket_recv(sock, (char*) mask_bytes, &len);
                    }
                    else {
                        rv = lua_websocket_readbytes(r->connection, 
                                (char*) mask_bytes, 4);
                    }
                    if (rv != APR_SUCCESS) {
                        return 0;
                    }
                }
                if (plen < (HUGE_STRING_LEN*1024) && plen > 0) {
                    apr_size_t remaining = plen;
                    apr_size_t received;
                    apr_off_t at = 0;
                    char *buffer = apr_palloc(r->pool, plen+1);
                    buffer[plen] = 0;

                    if (plaintext) {
                        while (remaining > 0) {
                            received = remaining;
                            rv = apr_socket_recv(sock, buffer+at, &received);
                            if (received > 0 ) {
                                remaining -= received;
                                at += received;
                            }
                        }
                        ap_log_rerror(APLOG_MARK, APLOG_TRACE1, 0, r, 
                                "Websocket: Frame contained %" APR_OFF_T_FMT " bytes, pushed to Lua stack", 
                                at);
                    }
                    else {
                        rv = lua_websocket_readbytes(r->connection, buffer, 
                                remaining);
                        ap_log_rerror(APLOG_MARK, APLOG_TRACE1, 0, r, 
                                "Websocket: SSL Frame contained %" APR_SIZE_T_FMT " bytes, "\
                                "pushed to Lua stack", 
                                remaining);
                    }
                    if (mask) {
                        for (n = 0; n < plen; n++) {
                            buffer[n] ^= mask_bytes[n%4];
                        }
                    }

                    lua_pushlstring(L, buffer, (size_t) plen); /* push to stack */
                    lua_pushboolean(L, fin); /* push FIN bit to stack as boolean */
                    return 2;
                }


                /* Decide if we need to react to the opcode or not */
                if (opcode == 0x09) { /* ping */
                    char frame[2];
                    plen = 2;
                    frame[0] = 0x8A;
                    frame[1] = 0;
                    apr_socket_send(sock, frame, &plen); /* Pong! */
                    do_read = 1;
                }
            }
        }
    }
    return 0;
}


static int lua_websocket_write(lua_State *L) 
{

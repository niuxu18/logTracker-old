static int lua_ivm_get(lua_State *L) 
{
    const char *key, *raw_key;
    lua_ivm_object *object = NULL;
    request_rec *r = ap_lua_check_request_rec(L, 1);
    key = luaL_checkstring(L, 2);
    raw_key = apr_pstrcat(r->pool, "lua_ivm_", key, NULL);
    apr_thread_mutex_lock(lua_ivm_mutex);
    apr_pool_userdata_get((void **)&object, raw_key, r->server->process->pool);
    if (object) {
        if (object->type == LUA_TBOOLEAN) lua_pushboolean(L, (int) object->number);
        else if (object->type == LUA_TNUMBER) lua_pushnumber(L, object->number);
        else if (object->type == LUA_TSTRING) lua_pushlstring(L, object->vb.buf, object->size);
        apr_thread_mutex_unlock(lua_ivm_mutex);
        return 1;
    }
    else {
        apr_thread_mutex_unlock(lua_ivm_mutex);
        return 0;
    }
}
static int lua_translate_name_harness_last(request_rec *r)
{
    return lua_request_rec_hook_harness(r, "translate_name", AP_LUA_HOOK_LAST);
}
const headerMangler *
HeaderManglers::find(const HttpHeaderEntry &e) const
{
    // a known header with a configured ACL list
    if (e.id != HDR_OTHER && 0 <= e.id && e.id < HDR_ENUM_END &&
            known[e.id].access_list)
        return &known[e.id];

    // a custom header
    if (e.id == HDR_OTHER) {
        // does it have an ACL list configured?
        // Optimize: use a name type that we do not need to convert to here
        const ManglersByName::const_iterator i = custom.find(e.name.termedBuf());
        if (i != custom.end())
            return &i->second;
    }

    // Next-to-last resort: "Other" rules match any custom header
    if (e.id == HDR_OTHER && known[HDR_OTHER].access_list)
        return &known[HDR_OTHER];

    // Last resort: "All" rules match any header
    if (all.access_list)
        return &all;

    return NULL;
}
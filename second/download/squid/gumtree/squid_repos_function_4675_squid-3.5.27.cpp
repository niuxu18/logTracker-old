SBufList
ACLARP::dump() const
{
    SBufList sl;
    for (AclArpData_t::iterator i = aclArpData.begin(); i != aclArpData.end(); ++i) {
        char buf[48];
        i->encode(buf,48);
        sl.push_back(SBuf(buf));
    }
    return sl;
}
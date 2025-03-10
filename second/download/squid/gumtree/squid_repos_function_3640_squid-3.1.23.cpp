bool IpAddress::IsIPv4() const
{
    return IsAnyAddr() || IsNoAddr() || IN6_IS_ADDR_V4MAPPED( &m_SocketAddr.sin6_addr );
}
(addr->sa_family == AF_INET6) {
    memcpy(&req->storage,
           addr,
           sizeof(struct sockaddr_in6));
  }
{
    /* convert addrinfoW to addrinfo */
    /* first calculate required length */
    addrinfow_ptr = req->addrinfow;
    while (addrinfow_ptr != NULL) {
      addrinfo_len += addrinfo_struct_len +
          ALIGNED_SIZE(addrinfow_ptr->ai_addrlen);
      if (addrinfow_ptr->ai_canonname != NULL) {
        name_len = WideCharToMultiByte(CP_UTF8,
                                       0,
                                       addrinfow_ptr->ai_canonname,
                                       -1,
                                       NULL,
                                       0,
                                       NULL,
                                       NULL);
        if (name_len == 0) {
          req->retcode = uv_translate_sys_error(GetLastError());
          goto complete;
        }
        addrinfo_len += ALIGNED_SIZE(name_len);
      }
      addrinfow_ptr = addrinfow_ptr->ai_next;
    }

    /* allocate memory for addrinfo results */
    alloc_ptr = (char*)uv__malloc(addrinfo_len);

    /* do conversions */
    if (alloc_ptr != NULL) {
      cur_ptr = alloc_ptr;
      addrinfow_ptr = req->addrinfow;

      while (addrinfow_ptr != NULL) {
        /* copy addrinfo struct data */
        assert(cur_ptr + addrinfo_struct_len <= alloc_ptr + addrinfo_len);
        addrinfo_ptr = (struct addrinfo*)cur_ptr;
        addrinfo_ptr->ai_family = addrinfow_ptr->ai_family;
        addrinfo_ptr->ai_socktype = addrinfow_ptr->ai_socktype;
        addrinfo_ptr->ai_protocol = addrinfow_ptr->ai_protocol;
        addrinfo_ptr->ai_flags = addrinfow_ptr->ai_flags;
        addrinfo_ptr->ai_addrlen = addrinfow_ptr->ai_addrlen;
        addrinfo_ptr->ai_canonname = NULL;
        addrinfo_ptr->ai_addr = NULL;
        addrinfo_ptr->ai_next = NULL;

        cur_ptr += addrinfo_struct_len;

        /* copy sockaddr */
        if (addrinfo_ptr->ai_addrlen > 0) {
          assert(cur_ptr + addrinfo_ptr->ai_addrlen <=
                 alloc_ptr + addrinfo_len);
          memcpy(cur_ptr, addrinfow_ptr->ai_addr, addrinfo_ptr->ai_addrlen);
          addrinfo_ptr->ai_addr = (struct sockaddr*)cur_ptr;
          cur_ptr += ALIGNED_SIZE(addrinfo_ptr->ai_addrlen);
        }

        /* convert canonical name to UTF-8 */
        if (addrinfow_ptr->ai_canonname != NULL) {
          name_len = WideCharToMultiByte(CP_UTF8,
                                         0,
                                         addrinfow_ptr->ai_canonname,
                                         -1,
                                         NULL,
                                         0,
                                         NULL,
                                         NULL);
          assert(name_len > 0);
          assert(cur_ptr + name_len <= alloc_ptr + addrinfo_len);
          name_len = WideCharToMultiByte(CP_UTF8,
                                         0,
                                         addrinfow_ptr->ai_canonname,
                                         -1,
                                         cur_ptr,
                                         name_len,
                                         NULL,
                                         NULL);
          assert(name_len > 0);
          addrinfo_ptr->ai_canonname = cur_ptr;
          cur_ptr += ALIGNED_SIZE(name_len);
        }
        assert(cur_ptr <= alloc_ptr + addrinfo_len);

        /* set next ptr */
        addrinfow_ptr = addrinfow_ptr->ai_next;
        if (addrinfow_ptr != NULL) {
          addrinfo_ptr->ai_next = (struct addrinfo*)cur_ptr;
        }
      }
      req->addrinfo = (struct addrinfo*)alloc_ptr;
    } else {
      req->retcode = UV_EAI_MEMORY;
    }
  }
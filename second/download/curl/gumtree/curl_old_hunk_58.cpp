        localaddr->sin_addr = natAddr;
      }
    }
  }
#endif

  if(Curl_base64_encode(adat.dat, adat.length, &p) < 0) {
    Curl_failf(data, "Out of memory base64-encoding");
    return AUTH_CONTINUE;
  }

  result = Curl_ftpsendf(conn, "ADAT %s", p);


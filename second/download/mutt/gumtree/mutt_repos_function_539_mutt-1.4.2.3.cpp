static int check_certificate_by_signer (X509 *peercert)
{
  X509_STORE_CTX xsc;
  X509_STORE *ctx;
  int pass = 0;

  ctx = X509_STORE_new ();
  if (ctx == NULL) return 0;

  if (option (OPTSSLSYSTEMCERTS))
  {
    if (X509_STORE_set_default_paths (ctx))
      pass++;
    else
      dprint (2, (debugfile, "X509_STORE_set_default_paths failed\n"));
  }

  if (X509_STORE_load_locations (ctx, SslCertFile, NULL))
    pass++;
  else
    dprint (2, (debugfile, "X509_STORE_load_locations_failed\n"));

  if (pass == 0)
  {
    /* nothing to do */
    X509_STORE_free (ctx);
    return 0;
  }

  X509_STORE_CTX_init (&xsc, ctx, peercert, NULL);

  pass = (X509_verify_cert (&xsc) > 0);
#ifdef DEBUG
  if (! pass)
  {
    char buf[SHORT_STRING];
    int err;

    err = X509_STORE_CTX_get_error (&xsc);
    snprintf (buf, sizeof (buf), "%s (%d)", 
	X509_verify_cert_error_string(err), err);
    dprint (2, (debugfile, "X509_verify_cert: %s\n", buf));
  }
#endif
  X509_STORE_CTX_cleanup (&xsc);
  X509_STORE_free (ctx);

  return pass;
}
static int
find_module (handle, dir, libdir, dlname, old_name, installed)
     lt_dlhandle *handle;
     const char *dir;
     const char *libdir;
     const char *dlname;
     const char *old_name;
     int installed;
{
  /* Try to open the old library first; if it was dlpreopened,
     we want the preopened version of it, even if a dlopenable
     module is available.  */
  if (old_name && tryall_dlopen (handle, old_name) == 0)
    {
      return 0;
    }

  /* Try to open the dynamic library.  */
  if (dlname)
    {
      /* try to open the installed module */
      if (installed && libdir)
	{
	  if (tryall_dlopen_module (handle,
				    (const char *) 0, libdir, dlname) == 0)
	    return 0;
	}

      /* try to open the not-installed module */
      if (!installed)
	{
	  if (tryall_dlopen_module (handle, dir, objdir, dlname) == 0)
	    return 0;
	}

      /* maybe it was moved to another directory */
      {
	  if (dir && (tryall_dlopen_module (handle,
				    (const char *) 0, dir, dlname) == 0))
	    return 0;
      }
    }

  return 1;
}
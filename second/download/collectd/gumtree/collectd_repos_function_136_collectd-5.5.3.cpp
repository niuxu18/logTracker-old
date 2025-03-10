lt_dlvtable *
get_vtable (lt_user_data loader_data)
{
  if (!vtable)
    {
      vtable = (lt_dlvtable *) lt__zalloc (sizeof *vtable);
    }

  if (vtable && !vtable->name)
    {
      vtable->name		= "lt_preopen";
      vtable->sym_prefix	= 0;
      vtable->module_open	= vm_open;
      vtable->module_close	= vm_close;
      vtable->find_sym		= vm_sym;
      vtable->dlloader_init	= vl_init;
      vtable->dlloader_exit	= vl_exit;
      vtable->dlloader_data	= loader_data;
      vtable->priority		= LT_DLLOADER_PREPEND;
    }

  if (vtable && (vtable->dlloader_data != loader_data))
    {
      LT__SETERROR (INIT_LOADER);
      return 0;
    }

  return vtable;
}
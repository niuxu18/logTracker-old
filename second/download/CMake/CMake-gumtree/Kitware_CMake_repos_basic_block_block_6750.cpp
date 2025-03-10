{
  PK11SlotInfo *slot;
  PK11GenericObject *obj;
  CK_BBOOL cktrue = CK_TRUE;
  CK_BBOOL ckfalse = CK_FALSE;
  CK_ATTRIBUTE attrs[/* max count of attributes */ 4];
  int attr_cnt = 0;
  CURLcode result = (cacert)
    ? CURLE_SSL_CACERT_BADFILE
    : CURLE_SSL_CERTPROBLEM;

  const int slot_id = (cacert) ? 0 : 1;
  char *slot_name = aprintf("PEM Token #%d", slot_id);
  if(!slot_name)
    return CURLE_OUT_OF_MEMORY;

  slot = nss_find_slot_by_name(slot_name);
  free(slot_name);
  if(!slot)
    return result;

  PK11_SETATTRS(attrs, attr_cnt, CKA_CLASS, &obj_class, sizeof(obj_class));
  PK11_SETATTRS(attrs, attr_cnt, CKA_TOKEN, &cktrue, sizeof(CK_BBOOL));
  PK11_SETATTRS(attrs, attr_cnt, CKA_LABEL, (unsigned char *)filename,
                (CK_ULONG)strlen(filename) + 1);

  if(CKO_CERTIFICATE == obj_class) {
    CK_BBOOL *pval = (cacert) ? (&cktrue) : (&ckfalse);
    PK11_SETATTRS(attrs, attr_cnt, CKA_TRUST, pval, sizeof(*pval));
  }

  obj = PK11_CreateGenericObject(slot, attrs, attr_cnt, PR_FALSE);
  PK11_FreeSlot(slot);
  if(!obj)
    return result;

  if(insert_wrapped_ptr(&ssl->obj_list, obj) != CURLE_OK) {
    PK11_DestroyGenericObject(obj);
    return CURLE_OUT_OF_MEMORY;
  }

  if(!cacert && CKO_CERTIFICATE == obj_class)
    /* store reference to a client certificate */
    ssl->obj_clicert = obj;

  return CURLE_OK;
}
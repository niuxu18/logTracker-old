static void pgp_entry (char *s, size_t l, MUTTMENU * menu, int num)
{
  pgp_uid_t **KeyTable = (pgp_uid_t **) menu->data;
  pgp_entry_t entry;
  
  entry.uid = KeyTable[num];
  entry.num = num + 1;

  mutt_FormatString (s, l, NONULL (PgpEntryFormat), pgp_entry_fmt, 
		     (unsigned long) &entry, M_FORMAT_ARROWCURSOR);
}
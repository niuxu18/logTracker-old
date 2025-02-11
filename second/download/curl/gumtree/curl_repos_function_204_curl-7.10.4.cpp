char *Curl_FormBoundary(void)
{
  char *retstring;
  static int randomizer=0; /* this is just so that two boundaries within
			      the same form won't be identical */
  int i;

  static char table62[]=
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

  retstring = (char *)malloc(BOUNDARY_LENGTH);

  if(!retstring)
    return NULL; /* failed */

  srand(time(NULL)+randomizer++); /* seed */

  strcpy(retstring, "curl"); /* bonus commercials 8*) */

  for(i=4; i<(BOUNDARY_LENGTH-1); i++) {
    retstring[i] = table62[rand()%62];
  }
  retstring[BOUNDARY_LENGTH-1]=0; /* zero terminate */

  return retstring;
}
static BOOL set_boolean(BOOL *pb, char *parmvalue)
{
   BOOL bRetval;

   bRetval = True;
   if (strwicmp(parmvalue, "yes") == 0 ||
       strwicmp(parmvalue, "true") == 0 ||
       strwicmp(parmvalue, "1") == 0)
      *pb = True;
   else
      if (strwicmp(parmvalue, "no") == 0 ||
          strwicmp(parmvalue, "False") == 0 ||
          strwicmp(parmvalue, "0") == 0)
         *pb = False;
      else
      {
         rprintf(FERROR, "Badly formed boolean in configuration file: \"%s\".\n",
               parmvalue);
         bRetval = False;
      }
   return (bRetval);
}
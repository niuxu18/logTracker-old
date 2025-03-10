static void emit_link(request_rec *r, const char *anchor, char column, 
                      char curkey, char curdirection, 
                      const char *colargs, int nosort)
{
    char qvalue[13];
    int reverse;

    if (!nosort) {
        reverse = ((curkey == column) && (curdirection == D_ASCENDING));
        qvalue[0] = '?';
        qvalue[1] = 'C';
        qvalue[2] = '=';
        qvalue[3] = column;
        qvalue[4] = '&';
        qvalue[5] = 'a';
        qvalue[6] = 'm';
        qvalue[7] = 'p';
        qvalue[8] = ';';
        qvalue[9] = 'O';
        qvalue[10] = '=';
        qvalue[11] = reverse ? D_DESCENDING : D_ASCENDING;
        qvalue[12] = '\0';
        ap_rvputs(r, "<a href=\"", qvalue, colargs ? colargs : "", 
                     "\">", anchor, "</a>", NULL);
    }
    else {
        ap_rputs(anchor, r);
    }
}
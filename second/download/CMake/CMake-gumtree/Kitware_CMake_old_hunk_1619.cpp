          "# This file was generated by libcurl! Edit at your own risk.\n\n",
          out);
    co = c->cookies;
     
    while(co) {
      fprintf(out,
              "%s\t" /* domain */
              "%s\t" /* field1 */
              "%s\t" /* path */
              "%s\t" /* secure */
              "%u\t" /* expires */
              "%s\t" /* name */
              "%s\n", /* value */
              co->domain?co->domain:"unknown",
              co->field1==2?"TRUE":"FALSE",
              co->path?co->path:"/",
              co->secure?"TRUE":"FALSE",
              (unsigned int)co->expires,

     const char *p;
     const char *m = text;
     assert(m);
     content->init();
 
     while ((p = strchr(m, '%'))) {
-        content->append(m, p - m);	/* copy */
+        content->append(m, p - m);  /* copy */
         const char *t = Convert(*++p, false, allowRecursion);	/* convert */
-        content->Printf("%s", t);	/* copy */
-        m = p + 1;			/* advance */
+        content->Printf("%s", t);   /* copy */
+        m = p + 1;          /* advance */
     }
 
     if (*m)
-        content->Printf("%s", m);	/* copy tail */
+        content->Printf("%s", m);   /* copy tail */
 
     content->terminate();
 
     assert((size_t)content->contentSize() == strlen(content->content()));
 
     return content;

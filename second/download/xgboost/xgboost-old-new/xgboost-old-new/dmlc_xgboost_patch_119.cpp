@@ -328,14 +328,14 @@ struct WXQSummary : public WQSummary<DType, RType> {
     }
     if (nbig >= n - 1) {
       // see what was the case
-      fprintf(stderr, "LOG: check quantile stats, nbig=%lu, n=%lu\n", nbig, n);
-      fprintf(stderr, "LOG: srcsize=%lu, maxsize=%lu, range=%g, chunk=%g\n",
-              src.size, maxsize, static_cast<double>(range),
-              static_cast<double>(chunk));      
+      utils::Printf("LOG: check quantile stats, nbig=%lu, n=%lu\n", nbig, n);
+      utils::Printf("LOG: srcsize=%lu, maxsize=%lu, range=%g, chunk=%g\n",
+                    src.size, maxsize, static_cast<double>(range),
+                    static_cast<double>(chunk));      
       for (size_t i = 0; i < src.size; ++i) {
-        printf("[%lu] rmin=%g, rmax=%g, wmin=%g, v=%g, isbig=%d\n", i,
-               src.data[i].rmin, src.data[i].rmax,  src.data[i].wmin,
-               src.data[i].value, CheckLarge(src.data[i], chunk));
+        utils::Printf("[%lu] rmin=%g, rmax=%g, wmin=%g, v=%g, isbig=%d\n", i,
+                      src.data[i].rmin, src.data[i].rmax,  src.data[i].wmin,
+                      src.data[i].value, CheckLarge(src.data[i], chunk));
       }
       utils::Assert(nbig < n - 1, "quantile: too many large chunk");
     }
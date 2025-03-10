@@ -181,8 +181,14 @@ class BoostLearnTask {
       if (!silent) printf("boosting round %d, %lu sec elapsed\n", i, elapsed);
       learner.UpdateOneIter(i, *data); 
       std::string res = learner.EvalOneIter(i, devalall, eval_data_names);
-      if (silent < 2) {
-        fprintf(stderr, "%s\n", res.c_str());
+      if (rabit::IsDistributed()){
+        if (rabit::GetRank() == 0) {
+          rabit::TrackerPrintf("%s\n", res.c_str());
+        }
+      } else {
+        if (silent < 2) {
+          fprintf(stderr, "%s\n", res.c_str());
+        }
       }
       if (save_period != 0 && (i + 1) % save_period == 0) {
         this->SaveModel(i);
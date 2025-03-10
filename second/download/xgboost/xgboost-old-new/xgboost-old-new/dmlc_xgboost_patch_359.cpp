@@ -1,10 +1,12 @@
 #include <vector>
 #include <string>
+#include <utility>
 #include <cstring>
 #include "xgboost_wrapper.h"
 #include "xgboost_R.h"
 #include "../src/utils/utils.h"
 #include "../src/utils/omp.h"
+#include "../src/utils/matrix_csr.h"
 
 using namespace xgboost;
 
@@ -21,6 +23,63 @@ extern "C" {
     UNPROTECT(1);
     return ret;
   }
+  SEXP XGDMatrixCreateFromMat_R(SEXP mat, 
+                                SEXP missing) {
+    SEXP dim = getAttrib(mat, R_DimSymbol);
+    int nrow = INTEGER(dim)[0];
+    int ncol = INTEGER(dim)[1];    
+    double *din = REAL(mat);
+    std::vector<float> data(nrow * ncol);
+    #pragma omp parallel for schedule(static)
+    for (int i = 0; i < nrow; ++i) {
+      for (int j = 0; j < ncol; ++j) {
+        data[i * ncol +j] = din[i + nrow * j];
+      }
+    }
+    void *handle = XGDMatrixCreateFromMat(&data[0], nrow, ncol, asReal(missing));
+    SEXP ret = PROTECT(R_MakeExternalPtr(handle, R_NilValue, R_NilValue));
+    R_RegisterCFinalizerEx(ret, _DMatrixFinalizer, TRUE);
+    UNPROTECT(1);
+    return ret;    
+  }
+  SEXP XGDMatrixCreateFromCSC_R(SEXP indptr,
+                                SEXP indices,
+                                SEXP data) {
+    const int *col_ptr = INTEGER(indptr);
+    const int *row_index = INTEGER(indices);
+    const double *col_data = REAL(data);
+    int ncol = length(indptr) - 1;
+    int ndata = length(data);
+    // transform into CSR format
+    std::vector<size_t> row_ptr;
+    std::vector< std::pair<unsigned, float> > csr_data;
+    utils::SparseCSRMBuilder< std::pair<unsigned,float> > builder(row_ptr, csr_data);
+    builder.InitBudget();
+    for (int i = 0; i < ncol; ++i) {
+      for (int j = col_ptr[i]; j < col_ptr[i+1]; ++j) {
+        builder.AddBudget(row_index[j]);
+      }
+    }
+    builder.InitStorage();
+    for (int i = 0; i < ncol; ++i) {
+      for (int j = col_ptr[i]; j < col_ptr[i+1]; ++j) {
+        builder.PushElem(row_index[j], std::make_pair(i, col_data[j]));
+      }
+    }
+    utils::Assert(csr_data.size() == static_cast<size_t>(ndata), "BUG CreateFromCSC");
+    std::vector<float> row_data(ndata);
+    std::vector<unsigned> col_index(ndata);
+    #pragma omp parallel for schedule(static)
+    for (int i = 0; i < ndata; ++i) {
+      col_index[i] = csr_data[i].first;
+      row_data[i] = csr_data[i].second;      
+    }
+    void *handle = XGDMatrixCreateFromCSR(&row_ptr[0], &col_index[0], &row_data[0], row_ptr.size(), ndata );
+    SEXP ret = PROTECT(R_MakeExternalPtr(handle, R_NilValue, R_NilValue));
+    R_RegisterCFinalizerEx(ret, _DMatrixFinalizer, TRUE);
+    UNPROTECT(1);
+    return ret;
+  }
   void XGDMatrixSaveBinary_R(SEXP handle, SEXP fname, SEXP silent) {
     XGDMatrixSaveBinary(R_ExternalPtrAddr(handle),
                         CHAR(asChar(fname)), asInteger(silent));
@@ -142,7 +201,7 @@ extern "C" {
     FILE *fo = utils::FopenCheck(CHAR(asChar(fname)), "w");
     for (size_t i = 0; i < olen; ++i) {
       fprintf(fo, "booster[%lu]:\n", i);
-      fprintf(fo, "%s\n", res[i]);
+      fprintf(fo, "%s", res[i]);
     }
     fclose(fo);
   }
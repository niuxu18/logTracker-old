@@ -11,27 +11,20 @@
 #include "../utils/xgboost_utils.h"
 #include "xgboost_gbmbase.h"
 // implementations of boosters
-#include "tree/xgboost_svdf_tree.hpp"
+#include "tree/xgboost_tree.hpp"
 #include "linear/xgboost_linear.hpp"
 
 namespace xgboost{
 	namespace booster{
-		/*
-         * \brief listing the types of boosters
-         */
-		enum BOOSTER_TYPE_LIST{
-			TREE,
-			LINEAR,
-		};
 		/*! 
 		* \brief create a gradient booster, given type of booster
 		* \param booster_type type of gradient booster, can be used to specify implements
 		* \return the pointer to the gradient booster created
 		*/
 		IBooster *CreateBooster( int booster_type ){
 			switch( booster_type ){
-            case TREE: return new RTreeTrainer();
-            case LINEAR: return new LinearBooster();
+            case 0: return new RegTreeTrainer();
+            case 1: return new LinearBooster();
 			default: utils::Error("unknown booster_type"); return NULL;
 			}
 		}
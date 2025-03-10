/*!
* \file xgboost.cpp
* \brief bootser implementations 
* \author Tianqi Chen: tianqi.tchen@gmail.com
*/
// implementation of boosters go to here 
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#include <climits>
#include "xgboost.h"
#include "../utils/xgboost_utils.h"
#include "xgboost_gbmbase.h"
// implementations of boosters
#include "tree/xgboost_tree.hpp"
#include "linear/xgboost_linear.hpp"

namespace xgboost{
	namespace booster{
		/*! 
		* \brief create a gradient booster, given type of booster
		* \param booster_type type of gradient booster, can be used to specify implements
		* \return the pointer to the gradient booster created
		*/
		IBooster *CreateBooster( int booster_type ){
			switch( booster_type ){
            case 0: return new RegTreeTrainer();
            case 1: return new LinearBooster();
			default: utils::Error("unknown booster_type"); return NULL;
			}
		}
	};
};



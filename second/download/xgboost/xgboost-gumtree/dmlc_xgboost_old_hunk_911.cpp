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
            case 0: return new RTreeTrainer();
            case 1: return new LinearBooster();
            default: utils::Error("unknown booster_type"); return NULL;
            }
        }
    };
};



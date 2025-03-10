           if( !strcmp("reg:logistic", name ) )    return new RegressionObj( LossType::kLogisticNeglik );
           if( !strcmp("binary:logistic", name ) ) return new RegressionObj( LossType::kLogisticClassify );
           if( !strcmp("binary:logitraw", name ) ) return new RegressionObj( LossType::kLogisticRaw );
           if( !strcmp("multi:softmax", name ) )   return new SoftmaxMultiClassObj(0);
           if( !strcmp("multi:softprob", name ) )   return new SoftmaxMultiClassObj(1);
           if( !strcmp("rank:pairwise", name ) ) return new PairwiseRankObj();
           if( !strcmp("rank:softmax", name ) )  return new SoftmaxRankObj();
           utils::Error("unknown objective function type");

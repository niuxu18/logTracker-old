                      const std::vector<float> &labels,
                      const std::vector<int> &group_index ) const {
        for( size_t i = 0; i < evals_.size(); ++ i ) {
            float res = evals_[i]->Eval( preds, labels );
            fprintf( fo, "\t%s-%s:%f", evname, evals_[i]->Name(), res );
        }
    }
    
private:
    EvalMAP map_;
    EvalNDCG ndcg_;
    std::vector<const IRankEvaluator*> evals_;

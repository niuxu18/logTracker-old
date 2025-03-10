            };
        }
    }


    namespace regrank{
        // simple pairwise rank 
        class LambdaRankObj : public IObjFunction{
        public:
            LambdaRankObj(void){}

            virtual ~LambdaRankObj(){}
            
            virtual void SetParam(const char *name, const char *val){
                if (!strcmp("loss_type", name)) loss_.loss_type = atoi(val);
                if (!strcmp("sampler", name)) sampler_.AssignSampler(atoi(val));
                if (!strcmp("lambda", name)) lambda_ = atoi(val);
            }

            virtual void GetGradient(const std::vector<float>& preds,
                const DMatrix::Info &info,
                int iter,
                std::vector<float> &grad,
                std::vector<float> &hess) {
                grad.resize(preds.size()); hess.resize(preds.size());
                const std::vector<unsigned> &group_index = info.group_ptr;
                utils::Assert(group_index.size() != 0 && group_index.back() == preds.size(), "rank loss must have group file");
                
                for (int i = 0; i < group_index.size() - 1; i++){
                    sample::Pairs pairs = sampler_.GenPairs(preds, info.labels, group_index[i], group_index[i + 1]);
                    //pairs.GetPairs()
                    std::vector< std::tuple<float, float, int> > sorted_triple = GetSortedTuple(preds, info.labels, group_index, i);
                    std::vector<int> index_remap = GetIndexMap(sorted_triple, group_index[i]);
                    GetGroupGradient(preds, info.labels, group_index,
                        grad, hess, sorted_triple, index_remap, pairs, i);
                }
            }

            virtual const char* DefaultEvalMetric(void) {
                return "auc";
            }

        private:
            int lambda_;
            const static int PAIRWISE = 0;
            const static int MAP = 1;
            const static int NDCG = 2;
            sample::PairSamplerWrapper sampler_;
            LossType loss_;
            /* \brief Sorted tuples of a group by the predictions, and 
            *         the fields in the return tuples successively are predicions, 
            *         labels, and the index of the instance
            */
            inline std::vector< std::tuple<float, float, int> > GetSortedTuple(const std::vector<float> &preds,
                const std::vector<float> &labels,
                const std::vector<unsigned> &group_index,
                int group){
                std::vector< std::tuple<float, float, int> > sorted_triple;
                for (int j = group_index[group]; j < group_index[group + 1]; j++){
                    sorted_triple.push_back(std::tuple<float, float, int>(preds[j], labels[j], j));
                }
                std::sort(sorted_triple.begin(), sorted_triple.end(), 
                    [](std::tuple<float, float, int> a, std::tuple<float, float, int> b){
                    return std::get<0>(a) > std::get<0>(b);
                });
                return sorted_triple;
            }

            inline std::vector<int> GetIndexMap(std::vector< std::tuple<float, float, int> > sorted_triple, int start){
                std::vector<int> index_remap;
                index_remap.resize(sorted_triple.size());
                for (int i = 0; i < sorted_triple.size(); i++){
                    index_remap[std::get<2>(sorted_triple[i]) - start] = i;
                }
                return index_remap;
            }

            inline float GetLambdaMAP(const std::vector< std::tuple<float, float, int> > sorted_triple,
                int index1, int index2,
                std::vector< std::tuple<float, float, float, float> > map_acc){
                if (index1 > index2) std::swap(index1, index2);
                float original = std::get<0>(map_acc[index2]);
                if (index1 != 0) original -= std::get<0>(map_acc[index1 - 1]);
                float changed = 0;
                if (std::get<1>(sorted_triple[index1]) < std::get<1>(sorted_triple[index2])){
                    changed += std::get<2>(map_acc[index2 - 1]) - std::get<2>(map_acc[index1]);
                    changed += (std::get<3>(map_acc[index1])+ 1.0f) / (index1 + 1);
                }
                else{
                    changed += std::get<1>(map_acc[index2 - 1]) - std::get<1>(map_acc[index1]);
                    changed += std::get<3>(map_acc[index2]) / (index2 + 1);
                }
                float ans = (changed - original) / (std::get<3>(map_acc[map_acc.size() - 1]));
                if (ans < 0) ans = -ans;
                return ans;
            }

            inline float GetLambdaNDCG(const std::vector< std::tuple<float, float, int> > sorted_triple,
                int index1,
                int index2, float IDCG){
                float original = pow(2, std::get<1>(sorted_triple[index1])) / log(index1 + 2)
                    + pow(2, std::get<1>(sorted_triple[index2])) / log(index2 + 2);
                float changed = pow(2, std::get<1>(sorted_triple[index2])) / log(index1 + 2)
                    + pow(2, std::get<1>(sorted_triple[index1])) / log(index2 + 2);
                float ans = (original - changed) / IDCG;
                if (ans < 0) ans = -ans;
                return ans;
            }


            inline float GetIDCG(const std::vector< std::tuple<float, float, int> > sorted_triple){
                std::vector<float> labels;
                for (int i = 0; i < sorted_triple.size(); i++){
                    labels.push_back(std::get<1>(sorted_triple[i]));
                }
                
                std::sort(labels.begin(), labels.end(), std::greater<float>());
                return EvalNDCG::DCG(labels);
            }

            inline std::vector< std::tuple<float, float, float, float> > GetMAPAcc(const std::vector< std::tuple<float, float, int> > sorted_triple){
                std::vector< std::tuple<float, float, float, float> > map_acc;
                float hit = 0, acc1 = 0, acc2 = 0, acc3 = 0;
                for (int i = 0; i < sorted_triple.size(); i++){
                    if (std::get<1>(sorted_triple[i]) == 1) {
                        hit++;
                        acc1 += hit / (i + 1);
                        acc2 += (hit - 1) / (i + 1);
                        acc3 += (hit + 1) / (i + 1);
                    }
                    map_acc.push_back(std::make_tuple(acc1, acc2, acc3, hit));
                }
                return map_acc;

            }

            inline void GetGroupGradient(const std::vector<float> &preds,
                const std::vector<float> &labels,
                const std::vector<unsigned> &group_index,
                std::vector<float> &grad,
                std::vector<float> &hess,
                const std::vector< std::tuple<float, float, int> > sorted_triple,
                const std::vector<int> index_remap,
                const sample::Pairs& pairs,
                int group){
                bool j_better;
                float IDCG, pred_diff, pred_diff_exp, delta;
                float first_order_gradient, second_order_gradient;
                std::vector< std::tuple<float, float, float, float> > map_acc;

                if (lambda_ == NDCG){
                    IDCG = GetIDCG(sorted_triple);
                }
                else if (lambda_ == MAP){
                    map_acc = GetMAPAcc(sorted_triple);
                }

                for (int j = group_index[group]; j < group_index[group + 1]; j++){
                    std::vector<int> pair_instance = pairs.GetPairs(j);
                    for (int k = 0; k < pair_instance.size(); k++){
                        j_better = labels[j] > labels[pair_instance[k]];
                        if (j_better){
                            switch (lambda_){
                                case PAIRWISE: delta = 1.0; break;
                                case MAP: delta = GetLambdaMAP(sorted_triple, index_remap[j - group_index[group]], index_remap[pair_instance[k] - group_index[group]], map_acc); break;
                                case NDCG: delta = GetLambdaNDCG(sorted_triple, index_remap[j - group_index[group]], index_remap[pair_instance[k] - group_index[group]], IDCG); break;
                                default: utils::Error("Cannot find the specified loss type");
                            }

                            pred_diff = preds[preds[j] - pair_instance[k]];
                            pred_diff_exp = j_better ? expf(-pred_diff) : expf(pred_diff);
                            first_order_gradient = delta * FirstOrderGradient(pred_diff_exp);
                            second_order_gradient = 2 * delta * SecondOrderGradient(pred_diff_exp);
                            hess[j] += second_order_gradient;
                            grad[j] += first_order_gradient;
                            hess[pair_instance[k]] += second_order_gradient;
                            grad[pair_instance[k]] += -first_order_gradient;
                        }
                    }
                }
            }
            inline float FirstOrderGradient(float pred_diff_exp) const {
                return -pred_diff_exp / (1 + pred_diff_exp);
            }
            inline float SecondOrderGradient(float pred_diff_exp) const {
                return pred_diff_exp / pow(1 + pred_diff_exp, 2);
            }        
        };

}
 No newline at end of file
#endif

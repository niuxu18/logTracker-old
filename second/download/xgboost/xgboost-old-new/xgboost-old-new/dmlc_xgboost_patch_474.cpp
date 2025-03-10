@@ -11,314 +11,319 @@
 #include "../utils/xgboost_config.h"
 
 namespace xgboost{
-	namespace base{
-		/*!
-		* \brief wrapping the training process of the gradient boosting model,
-		*   given the configuation
-		* \author Kailong Chen: chenkl198812@gmail.com, Tianqi Chen: tianqi.chen@gmail.com
-		*/
-		class BoostTask{
-		public:
-			inline int Run(int argc, char *argv[]){
-				if (argc < 2){
-					printf("Usage: <config>\n");
-					return 0;
-				}
-				utils::ConfigIterator itr(argv[1]);
-				while (itr.Next()){
-					this->SetParam(itr.name(), itr.val());
-				}
-				for (int i = 2; i < argc; i++){
-					char name[256], val[256];
-					if (sscanf(argv[i], "%[^=]=%s", name, val) == 2){
-						this->SetParam(name, val);
-					}
-				}
-				this->InitData();
-				this->InitLearner();
-				if (task == "dump"){
-					this->TaskDump();
-					return 0;
-				}
-				if (task == "interact"){
-					this->TaskInteractive(); return 0;
-				}
-				if (task == "dumppath"){
-					this->TaskDumpPath(); return 0;
-				}
-				if (task == "eval"){
-					this->TaskEval(); return 0;
-				}
-				if (task == "pred"){
-					this->TaskPred();
-				}
-				else{
-					this->TaskTrain();
-				}
-				return 0;
-			}
+    namespace base{
+        /*!
+        * \brief wrapping the training process of the gradient boosting model,
+        *   given the configuation
+        * \author Kailong Chen: chenkl198812@gmail.com, Tianqi Chen: tianqi.chen@gmail.com
+        */
+        class BoostTask{
+        public:
+            inline int Run(int argc, char *argv[]){
 
-			enum learning_tasks{
-				REGRESSION = 0,
-				BINARY_CLASSIFICATION = 1,
-				RANKING = 2
-			};
+                if (argc < 2){
+                    printf("Usage: <config>\n");
+                    return 0;
+                }
+                utils::ConfigIterator itr(argv[1]);
+                while (itr.Next()){
+                    this->SetParam(itr.name(), itr.val());
+                }
+                for (int i = 2; i < argc; i++){
+                    char name[256], val[256];
+                    if (sscanf(argv[i], "%[^=]=%s", name, val) == 2){
+                        this->SetParam(name, val);
+                    }
+                }
+		
+                this->InitData();
+                this->InitLearner();
+                if (task == "dump"){
+                    this->TaskDump();
+                    return 0;
+                }
+                if (task == "interact"){
+                    this->TaskInteractive(); return 0;
+                }
+                if (task == "dumppath"){
+                    this->TaskDumpPath(); return 0;
+                }
+                if (task == "eval"){
+                    this->TaskEval(); return 0;
+                }
+                if (task == "pred"){
+                    this->TaskPred();
+                }
+                else{
+                    this->TaskTrain();
+                }
+                return 0;
+            }
 
-			/* \brief set learner
-			 * \param learner the passed in learner
-			 */
-			inline void SetLearner(BoostLearner* learner){
-				learner_ = learner;
-			}
+            enum learning_tasks{
+                REGRESSION = 0,
+                BINARY_CLASSIFICATION = 1,
+                RANKING = 2
+            };
 
-			inline void SetParam(const char *name, const char *val){
-				if (!strcmp("learning_task", name)) learning_task = atoi(val);
-				if (!strcmp("silent", name))       silent = atoi(val);
-				if (!strcmp("use_buffer", name))   use_buffer = atoi(val);
-				if (!strcmp("seed", name))         random::Seed(atoi(val));
-				if (!strcmp("num_round", name))    num_round = atoi(val);
-				if (!strcmp("save_period", name))  save_period = atoi(val);
-				if (!strcmp("task", name))         task = val;
-				if (!strcmp("data", name))        train_path = val;
-				if (!strcmp("test:data", name))   test_path = val;
-				if (!strcmp("model_in", name))    model_in = val;
-				if (!strcmp("model_out", name))   model_out = val;
-				if (!strcmp("model_dir", name))    model_dir_path = val;
-				if (!strcmp("fmap", name))        name_fmap = val;
-				if (!strcmp("name_dump", name))   name_dump = val;
-				if (!strcmp("name_dumppath", name))   name_dumppath = val;
-				if (!strcmp("name_pred", name))   name_pred = val;
-				if (!strcmp("dump_stats", name))   dump_model_stats = atoi(val);
-				if (!strcmp("interact:action", name))  interact_action = val;
-				if (!strncmp("batch:", name, 6)){
-					cfg_batch.PushBack(name + 6, val);
-				}
-				if (!strncmp("eval[", name, 5)) {
-					char evname[256];
-					utils::Assert(sscanf(name, "eval[%[^]]", evname) == 1, "must specify evaluation name for display");
-					eval_data_names.push_back(std::string(evname));
-					eval_data_paths.push_back(std::string(val));
-				}
-				cfg.PushBack(name, val);
-			}
-		public:
-			BoostTask(void){
-				// default parameters
-				silent = 0;
-				use_buffer = 1;
-				num_round = 10;
-				save_period = 0;
-				dump_model_stats = 0;
-				task = "train";
-				model_in = "NULL";
-				model_out = "NULL";
-				name_fmap = "NULL";
-				name_pred = "pred.txt";
-				name_dump = "dump.txt";
-				name_dumppath = "dump.path.txt";
-				model_dir_path = "./";
-				interact_action = "update";
-			}
-			~BoostTask(void){
-				for (size_t i = 0; i < deval.size(); i++){
-					delete deval[i];
-				}
-			}
-		private:
+            /* \brief set learner
+             * \param learner the passed in learner
+             */
+            inline void SetLearner(BoostLearner* learner){
+                learner_ = learner;
+            }
 
+            inline void SetParam(const char *name, const char *val){
+                if (!strcmp("learning_task", name)) learning_task = atoi(val);
+                if (!strcmp("silent", name))       silent = atoi(val);
+                if (!strcmp("use_buffer", name))   use_buffer = atoi(val);
+                if (!strcmp("seed", name))         random::Seed(atoi(val));
+                if (!strcmp("num_round", name))    num_round = atoi(val);
+                if (!strcmp("save_period", name))  save_period = atoi(val);
+                if (!strcmp("task", name))         task = val;
+                if (!strcmp("data", name))        train_path = val;
+                if (!strcmp("test:data", name))   test_path = val;
+                if (!strcmp("model_in", name))    model_in = val;
+                if (!strcmp("model_out", name))   model_out = val;
+                if (!strcmp("model_dir", name))    model_dir_path = val;
+                if (!strcmp("fmap", name))        name_fmap = val;
+                if (!strcmp("name_dump", name))   name_dump = val;
+                if (!strcmp("name_dumppath", name))   name_dumppath = val;
+                if (!strcmp("name_pred", name))   name_pred = val;
+                if (!strcmp("dump_stats", name))   dump_model_stats = atoi(val);
+                if (!strcmp("interact:action", name))  interact_action = val;
+                if (!strncmp("batch:", name, 6)){
+                    cfg_batch.PushBack(name + 6, val);
+                }
+                if (!strncmp("eval[", name, 5)) {
+                    char evname[256];
+                    utils::Assert(sscanf(name, "eval[%[^]]", evname) == 1, "must specify evaluation name for display");
+                    eval_data_names.push_back(std::string(evname));
+                    eval_data_paths.push_back(std::string(val));
+                }
+                cfg.PushBack(name, val);
+            }
+        public:
+            BoostTask(void){
+                // default parameters
+                silent = 0;
+                use_buffer = 1;
+                num_round = 10;
+                save_period = 0;
+                dump_model_stats = 0;
+                task = "train";
+                model_in = "NULL";
+                model_out = "NULL";
+                name_fmap = "NULL";
+                name_pred = "pred.txt";
+                name_dump = "dump.txt";
+                name_dumppath = "dump.path.txt";
+                model_dir_path = "./";
+                interact_action = "update";
+            }
+            ~BoostTask(void){
+                for (size_t i = 0; i < deval.size(); i++){
+                    delete deval[i];
+                }
+            }
+        private:
 
-			inline void InitData(void){
-				if (name_fmap != "NULL") fmap.LoadText(name_fmap.c_str());
-				if (task == "dump") return;
-				if (learning_task == RANKING){
-					char instance_path[256], group_path[256];
-					if (task == "pred" || task == "dumppath"){
-						sscanf(test_path.c_str(), "%[^;];%s", instance_path, group_path);
-						data.CacheLoad(instance_path, group_path, silent != 0, use_buffer != 0);
-					}
-					else{
-						// training 
-						sscanf(train_path.c_str(), "%[^;];%s", instance_path, group_path);
-						data.CacheLoad(instance_path, group_path, silent != 0, use_buffer != 0);
-						utils::Assert(eval_data_names.size() == eval_data_paths.size());
-						for (size_t i = 0; i < eval_data_names.size(); ++i){
-							deval.push_back(new DMatrix());
-							sscanf(eval_data_paths[i].c_str(), "%[^;];%s", instance_path, group_path);
-							deval.back()->CacheLoad(instance_path, group_path, silent != 0, use_buffer != 0);
-						}
-					}
 
+            inline void InitData(void){
+	        
+                if (name_fmap != "NULL") fmap.LoadText(name_fmap.c_str());
+                if (task == "dump") return;
+                if (learning_task == RANKING){
+                    char instance_path[256], group_path[256];
+                    if (task == "pred" || task == "dumppath"){
+                        sscanf(test_path.c_str(), "%[^;];%s", instance_path, group_path);
+                        data.CacheLoad(instance_path, group_path, silent != 0, use_buffer != 0);
+                    }
+                    else{
+                        // training 
+                        sscanf(train_path.c_str(), "%[^;];%s", instance_path, group_path);
+		        data.CacheLoad(instance_path, group_path, silent != 0, use_buffer != 0);
+                        
+			utils::Assert(eval_data_names.size() == eval_data_paths.size());
+                        for (size_t i = 0; i < eval_data_names.size(); ++i){
+                            deval.push_back(new DMatrix());
+                            sscanf(eval_data_paths[i].c_str(), "%[^;];%s", instance_path, group_path);
+                            deval.back()->CacheLoad(instance_path, group_path, silent != 0, use_buffer != 0);
+                        }
+                    }
+                }
+                else{
+                    if (task == "pred" || task == "dumppath"){
+                        data.CacheLoad(test_path.c_str(), "", silent != 0, use_buffer != 0);
+                    }
+                    else{
+                        // training 
+                        data.CacheLoad(train_path.c_str(), "", silent != 0, use_buffer != 0);
+                        utils::Assert(eval_data_names.size() == eval_data_paths.size());
+                        for (size_t i = 0; i < eval_data_names.size(); ++i){
+                            deval.push_back(new DMatrix());
+                            deval.back()->CacheLoad(eval_data_paths[i].c_str(), "", silent != 0, use_buffer != 0);
+                        }
+                    }
+                }
 
-				}
-				else{
-					if (task == "pred" || task == "dumppath"){
-						data.CacheLoad(test_path.c_str(), "", silent != 0, use_buffer != 0);
-					}
-					else{
-						// training 
-						data.CacheLoad(train_path.c_str(), "", silent != 0, use_buffer != 0);
-						utils::Assert(eval_data_names.size() == eval_data_paths.size());
-						for (size_t i = 0; i < eval_data_names.size(); ++i){
-							deval.push_back(new DMatrix());
-							deval.back()->CacheLoad(eval_data_paths[i].c_str(), "", silent != 0, use_buffer != 0);
-						}
-					}
-				}
+                learner_->SetData(&data, deval, eval_data_names);
+		if(!silent) printf("BoostTask:Data Initiation Done!\n");
+            }
+            
+            inline void InitLearner(void){
+                cfg.BeforeFirst();
+                while (cfg.Next()){
+		     learner_->SetParam(cfg.name(), cfg.val());
+                }
+                if (model_in != "NULL"){
+                    utils::FileStream fi(utils::FopenCheck(model_in.c_str(), "rb"));
+                    learner_->LoadModel(fi);
+                    fi.Close();
+                }
+                else{
+                    utils::Assert(task == "train", "model_in not specified");
+                    learner_->InitModel();
+                }
+                learner_->InitTrainer();
+		if(!silent) printf("BoostTask:InitLearner Done!\n");
+            }
 
-				learner_->SetData(&data, deval, eval_data_names);
-			}
-			inline void InitLearner(void){
-				cfg.BeforeFirst();
-				while (cfg.Next()){
-					learner_->SetParam(cfg.name(), cfg.val());
-				}
-				if (model_in != "NULL"){
-					utils::FileStream fi(utils::FopenCheck(model_in.c_str(), "rb"));
-					learner_->LoadModel(fi);
-					fi.Close();
-				}
-				else{
-					utils::Assert(task == "train", "model_in not specified");
-					learner_->InitModel();
-				}
-				learner_->InitTrainer();
-			}
+            inline void TaskTrain(void){
+                const time_t start = time(NULL);
+                unsigned long elapsed = 0;
+                for (int i = 0; i < num_round; ++i){
+                    elapsed = (unsigned long)(time(NULL) - start);
+                    if (!silent) printf("boosting round %d, %lu sec elapsed\n", i, elapsed);
+                    learner_->UpdateOneIter(i);
+		    learner_->EvalOneIter(i);
+                    if (save_period != 0 && (i + 1) % save_period == 0){
+                        this->SaveModel(i);
+                    }
+                    elapsed = (unsigned long)(time(NULL) - start);
+                }
+                // always save final round
+                if (save_period == 0 || num_round % save_period != 0){
+                    if (model_out == "NULL"){
+                        this->SaveModel(num_round - 1);
+                    }
+                    else{
+                        this->SaveModel(model_out.c_str());
+                    }
+                }
+                if (!silent){
+                    printf("\nupdating end, %lu sec in all\n", elapsed);
+                }
+            }
+            inline void TaskEval(void){
+                learner_->EvalOneIter(0);
+            }
+            inline void TaskInteractive(void){
+                const time_t start = time(NULL);
+                unsigned long elapsed = 0;
+                int batch_action = 0;
 
-			inline void TaskTrain(void){
-				const time_t start = time(NULL);
-				unsigned long elapsed = 0;
-				for (int i = 0; i < num_round; ++i){
-					elapsed = (unsigned long)(time(NULL) - start);
-					if (!silent) printf("boosting round %d, %lu sec elapsed\n", i, elapsed);
-					learner_->UpdateOneIter(i);
-					learner_->EvalOneIter(i);
-					if (save_period != 0 && (i + 1) % save_period == 0){
-						this->SaveModel(i);
-					}
-					elapsed = (unsigned long)(time(NULL) - start);
-				}
-				// always save final round
-				if (save_period == 0 || num_round % save_period != 0){
-					if (model_out == "NULL"){
-						this->SaveModel(num_round - 1);
-					}
-					else{
-						this->SaveModel(model_out.c_str());
-					}
-				}
-				if (!silent){
-					printf("\nupdating end, %lu sec in all\n", elapsed);
-				}
-			}
-			inline void TaskEval(void){
-				learner_->EvalOneIter(0);
-			}
-			inline void TaskInteractive(void){
-				const time_t start = time(NULL);
-				unsigned long elapsed = 0;
-				int batch_action = 0;
+                cfg_batch.BeforeFirst();
+                while (cfg_batch.Next()){
+                    if (!strcmp(cfg_batch.name(), "run")){
+                        learner_->UpdateInteract(interact_action);
+                        batch_action += 1;
+                    }
+                    else{
+                        learner_->SetParam(cfg_batch.name(), cfg_batch.val());
+                    }
+                }
 
-				cfg_batch.BeforeFirst();
-				while (cfg_batch.Next()){
-					if (!strcmp(cfg_batch.name(), "run")){
-						learner_->UpdateInteract(interact_action);
-						batch_action += 1;
-					}
-					else{
-						learner_->SetParam(cfg_batch.name(), cfg_batch.val());
-					}
-				}
+                if (batch_action == 0){
+                    learner_->UpdateInteract(interact_action);
+                }
+                utils::Assert(model_out != "NULL", "interactive mode must specify model_out");
+                this->SaveModel(model_out.c_str());
+                elapsed = (unsigned long)(time(NULL) - start);
 
-				if (batch_action == 0){
-					learner_->UpdateInteract(interact_action);
-				}
-				utils::Assert(model_out != "NULL", "interactive mode must specify model_out");
-				this->SaveModel(model_out.c_str());
-				elapsed = (unsigned long)(time(NULL) - start);
+                if (!silent){
+                    printf("\ninteractive update, %d batch actions, %lu sec in all\n", batch_action, elapsed);
+                }
+            }
 
-				if (!silent){
-					printf("\ninteractive update, %d batch actions, %lu sec in all\n", batch_action, elapsed);
-				}
-			}
+            inline void TaskDump(void){
+                FILE *fo = utils::FopenCheck(name_dump.c_str(), "w");
+                learner_->DumpModel(fo, fmap, dump_model_stats != 0);
+                fclose(fo);
+            }
+            inline void TaskDumpPath(void){
+                FILE *fo = utils::FopenCheck(name_dumppath.c_str(), "w");
+                learner_->DumpPath(fo, data);
+                fclose(fo);
+            }
+            inline void SaveModel(const char *fname) const{
+                utils::FileStream fo(utils::FopenCheck(fname, "wb"));
+                learner_->SaveModel(fo);
+                fo.Close();
+            }
+            inline void SaveModel(int i) const{
+                char fname[256];
+                sprintf(fname, "%s/%04d.model", model_dir_path.c_str(), i + 1);
+                this->SaveModel(fname);
+            }
+            inline void TaskPred(void){
+                std::vector<float> preds;
+                if (!silent) printf("start prediction...\n");
+                learner_->Predict(preds, data);
+                if (!silent) printf("writing prediction to %s\n", name_pred.c_str());
+                FILE *fo = utils::FopenCheck(name_pred.c_str(), "w");
+                for (size_t i = 0; i < preds.size(); i++){
+                    fprintf(fo, "%f\n", preds[i]);
+                }
+                fclose(fo);
+            }
+        private:
+            /* \brief specify the learning task*/
+            int learning_task;
+            /* \brief whether silent */
+            int silent;
+            /* \brief whether use auto binary buffer */
+            int use_buffer;
+            /* \brief number of boosting iterations */
+            int num_round;
+            /* \brief the period to save the model, 0 means only save the final round model */
+            int save_period;
+            /*! \brief interfact action */
+            std::string interact_action;
+            /* \brief the path of training/test data set */
+            std::string train_path, test_path;
+            /* \brief the path of test model file, or file to restart training */
+            std::string model_in;
+            /* \brief the path of final model file, to be saved */
+            std::string model_out;
+            /* \brief the path of directory containing the saved models */
+            std::string model_dir_path;
+            /* \brief task to perform, choosing training or testing */
+            std::string task;
+            /* \brief name of predict file */
+            std::string name_pred;
+            /* \brief whether dump statistics along with model */
+            int dump_model_stats;
+            /* \brief name of feature map */
+            std::string name_fmap;
+            /* \brief name of dump file */
+            std::string name_dump;
+            /* \brief name of dump path file */
+            std::string name_dumppath;
+            /* \brief the paths of validation data sets */
+            std::vector<std::string> eval_data_paths;
+            /* \brief the names of the evaluation data used in output log */
+            std::vector<std::string> eval_data_names;
+            /*! \brief saves configurations */
+            utils::ConfigSaver cfg;
+            /*! \brief batch configurations */
+            utils::ConfigSaver cfg_batch;
+        private:
+            DMatrix data;
+            std::vector<DMatrix*> deval;
+            utils::FeatMap fmap;
+            BoostLearner* learner_;
 
-			inline void TaskDump(void){
-				FILE *fo = utils::FopenCheck(name_dump.c_str(), "w");
-				learner_->DumpModel(fo, fmap, dump_model_stats != 0);
-				fclose(fo);
-			}
-			inline void TaskDumpPath(void){
-				FILE *fo = utils::FopenCheck(name_dumppath.c_str(), "w");
-				learner_->DumpPath(fo, data);
-				fclose(fo);
-			}
-			inline void SaveModel(const char *fname) const{
-				utils::FileStream fo(utils::FopenCheck(fname, "wb"));
-				learner_->SaveModel(fo);
-				fo.Close();
-			}
-			inline void SaveModel(int i) const{
-				char fname[256];
-				sprintf(fname, "%s/%04d.model", model_dir_path.c_str(), i + 1);
-				this->SaveModel(fname);
-			}
-			inline void TaskPred(void){
-				std::vector<float> preds;
-				if (!silent) printf("start prediction...\n");
-				learner_->Predict(preds, data);
-				if (!silent) printf("writing prediction to %s\n", name_pred.c_str());
-				FILE *fo = utils::FopenCheck(name_pred.c_str(), "w");
-				for (size_t i = 0; i < preds.size(); i++){
-					fprintf(fo, "%f\n", preds[i]);
-				}
-				fclose(fo);
-			}
-		private:
-			/* \brief specify the learning task*/
-			int learning_task;
-			/* \brief whether silent */
-			int silent;
-			/* \brief whether use auto binary buffer */
-			int use_buffer;
-			/* \brief number of boosting iterations */
-			int num_round;
-			/* \brief the period to save the model, 0 means only save the final round model */
-			int save_period;
-			/*! \brief interfact action */
-			std::string interact_action;
-			/* \brief the path of training/test data set */
-			std::string train_path, test_path;
-			/* \brief the path of test model file, or file to restart training */
-			std::string model_in;
-			/* \brief the path of final model file, to be saved */
-			std::string model_out;
-			/* \brief the path of directory containing the saved models */
-			std::string model_dir_path;
-			/* \brief task to perform, choosing training or testing */
-			std::string task;
-			/* \brief name of predict file */
-			std::string name_pred;
-			/* \brief whether dump statistics along with model */
-			int dump_model_stats;
-			/* \brief name of feature map */
-			std::string name_fmap;
-			/* \brief name of dump file */
-			std::string name_dump;
-			/* \brief name of dump path file */
-			std::string name_dumppath;
-			/* \brief the paths of validation data sets */
-			std::vector<std::string> eval_data_paths;
-			/* \brief the names of the evaluation data used in output log */
-			std::vector<std::string> eval_data_names;
-			/*! \brief saves configurations */
-			utils::ConfigSaver cfg;
-			/*! \brief batch configurations */
-			utils::ConfigSaver cfg_batch;
-		private:
-			DMatrix data;
-			std::vector<DMatrix*> deval;
-			utils::FeatMap fmap;
-			BoostLearner* learner_;
-
-		};
-	};
+        };
+    };
 };
  if (!strcmp("reg:logistic", name)) return new RegLossObj(LossType::kLogisticNeglik);
  if (!strcmp("binary:logistic", name)) return new RegLossObj(LossType::kLogisticClassify);
  if (!strcmp("binary:logitraw", name)) return new RegLossObj(LossType::kLogisticRaw);
  if (!strcmp("multi:softmax", name)) return new SoftmaxMultiClassObj(0);
  if (!strcmp("multi:softprob", name)) return new SoftmaxMultiClassObj(1);
  utils::Error("unknown objective function type: %s", name);
  return NULL;
}

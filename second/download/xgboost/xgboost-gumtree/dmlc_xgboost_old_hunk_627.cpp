  if (!strcmp("reg:logistic", name)) return new RegLossObj(LossType::kLogisticNeglik);
  if (!strcmp("binary:logistic", name)) return new RegLossObj(LossType::kLogisticClassify);
  if (!strcmp("binary:logitraw", name)) return new RegLossObj(LossType::kLogisticRaw);
  utils::Error("unknown objective function type: %s", name);
  return NULL;
}

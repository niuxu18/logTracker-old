template<typename T>
inline bool CheckNAN(T v) {
#ifdef _MSC_VER
  return (_isnan(v) != 0);
#else
  return isnan(v);
#endif
}
template<typename T>
inline T LogGamma(T v) {
#ifdef _MSC_VER
#if _MSC_VER >= 1800
  return lgamma(v);
#else
#pragma message ("Warning: lgamma function was not available until VS2013, poisson regression will be disabled")
  utils::Error("lgamma function was not available until VS2013");
  return static_cast<T>(1.0);
#endif
#else
  return lgamma(v);
#endif
}
#endif
}  // namespace utils

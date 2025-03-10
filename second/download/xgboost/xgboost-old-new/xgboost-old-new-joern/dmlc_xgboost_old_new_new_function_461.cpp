inline void Assert(bool exp, const char *fmt, ...) {
  if (!exp) {
    std::string msg(kErrorBuffer, '\0');
    va_list args;
    va_start(args, fmt);
    vsnprintf(&msg[0], kErrorBuffer, fmt, args);
    va_end(args);
    HandleAssertError(msg.c_str());
  }
}
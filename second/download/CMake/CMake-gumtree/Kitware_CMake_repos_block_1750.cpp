{
  snprintf(name, size, "\\\\?\\pipe\\uv\\%p-%lu", ptr, GetCurrentProcessId());
}
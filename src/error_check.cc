#include "error_check.h"

void reportErrorMessage(const char *message)
{
  std::cerr << "OptiX Error: '" << message << "'\n";
}

void handleError(RTcontext context, RTresult code, const char *file, int line)
{
  const char *message;
  char s[2048];
  rtContextGetErrorString(context, code, &message);
  sprintf(s, "%s\n(%s:%d)", message, file, line);
  reportErrorMessage(s);
}

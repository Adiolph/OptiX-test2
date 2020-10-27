#ifndef ERROR_CHECK_H
#define ERROR_CHECK_H
#include <optix_world.h>

// Error handling extracted from SDK/sutil/sutil.{h,cpp}
struct APIError
{
  APIError(RTresult c, const std::string &f, int l)
      : code(c), file(f), line(l) {}
  RTresult code;
  std::string file;
  int line;
};

void reportErrorMessage(const char *message);
void handleError(RTcontext context, RTresult code, const char *file, int line);

#define RT_CHECK_ERROR(func)                    \
  do                                            \
  {                                             \
    RTresult code = func;                       \
    if (code != RT_SUCCESS)                     \
      throw APIError(code, __FILE__, __LINE__); \
  } while (0)

#define SUTIL_CATCH(ctx)                              \
  catch (APIError & e)                                \
  {                                                   \
    handleError(ctx, e.code, e.file.c_str(), e.line); \
  }                                                   \
  catch (std::exception & e)                          \
  {                                                   \
    reportErrorMessage(e.what());                     \
    exit(1);                                          \
  }

#endif  // ERROR_CHECK_H
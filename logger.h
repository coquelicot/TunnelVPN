#ifndef __logger__
#define __logger__

#include <cstdio>
#include <cstdlib>

#define _log(fmt, type, ...) fprintf(stderr, "[%-5s] %s(%s:%d) :: " fmt "\n", type, __func__, __FILE__, __LINE__, ##__VA_ARGS__)
#define logError(fmt, ...) { _log(fmt, "ERROR", ##__VA_ARGS__); exit(-1); }
#define logWarning(fmt, ...) { if (::logLevel >= ::LOG_LV_WARNING) _log(fmt, "WARN", ##__VA_ARGS__); }
#define logInfo(fmt, ...) { if (::logLevel >= ::LOG_LV_INFO) _log(fmt, "INFO", ##__VA_ARGS__); }
#define logDebug(fmt, ...) { if (::logLevel >= ::LOG_LV_DEBUG) _log(fmt, "DEBUG", ##__VA_ARGS__); }

extern int logLevel;
extern const int LOG_LV_ERROR;
extern const int LOG_LV_WARNING;
extern const int LOG_LV_INFO;
extern const int LOG_LV_DEBUG;

#endif

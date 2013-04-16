#ifndef SCALLOC_LOG_H_
#define SCALLOC_LOG_H_

#include <stdarg.h>
#include <unistd.h>

enum Severity {
  TRACE = -1,
  INFO = -2,
  WARNING = -3,
  ERROR = -4,
  FATAL = -5
};

const int kVerbosity = INFO;
const int kLogLen = 240;

void LogPrintf(int severity, const char* format, ...);

// Here comes the ugly part. What we basically want is a construct that is only
// compiled if the log level is succiciently high. Furthermore we don't even
// want a function call if the severity is too low.
//
// To do so we basically use a varadic macro (with gcc extension to allow empty
// variable args) and create a branch (if with constant expression that is
// false) that _should_ be optimized out by the compiler.

#define LOG_ON(severity) (kVerbosity >= severity)

#define LOG(severity, format, ...) do {           \
  if (LOG_ON(severity)) {                         \
    LogPrintf(severity, format, ##__VA_ARGS__);   \
  }                                               \
  if (severity == FATAL) {                        \
    abort();                                      \
  }                                               \
} while (0)

#define ErrorOut(format, ...) do {                \
  if (LOG_ON(FATAL)) {                            \
    LogPrintf(FATAL, format, ##__VA_ARGS__);      \
  }                                               \
  abort();                                        \
} while (0)

#endif  // SCALLOC_LOG_H_

// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_LOG_H_
#define SCALLOC_LOG_H_

#include <stdarg.h>
#include <stdlib.h>  // abort()
#include <unistd.h>

#ifndef LOG_LEVEL
#define LOG_LEVEL kWarning
#endif  // LOG_LEVEL

enum Severity {
  kTrace = -1,
  kInfo = -2,
  kWarning = -3,
  kError = -4,
  kFatal = -5
};

const int kVerbosity = LOG_LEVEL;
const int kLogLen = 240;

void LogPrintf(const char* category,
               const int severity,
               const char* format, ...);


// Here comes the ugly part. What we basically want is a construct that is only
// compiled if the log level is succiciently high. Furthermore we don't even
// want a function call if the severity is too low.
//
// To do so we basically use a varadic macro (with gcc extension to allow empty
// variable args) and create a branch (if with constant expression that is
// false) that _should_ be optimized out by the compiler.

#define LOG_ON(severity) (kVerbosity >= severity)


#define LOG(severity, format, ...) do {                                        \
  if (LOG_ON(severity)) {                                                      \
    LogPrintf("", severity, format, ##__VA_ARGS__);                            \
  }                                                                            \
  if (severity == kFatal) {                                                    \
    abort();                                                                   \
  }                                                                            \
} while (0)


#define LOG_CAT(cat, severity, format, ...) do {                               \
  if (LOG_ON(severity)) {                                                      \
    LogPrintf(cat, severity, format, ##__VA_ARGS__);                           \
  }                                                                            \
  if (severity == kFatal) {                                                    \
    abort();                                                                   \
  }                                                                            \
} while (0)


#define Fatal(format, ...) do {                                                \
  LogPrintf("" , kFatal, format, ##__VA_ARGS__);                               \
  abort();                                                                     \
  } while (0)


#define QUOTEME(x) #x


#ifdef DEBUG

#define ScallocAssert(c) do {                                                  \
  if (!(c)) {                                                                  \
    Fatal("assertion failed: " QUOTEME((c)));                                  \
  }                                                                            \
} while (0)

#else  // !DEBUG

#define ScallocAssert(c) do {                                                  \
} while (0)

#endif  // DEBUG

#endif  // SCALLOC_LOG_H_

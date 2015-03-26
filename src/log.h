// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_LOG_H_
#define SCALLOC_LOG_H_

#include <stdarg.h>
#include <stdlib.h>  // abort()
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifndef SCALLOC_LOG_LEVEL
#define SCALLOC_LOG_LEVEL kWarning
#endif  // SCALLOC_LOG_LEVEL

enum Severity {
  kTrace = -1,
  kInfo = -2,
  kWarning = -3,
  kError = -4,
  kFatal = -5
};


const int kVerbosity = SCALLOC_LOG_LEVEL;
const int kLogLen = 512;


// __FILE__ expands to the full path. Strip basename for __FILENAME__.
#define __FILENAME__                                                           \
    (strrchr(__FILE__, '/') ?                                                  \
        strrchr(__FILE__, '/') + 1 :                                           \
        strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)


inline void LogPrintf(
    int line, const char* file, const char* format, ...) {
  char buffer[kLogLen] = { 0 };
  char line_buffer[8];

  int rest = kLogLen
      - 1 /* \n */
      - 1 /* \0 */;

  snprintf(line_buffer, sizeof(line_buffer), "%d", line);
  // Start with "__FILENAME__:__LINE__ ".
  strncat(buffer, file, strlen(file));
  rest -= strlen(file);
  strncat(buffer, ":", 1);
  rest -= 1;
  strncat(buffer, line_buffer, strlen(line_buffer));
  rest -= strlen(line_buffer);
  strncat(buffer, " ", 1);
  rest -= 1;

  // Sanity check.
  if (rest < 0) { abort(); }

  va_list args;
  va_start(args, format);
  char* rest_start = buffer + strlen(buffer);
  // vsnprintf size includes \0 (see man page).
  int would = vsnprintf(rest_start, rest + 1 /* including \0 */, format, args);
  // Ditto for the check.
  if (would >= (rest + 1 /* including \0 */)) {
    const char* truncate_suffix = "...";
    // For copying the suffix we need actual rest value again.
    strncpy(rest_start + (rest - strlen(truncate_suffix)),
            truncate_suffix,
            strlen(truncate_suffix));
  }

  strncat(buffer, "\n", 1);

  // Sanity check.
  if (buffer[kLogLen-1] != 0) {
    abort();
  }

  if (write(STDERR_FILENO, buffer, strlen(buffer)) == -1) {
    // Well, if write() failes, just give up...
    abort();
  }
}


#define LOG_ON(severity) (kVerbosity >= severity)


#define LOG(severity, format, ...) do {                                        \
  if (LOG_ON(severity)) {                                                      \
    LogPrintf(__LINE__, __FILENAME__, format, ##__VA_ARGS__);                 \
  }                                                                            \
  if (severity == kFatal) {                                                    \
    abort();                                                                   \
  }                                                                            \
} while (0)


#define Fatal(format, ...) do {                                                \
  LogPrintf(__LINE__, __FILENAME__, format, ##__VA_ARGS__);                   \
  abort();                                                                     \
} while (0)

#endif  // SCALLOC_LOG_H_

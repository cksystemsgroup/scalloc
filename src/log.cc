// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "log.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void LogPrintf(const char* category,
               const int severity,
               const char* format,
                ...) {
  const char* suffix = "...";
  const char* fatal_prefix = "FATAL ERROR: ";
  char buffer[kLogLen] = { 0 };
  int rest = kLogLen;
  const size_t netto_len = rest - strlen(suffix) - 1;
  va_list args;
  strncat(buffer, "[", 1);
  strncat(buffer, category, kLogLen - 3);
  rest -= strlen(category);
  strncat(buffer, "] ", 2);
  rest -= 3;
  if (severity == kFatal) {
    strncat(buffer, fatal_prefix, kLogLen);
    rest -= strlen(fatal_prefix);
  }
  va_start(args, format);
  int would = vsnprintf(&(buffer[kLogLen-rest]), netto_len, format, args);
  va_end(args);
  if (would > rest) {
    strncat(buffer, suffix, strlen(suffix));
  }
  strncat(buffer, "\n", 1);
  if (write(STDERR_FILENO, buffer, strlen(buffer)) == -1) {
    // Well, if write() failes, just give up...
    abort();
  }
}

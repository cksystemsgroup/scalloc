// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "huge_page.h"

#include <fcntl.h>

#include "utils.h"

namespace {
const char* kProcHugepage = "/proc/meminfo";
}

namespace scalloc {
namespace hugepage {

size_t GetSize() {
  const int fd = open(kProcHugepage, O_RDONLY);
  const int64_t size = utils::ReadProcEntry(fd, "Hugepagesize");
  if (size == -1) {
    return 0;
  }
  return static_cast<size_t>(size);
}

}  // namespace hugepage
}  // namespace scalloc


// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_HUGE_PAGE_H
#define SCALLOC_HUGE_PAGE_H

#ifdef __linux__

#include <stddef.h>

namespace scalloc {
namespace hugepage {

// Returns the size of a hugepage in kB, or 0 if hugepages are not supported.
size_t GetSize();

}  // namespace hugepage
}  // namespace scalloc

#endif  // __linux__

#endif  // SCALLOC_HUGE_PAGE_H


// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_SCALLOC_GUARD_H_
#define SCALLOC_SCALLOC_GUARD_H_

namespace scalloc {

class ScallocGuard {
 public:
  static int scallocguard_refcount;

  ScallocGuard();
  ~ScallocGuard();
};

}  // namespace scalloc

#endif  // SCALLOC_SCALLOC_GUARD_H_

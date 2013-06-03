// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_RUNTIME_VARS_H_
#define SCALLOC_RUNTIME_VARS_H_

#include <stddef.h>  // size_t

#include "common.h"

class RuntimeVars {
 public:
  static void InitModule();

  static size_t SystemPageSize();
  static size_t Cpus();

 private:
  static RuntimeVars vars_;

  size_t system_page_size_;
  size_t number_cpus_;
} cache_aligned;

inline size_t RuntimeVars::SystemPageSize() {
  return vars_.system_page_size_;
}

inline size_t RuntimeVars::Cpus() {
  return vars_.number_cpus_;
}

#endif  // SCALLOC_RUNTIME_VARS_H_

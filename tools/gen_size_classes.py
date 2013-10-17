#!/usr/bin/env python


min_alignment = 16
max_fine_shift = 8
max_fine_size = 1 << max_fine_shift
max_small_shift = 21
max_small_size = 1 << max_small_shift

fine_classes = max_fine_size / min_alignment + 1
coarse_classes = 21 - 8
num_classes = fine_classes + coarse_classes

class_to_size = [0]

lines = []

lines.append("  SIZE_CLASS(0, 0, 0, 0) \\")

span_size = 1 << 13
for i in range(1, fine_classes/2):
  class_to_size.append(class_to_size[i-1] + min_alignment)
  lines.append("  SIZE_CLASS({0}, {1}, {2}, {3}) \\".format(
    i,
    class_to_size[i],
    span_size,
    "({0} - sizeof(SpanHeader))/{1}".format(span_size, class_to_size[i])
  ))

span_size = 1 << 14
for i in range(fine_classes/2, fine_classes):
  class_to_size.append(class_to_size[i-1] + min_alignment)
  lines.append("  SIZE_CLASS({0}, {1}, {2}, {3}) \\".format(
    i,
    class_to_size[i],
    span_size,
    "({0} - sizeof(SpanHeader))/{1}".format(span_size, class_to_size[i])
  ))

objs = [
  1 << 6,
  1 << 5,
  1 << 4,
  1 << 3,
  1 << 3,
  1 << 3,
  1 << 3,
  1 << 2,
  1 << 2,
  1 << 2,
  1 << 2,
  1 << 1,
  1 << 0,
]

large = []
for i in range(fine_classes, num_classes):
  class_to_size.append(class_to_size[i-1] * 2)
  large.append("  SIZE_CLASS({0}, {1}, {2}, {3})".format(
    i,
    class_to_size[i],
    "(({0} * {1} + sizeof(SpanHeader))/kPageSize + 1) * kPageSize".format(objs[i
      - fine_classes], class_to_size[i]),
    objs[i - fine_classes]
  ))
lines.append(" \\\n".join(large))

f = open("src/size_classes_raw.h", "wt")

f.write(
"""// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

// This file is auto-generated using ``python tools/gen_size_classes.py''

#ifndef SCALLOC_SIZE_CLASSES_RAW_H_
#define SCALLOC_SIZE_CLASSES_RAW_H_

#define SIZE_CLASSES \\
{size_classes}

#endif  // SCALLOC_SIZE_CLASSES_RAW_H_
""".format(
  size_classes="\n".join(lines),
))

f.close()

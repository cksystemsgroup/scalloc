#!/usr/bin/env python

# Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
# Please see the AUTHORS file for details.  Use of this source code is governed
# by a BSD license that can be found in the LICENSE file.

import os
import sys

# The size class with index 0 corresponds to a size of 0 bytes.
# Size class format:
#     (index, size, real span size, #objects)
class SizeClass:
  def __init__(self, index, size, real_span, objs):
    self.size = size
    self.index = index
    self.real_span = real_span
    self.objs = objs

  def __str__(self):
    return "  SIZE_CLASS({0}, {1}, {2}, {3}) /* NOLINT */".format(
      self.index, self.size, self.real_span, self.objs)

min_alignment = 16
max_fine_shift = 8
max_fine_size = 1 << max_fine_shift
max_small_shift = 21
max_small_size = 1 << max_small_shift
fine_classes = max_fine_size / min_alignment + 1
coarse_classes = max_small_shift - max_fine_shift
num_classes = fine_classes + coarse_classes

def generate_size_classes():
  class_to_size = [ 0 ]
  size_classes = [ SizeClass(0, 0, 0, 0) ]

  # For the first half offine classes we use real spans of 4 pages.
  span_size = 1 << 14
  fine = []
  for i in range(1, fine_classes/2):
    class_to_size.append(class_to_size[i-1] + min_alignment)
    fine.append(SizeClass(
      i, class_to_size[i], span_size, 
      "({0} - sizeof(SpanHeader))/{1}".format(span_size, class_to_size[i])))
  size_classes.extend(fine)

  # For the next half we switch use real spans of 8 pages.
  span_size = 1 << 15
  fine = []
  for i in range(fine_classes/2, fine_classes):
    class_to_size.append(class_to_size[i-1] + min_alignment)
    fine.append(SizeClass(
      i, class_to_size[i], span_size, 
      "({0} - sizeof(SpanHeader))/{1}".format(span_size, class_to_size[i])))
  size_classes.extend(fine)

  # For coarse grained objects we derive the span size from the number of
  # objects.
  objs = [ 1 << 6, 1 << 5, 1 << 4, 1 << 3, 1 << 3, 1 << 3, 1 << 3, 1 << 2,
           1 << 2, 1 << 2, 1 << 2, 1 << 1, 1 << 0 ]
  coarse = []
  for i in range(fine_classes, num_classes):
    class_to_size.append(class_to_size[i-1] * 2)
    coarse.append(SizeClass(
      i, class_to_size[i],
      "(({0} * {1} + sizeof(SpanHeader))/kPageSize + 1) * kPageSize".format(
          objs[i - fine_classes], class_to_size[i]),
      objs[i - fine_classes]))
  size_classes.extend(coarse)
  return size_classes


def generate_huge_size_classes():
  size_classes =  [ SizeClass(0, 0, 0, 0) ]
  sz_range = [ 0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224,
      240, 256, 512, 1024, 2048, 4096, 8192, 16384 ]
  for i in range(1, len(sz_range)):
    size_classes.append(SizeClass(
      i, sz_range[i],
      "HUGEPAGE_SIZE",
      "((HUGEPAGE_SIZE - sizeof(SpanHeader))/{0})".format(sz_range[i])))
  return size_classes


def write_size_classes(file_name, size_classes, cmd_args):
  guard = os.path.basename(file_name).replace(".", "_").upper()
  text = """\
// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

//
//                          +----------------+
//                          |  DO NOT EDIT!  |
//                          +----------------+
//
// This file is auto-generated using ``tools/gen_size_classes.py {1}''

#ifndef SCALLOC_{2}_
#define SCALLOC_{2}_

#define SIZE_CLASSES \\
{0}

#endif  // SCALLOC_{2}_
""".format(" \\\n".join(str(sz) for sz in size_classes), cmd_args, guard)
  f = open(file_name, "wt")
  f.write(text)
  f.close()


def print_usage():
  pass


def main():
  if len(sys.argv) > 2:
    print_usage()
    sys.exit(-1)

  if len(sys.argv) == 1 :
    size_classes = generate_size_classes()
    write_size_classes("src/size_classes_raw.h", size_classes, "huge")
  elif len(sys.argv) == 2 and sys.argv[1] == 'huge':
    size_classes = generate_huge_size_classes()
    write_size_classes("src/size_classes_raw_hugepage.h", size_classes, "huge")
  else:
    print_usage()
    sys.exit(-1)

  sys.exit(0)

if __name__ == '__main__':
  main()


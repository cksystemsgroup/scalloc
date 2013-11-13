{
  'variables': {
    'default_cflags' : [
      '-Wall',
      '-Werror',
      '-fPIC',
      '-m64',
      '-std=c++11',
      '-fno-omit-frame-pointer',
      '-ffast-math'
    ],
    'default_ldflags': [
    ],
    'heap_profile%': 0,
    'log_level%': "kWarning",
    'span_reuse_threshold%': 80,
    'local_reuse_threshold%': 80,
    'small_space%': -1,
    'eager_madvise_threshold%': -1,
    'madvise_strategy%': "same-thread",
  },
  'target_defaults': {
    'product_dir': "out/<(CONFIGURATION_NAME)",
    'configurations': {
      'Debug': {
        'cflags': ['-g', '-O0'],
        'xcode_settings': {
          'OTHER_CFLAGS': ['-g', '-O0']
        },
        'defines': [
          'DEBUG',
        ]
      },
      'Release': {
        'cflags': ['-O3'],
        'xcode_settings': {
          'OTHER_CFLAGS': ['-O3']
        },
      }
    },
  },
  'targets': [
    {
      'target_name': 'scalloc',
      'product_name': 'scalloc',
      'type' : 'shared_library',
      'cflags': [ '<@(default_cflags)' ],
      'ldflags': [ '<@(default_ldflags)' ],
      'xcode_settings': {
        'OTHER_CFLAGS': [ '<@(default_cflags)' ],
        'OTHER_LDFLAGS': [ '<@(default_ldflags)' ],
        'CLANG_CXX_LANGUAGE_STANDARD': "c++0x",
        'CLANG_CXX_LIBRARY': "libc++",
      },
      'defines': [
        'LOG_LEVEL=<(log_level)',
        'SPAN_REUSE_THRESHOLD=<(span_reuse_threshold)',
        'LOCAL_REUSE_THRESHOLD=<(local_reuse_threshold)',
      ],
      'conditions': [
        ['OS=="linux"', {
          'ldflags': [
            '-pthread'
          ]
        }],
        ['<(small_space)!=-1', {
          'defines': [
            'SMALL_SPACE=<(small_space)'
          ]
        }],
        ['<(eager_madvise_threshold)!=-1', {
          'defines': [
            'EAGER_MADVISE_THRESHOLD=<(eager_madvise_threshold)'
          ]
        }],
        ['"<(madvise_strategy)"=="same-thread"', {
          'defines': [
            'MADVISE_SAME_THREAD'
          ]
        }, {
          'defines': [
            'MADVISE_SEPARATE_THREAD'
          ]
        }],
        ['<(heap_profile)!=0', {
          'defines': [
            'HEAP_PROFILE'
          ]
        }],
      ],
      'sources': [
        'src/allocators/arena.cc',
        'src/allocators/arena.h',
        'src/allocators/block_pool.cc',
        'src/allocators/block_pool.h',
        'src/allocators/large_allocator.h',
        'src/allocators/small_allocator.cc',
        'src/allocators/small_allocator.h',
        'src/allocators/span_pool.cc',
        'src/allocators/span_pool.h',
        'src/assert.h',
        'src/atomic.h',
        'src/block_header.h',
        'src/collector.cc',
        'src/collector.h',
        'src/common.h',
        'src/distributed_queue.cc',
        'src/distributed_queue.h',
        'src/freelist-inl.h',
        'src/heap_profiler.cc',
        'src/heap_profiler.h',
        'src/log.cc',
        'src/log.h',
        'src/override.h',
        'src/override_osx.h',
        'src/override_gcc_weak.h',
        'src/platform.h',
        'src/profiler.cc',
        'src/profiler.h',
        'src/random.h',
        'src/scalloc.cc',
        'src/scalloc.h',
        'src/scalloc_arenas.h',
        'src/scalloc_guard.h',
        'src/size_classes_raw.h',
        'src/size_classes.h',
        'src/spinlock-inl.h',
        'src/stack-inl.h',
        'src/system-alloc.cc',
        'src/system-alloc.h',
        'src/thread_cache.cc',
        'src/thread_cache.h',
        'src/typed_allocator.h',
        'src/utils.cc',
        'src/utils.h'
      ],
      'include_dirs': [
        'src',
        'src/allocators',
      ]
    },
  ],
}

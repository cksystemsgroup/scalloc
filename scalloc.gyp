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
    'enable_slow_span_reuse%': -1,
    'enable_free_list_reuse%': -1,
    'core_local%': -1,
  },
  'target_defaults': {
    'configurations': {
      'Debug': {
        'cflags': [ '<@(default_cflags)' , '-g -gdwarf-2', '-O0'  ],
        'ldflags': [ '<@(default_ldflags)' ],
        'xcode_settings': {
          'BUILT_PRODUCTS_DIR': "out/<(CONFIGURATION_NAME)",
          'OTHER_CFLAGS': [ '<@(default_cflags)' , '-g', '-O0'],
          'OTHER_LDFLAGS': [ '<@(default_ldflags)' ],
          'USE_HEADERMAP': 'NO',
          'CLANG_CXX_LANGUAGE_STANDARD': "c++0x",
          'CLANG_CXX_LIBRARY': "libc++",
        },
        'defines': [
          'DEBUG',
        ]
      },
      'Release': {
        'cflags': [ '<@(default_cflags)', '-O3' ],
        'ldflags': [ '<@(default_ldflags)' ],
        'xcode_settings': {
          'BUILT_PRODUCTS_DIR': "out/<(CONFIGURATION_NAME)",
          'OTHER_CFLAGS': [ '<@(default_cflags)', '-O3' ],
          'OTHER_LDFLAGS': [ '<@(default_ldflags)' ],
          'USE_HEADERMAP': 'NO',
          'CLANG_CXX_LANGUAGE_STANDARD': "c++0x",
          'CLANG_CXX_LIBRARY': "libc++",
        },
      }
    },
  },
  'conditions': [
    ['OS=="linux"', {
      'target_defaults': {
        'cflags': [ '-mcx16' ]
      }
    }]
  ],
  'targets': [
    {
      'target_name': 'scalloc',
      'product_name': 'scalloc',
      'type' : 'shared_library',
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
        ['<(enable_slow_span_reuse)!=-1', {
          'defines': [
            'REUSE_SLOW_SPANS'
          ]
        }],
        ['<(enable_free_list_reuse)!=-1', {
          'defines': [
            'REUSE_FREE_LIST'
          ]
        }],
        ['<(core_local)!=-1', {
          'defines': [
            'POLICY_CORE_LOCAL'
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
        'src/allocators/large-inl.h',
        'src/allocators/scalloc_core-inl.h',
        'src/allocators/span_pool.cc',
        'src/allocators/span_pool.h',
        'src/assert.h',
        'src/atomic.h',
        'src/buffer/core.cc',
        'src/buffer/core.h',
        'src/collector.cc',
        'src/collector.h',
        'src/common.h',
        'src/distributed_queue.cc',
        'src/distributed_queue.h',
        'src/fast_lock.h',
        'src/freelist-inl.h',
        'src/headers.h',
        'src/lock_utils-inl.h',
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
        'src/size_classes.cc',
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

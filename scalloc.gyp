{
  'includes': [
    'common.gypi',
  ],
  'variables': {
    'profile%': 0,
    'log_level%': "kWarning",
    'span_reuse_threshold%': 80,
    'local_reuse_threshold%': 80,
    'small_space%': -1,
    'eager_madvise_threshold%': -1,
    'incremental_freelist%': 1,
    'enable_slow_span_reuse%': -1,
    'enable_free_list_reuse%': -1,
    'core_local%': -1,
    'unit_tests%': -1,
    'max_parallelism%': -1,
    'dq_backend%': "-1",
    'dq_non_lin_empty%': -1,
    'huge_pages%': -1,
    'huge_page_space%': -1,
    'clab_policy%': "utilization",
    'clab_threshold%': -1,
    'size_classes_1mb%': -1,
    'size_class_config%': "default",
  },
  'conditions': [
    ['<(unit_tests)!=-1', {
      'includes': [
        'unittests.gypi'
      ]
    }],
  ],
  'targets': [
    {
      'target_name': 'scalloc',
      'product_name': 'scalloc',
      'type' : 'shared_library',
      'defines': [
        'SIZE_CLASS_CONFIG=<(size_class_config)',
        'LOG_LEVEL=<(log_level)',
        'SPAN_REUSE_THRESHOLD=<(span_reuse_threshold)',
        'LOCAL_REUSE_THRESHOLD=<(local_reuse_threshold)',
      ],
      'conditions': [
        ['OS=="linux"', {
          'ldflags': [ '-pthread' ],
          'cflags': [ '-mcx16' ],
          'sources': [
            'src/huge_page.cc',
            'src/huge_page.h',
          ],
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
        ['<(max_parallelism)!=-1', {
          'defines': [
            'MAX_PARALLELISM=<(max_parallelism)'
          ]
        }],
        ['"<(dq_backend)"!="-1"', {
          'defines': [
            'BACKEND_TYPE=<(dq_backend)'
          ]
        }],
        ['<(dq_non_lin_empty)!=-1', {
          'defines': [
            'DQ_NON_LIN_EMPTY'
          ]
        }],
        ['<(incremental_freelist)!=-1', {
          'defines': [
            'INCREMENTAL_FREELIST'
          ]
        }],
        ['<(core_local)!=-1', {
          'defines': [
            'POLICY_CORE_LOCAL'
          ],
          'sources': [
            'src/buffer/lab.cc',
            'src/buffer/lab.h',
          ]
        }],
        ['<(profile)!=0', {
          'defines': [
            'PROFILER'
          ]
        }],
        ['<(huge_pages)!=-1', {
          'defines': [
            'HUGE_PAGE'
          ]
        }],
        ['<(huge_page_space)!=-1', {
          'defines': [
            'HUGE_PAGE_SPACE=<(huge_page_space)'
          ]
        }],
        ['"<(clab_policy)"=="utilization"', {
          'defines': [
            'CLAB_UTILIZATION'
          ]
        }],
        ['"<(clab_policy)"=="threads"', {
          'defines': [
            'CLAB_THREADS'
          ]
        }],
        ['"<(clab_policy)"=="rr"', {
          'defines': [
            'CLAB_RR'
          ]
        }],
        ['<(clab_threshold)!=-1', {
          'defines': [
            'CLAB_THRESHOLD=<(clab_threshold)'
          ]
        }],
        ['<(size_classes_1mb)!=-1', {
          'defines': [
            'SZ_1MB'
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
        'src/allocators/scalloc_core.cc',
        'src/allocators/span_pool.cc',
        'src/allocators/span_pool.h',
        'src/allocators/typed_allocator.h',
        'src/atomic.h',
        'src/buffer/lab.cc',
        'src/buffer/lab.h',
        'src/config/default_size_classes_raw.h',
        'src/config/default_globals.h',
        'src/config/hugepage_size_classes_raw.h',
        'src/config/hugepage_globals.h',
        'src/config/1mb_size_classes_raw.h',
        'src/config/1mb_globals.h',
        'src/common.h',
        'src/distributed_queue.cc',
        'src/distributed_queue.h',
        'src/freelist-inl.h',
        'src/headers.h',
        'src/huge_page.cc',
        'src/huge_page.h',
        'src/lock_linux-inl.h',
        'src/lock_utils-inl.h',
        'src/log.cc',
        'src/log.h',
        'src/platform/assert.h',
        'src/platform/override.h',
        'src/platform/override_osx.h',
        'src/platform/override_gcc_weak.h',
        'src/platform.h',
        'src/profiler.h',
        'src/random.h',
        'src/scalloc.cc',
        'src/scalloc.h',
        'src/scalloc_arenas.h',
        'src/size_classes_raw.h',
        'src/size_classes.cc',
        'src/size_classes.h',
        'src/spinlock-inl.h',
        'src/stack-inl.h',
        'src/thread_cache.cc',
        'src/thread_cache.h',
        'src/utils.cc',
        'src/utils.h'
      ],
      'include_dirs': [
        'src',
        'src/allocators',
        'src/config',
      ]
    },
  ],
}

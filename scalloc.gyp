{
  'includes': [
    'common.gypi',
  ],
  'variables': {
    'log_level%': "kWarning",
    'reuse_threshold%': "80",
    'lab_model%': "SCALLOC_LAB_MODEL_TLAB",
    'madvise%': 'yes',
    'madvise_eager%': 'yes',
    'span_pool_backend_limit%': 'cpu',
    'cleanup_in_free%': 'yes',
  },
  'conditions': [
  ],
  'targets': [
    {
      'target_name': 'scalloc',
      'product_name': 'scalloc',
      'type' : 'shared_library',
      'defines': [
        'SCALLOC_LOG_LEVEL=<(log_level)',
        'SCALLOC_REUSE_THRESHOLD=<(reuse_threshold)',
        'SCALLOC_LAB_MODEL=<(lab_model)',
      ],
      'conditions': [
        ['OS=="linux"', {
          'ldflags': [ '-pthread' ],
          'libraries': ['-ldl'],
          'cflags': [ '-mcx16' ],
          'sources': [
            'src/platform/pthread_intercept.cc'
          ]
        }],
        ['"no"=="<(madvise)"', {
          'defines': [
            'SCALLOC_NO_MADVISE'
          ]
        }],
        ['"no"=="<(madvise_eager)"', {
          'defines': [
            'SCALLOC_NO_MADVISE_EAGER'
          ]
        }],
        ['"cpu"!="<(span_pool_backend_limit)"', {
          'defines': [
            'SCALLOC_SPAN_POOL_BACKEND_LIMIT=<(span_pool_backend_limit)'
          ]
        }],
        ['"yes"!="<(cleanup_in_free)"', {
          'defines': [
            'SCALLOC_NO_CLEANUP_IN_FREE'
          ]
        }],
      ],
      'sources': [
        'src/arena.h',
        'src/globals.h',
        'src/core.h',
        'src/lab.h',
        'src/log.h',
        'src/glue.h',
        'src/glue.cc',
        'src/platform/assert.h',
        'src/platform/globals.h',
        'src/platform/override.h',
        'src/platform/override_gcc_weak.h',
        'src/platform/override_osx.h',
        'src/platform/pthread_intercept.h',
        'src/platform/pthread_intercept.cc',
        'src/size_classes.h',
        'src/span.h',
        'src/span_pool.h',
        'src/utils.h'
      ],
      'include_dirs': [
        'src',
      ]
    },
  ],
}

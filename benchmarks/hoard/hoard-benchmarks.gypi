{
  'targets': [
    {
      'target_name': 'threadtest',
      'product_name': 'threadtest',
      'type' : 'executable',
      'conditions': [
        ['OS=="linux"', {
          'ldflags': [
            '-pthread'
          ]
        }],
      ],
      'sources': [
        'threadtest/threadtest.cpp',
      ],
      'include_dirs': [
        'common',
      ]
    },
    {
      'target_name': 'larson',
      'product_name': 'larson',
      'type' : 'executable',
      'cflags!': [ '-Wall', '-Werror' ],
      'conditions': [
        ['OS=="linux"', {
          'ldflags': [
            '-pthread'
          ]
        }],
      ],
      'sources': [
        'larson/larson.cpp',
      ],
      'include_dirs': [
        'common',
      ]
    },
    {
      'target_name': 'cache-scratch',
      'product_name': 'cache-scratch',
      'type' : 'executable',
      'cflags!': [ '-Wall', '-Werror' ],
      'conditions': [
        ['OS=="linux"', {
          'ldflags': [
            '-pthread'
          ]
        }],
      ],
      'sources': [
        'cache-scratch/cache-scratch.cpp',
      ],
      'include_dirs': [
        'common',
      ]
    },
    {
      'target_name': 'cache-thrash',
      'product_name': 'cache-thrash',
      'type' : 'executable',
      'cflags!': [ '-Wall', '-Werror' ],
      'conditions': [
        ['OS=="linux"', {
          'ldflags': [
            '-pthread'
          ]
        }],
      ],
      'sources': [
        'cache-thrash/cache-thrash.cpp',
      ],
      'include_dirs': [
        'common',
      ]
    }
  ]
}

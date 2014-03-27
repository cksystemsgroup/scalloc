{
  'targets' : [ 
    {
      'target_name': 'utils_test',
      'type': 'executable',
      'dependencies': [
        'third_party/gtest.gypi:gtestmain',
      ],
      'sources': [
        'test/utils_unittest.cc',
      ],
      'include_dirs': [
        'src',
      ],
      'conditions': [
        ['OS=="linux"', {
          'ldflags': [
            '-pthread'
          ]
        }],
      ],
    },
    {
      'target_name': 'atomic_test',
      'type': 'executable',
      'dependencies': [
        'third_party/gtest.gypi:gtestmain',
      ],
      'sources': [
        'test/atomic_unittest.cc',
      ],
      'include_dirs': [
        'src',
      ],
      'conditions': [
        ['OS=="linux"', {
          'ldflags': [
            '-pthread'
          ]
        }],
      ],
    },
    {
      'target_name': 'freelist_test',
      'type': 'executable',
      'dependencies': [
        'third_party/gtest.gypi:gtestmain',
      ],
      'sources': [
        'src/log.cc',
        'src/size_classes.cc',
        'test/freelist_unittest.cc',
      ],
      'include_dirs': [
        'src',
      ],
      'conditions': [
        ['OS=="linux"', {
          'ldflags': [
            '-pthread'
          ]
        }],
      ],
    },
  ],
}

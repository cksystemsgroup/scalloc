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
  ],
}

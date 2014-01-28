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
        'threadtest.cpp',
      ],
      'include_dirs': [
        'common',
      ]
    }
  ]
}

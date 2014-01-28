{
  'targets': [
    {
      'target_name': 'thread-termination',
      'product_name': 'thread-termination',
      'type' : 'executable',
      'defines': [
      ],
      'conditions': [
        ['OS=="linux"', {
          'ldflags': [
            '-pthread'
          ]
        }],
      ],
      'sources': [
        'thread-termination.cc',
      ],
    }
  ]
}

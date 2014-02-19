{
  'targets': [
    {
      'target_name': 'shbench',
      'product_name': 'shbench',
      'type' : 'executable',
      'defines': [
        'MALLOC_ONLY',
        'SYS_MULTI_THREAD'
      ],
      'conditions': [
        ['OS=="linux"', {
          'ldflags': [
            '-Wl,--no-as-needed',
            '-pthread',
            '-lrt',
          ]
        }],
      ],
      'sources': [
        'sh6bench.c',
        'smrtheap.h'
      ],
    },
  ]
}

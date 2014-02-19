{
  'variables': {
    'shbench%': "no",
  },
  'target_defaults': {
    'product_dir': 'out/<(CONFIGURATION_NAME)',
    'configurations': {
      'Debug': {
        'xcode_settings': {
          'OTHER_CFLAGS': [ '-g', '-gdwarf-2', '-O0'],
        }
      },
      'Release': {
        'xcode_settings': {
          'OTHER_CFLAGS': [ '-O3'],
        }
      }
    }
  },
  'conditions': [
    ['"<(shbench)"!="no" and OS!="mac"', {
      'includes': [
        'shbench/shbench.gypi'
      ]
    }]
  ],
  'includes': [
    'hoard/hoard-benchmarks.gypi',
    'thread-termination/thread-termination.gypi',
  ]
}

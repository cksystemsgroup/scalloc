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
}

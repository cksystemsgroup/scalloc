{
  'variables': {
    'default_cflags' : [
      '-Wall',
      '-Werror',
      '-fPIC',
      '-m64',
      '-std=c++11',
      '-fno-omit-frame-pointer',
      '-ffast-math',
      '-fno-exceptions',
      '-fno-rtti',
      '-mtune=native',
      '-ftls-model=initial-exec',
    ],
    'default_ldflags': [
    ],
  },
  'xcode_settings': {
    'SYMROOT': '<(DEPTH)/out',
  },
  'target_defaults': {
    'configurations': {
      'Base': {
        'abstract': 1,
        'xcode_settings': {
          'USE_HEADERMAP': 'NO',
          'CLANG_CXX_LANGUAGE_STANDARD': "c++0x",
          'CLANG_CXX_LIBRARY': "libc++",
          'GCC_TREAT_WARNINGS_AS_ERRORS': 'YES', # -Werror
          'GCC_ENABLE_CPP_EXCEPTIONS': 'NO', # -fno-exceptions
          'GCC_ENABLE_CPP_RTTI': 'NO', # -fno-rtti
          'ONLY_ACTIVE_ARCH': 'YES',
        }
      },
      'Debug': {
        'inherit_from': ['Base'],
        'cflags': [ '<@(default_cflags)' , '-g -gdwarf-2', '-O0'  ],
        'ldflags': [ '<@(default_ldflags)' ],
        'xcode_settings': {
          'OTHER_CFLAGS': [ '<@(default_cflags)' , '-g', '-O0'],
          'OTHER_LDFLAGS': [ '<@(default_ldflags)' ],
        },
        'defines': [
          'DEBUG',
        ]
      },
      'Release': {
        'inherit_from': ['Base'],
        'cflags': [ '<@(default_cflags)', '-O3' ],
        'ldflags': [ '<@(default_ldflags)' ],
        'xcode_settings': {
          'OTHER_CFLAGS': [ '<@(default_cflags)', '-O3' ],
          'OTHER_LDFLAGS': [ '<@(default_ldflags)' ],
        },
      },
      'Release-check': {
        'inherit_from': ['Base'],
        'cflags': [ '<@(default_cflags)', '-O3', '-g -gdwarf-2' ],
        'defines': ['PROFILE' ],
        'ldflags': [ '<@(default_ldflags)' ],
        'xcode_settings': {
          'OTHER_CFLAGS': [ '<@(default_cflags)', '-O3' ],
          'OTHER_LDFLAGS': [ '<@(default_ldflags)' ],
        },
      }
    },
  },
}

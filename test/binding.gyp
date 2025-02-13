{
  'variables': {
    'enable_asan%': 'false',
    'enable_typescript%': 'false'
  },
  'targets': [
    {
      'target_name': '<(test)',
      'sources': [ 'tests/<(test).cc', '<@(fixtures)' ],
      # RTTI is only for easier debugging of the templates
      'cflags!': [ '-fno-rtti' ],
      'cflags_cc!': [ '-fno-rtti' ],
      'cflags_cc': [ '-frtti' ],
      'xcode_settings': {
        'OTHER_CPLUSPLUSFLAGS': [
          '-frtti'
        ]
      },
      'include_dirs': [
        '.',
        '../include',
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      'dependencies': ["<!(node -p \"require('node-addon-api').gyp\")"],
    }
  ],
  'target_defaults': {
    'includes': [ '../except.gypi' ],
    'cflags': [
      '-fvisibility=hidden',
      '-std=c++17'
    ],
    'msvs_settings': {
      'VCCLCompilerTool': { 
        'AdditionalOptions': [ '/std:c++17', '/permissive-' ]
      }
    },
    'xcode_settings': {
      'OTHER_CPLUSPLUSFLAGS': [
        '-fvisibility=hidden',
        '-std=c++17'
      ]
    },
    'conditions': [
      ['enable_asan == "true"', {
        'cflags': [
          '-fsanitize=address'
        ],
        'xcode_settings': {
          'OTHER_CPLUSPLUSFLAGS': [
            '-fsanitize=address'
          ]
        }        
      }],
      ['enable_typescript == "true"', {
        'defines': [ 'NOBIND_TYPESCRIPT_GENERATOR=1' ]      
      }]
    ]
  }
}

{
  'variables': {
    'enable_asan%': 'false',
    'enable_typescript%': 'true',
    'enable_typescript_debug%': 'false',
    'enable_object_store%': 'true',
    'enable_require_basic_finalizers%': 'false',
    'test_output%': '<(test)'
  },
  'targets': [
    {
      'target_name': '<(test_output)',
      'sources': [ 'tests/<(test).cc', '<@(fixtures)' ],
      # RTTI is only for easier debugging of the templates
      'cflags!': [ '-fno-rtti' ],
      'cflags_cc!': [ '-fno-rtti' ],
      'cflags_cc': [ '-frtti' ],
      'xcode_settings': {
        'OTHER_CPLUSPLUSFLAGS': [
          '-frtti',
          '-ftemplate-backtrace-limit=0'
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
      ['enable_object_store == "false"', {
        'defines': [ 'NOBIND_NO_OBJECT_STORE' ]
      }],
      ['enable_require_basic_finalizers == "true"', {
        'defines': [ 'NODE_ADDON_API_REQUIRE_BASIC_FINALIZERS' ]
      }],
      ['enable_typescript == "false"', {
        'defines': [ 'NOBIND_NO_TYPESCRIPT_GENERATOR' ]      
      }],
      ['enable_typescript_debug == "true"', {
        'defines': [ 'NOBIND_TYPESCRIPT_DEBUG' ]      
      }]
    ]
  }
}

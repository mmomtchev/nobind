{
  'targets': [
    {
      'target_name': '<(test)',
      'sources': [ 'tests/<(test).cc', '<@(fixtures)' ],
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
    'cflags': [ '-fvisibility=hidden', '-std=c++17' ],
    'msvs_settings': {
      'VCCLCompilerTool': { 
        'AdditionalOptions': [ '/std:c++17' ]
      }
    },
    'xcode_settings': {
      'OTHER_CPLUSPLUSFLAGS': [
        '-std=c++17'
      ]
    }
  }
}

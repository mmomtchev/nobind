{
  'targets': [
    {
      'target_name': 'nobind',
      'sources': [ 'nobind.cc' ],
      'include_dirs': [
        '../include'
      ],
    },
    {
      'target_name': 'napi',
      'sources': [ 'napi.cc' ]
    },
    {
      'target_name': 'swig',
      'sources': [ 'swig_wrap.cxx' ]
    }
  ],
  'target_defaults': {
    'includes': [ '../except.gypi' ],
    'sources': [ 'string.cc' ],
    'dependencies': ["<!(node -p \"require('node-addon-api').gyp\")"],
    'include_dirs': [
      "<!@(node -p \"require('node-addon-api').include\")"
    ],
    'cflags': [
      '-fvisibility=hidden',
      '-std=c++17'
    ],
    'msvs_settings': {
      'VCCLCompilerTool': { 
        'AdditionalOptions': [ '/std:c++17' ]
      }
    },
    'xcode_settings': {
      'OTHER_CPLUSPLUSFLAGS': [
        '-fvisibility=hidden',
        '-std=c++17'
      ]
    }
  }
}

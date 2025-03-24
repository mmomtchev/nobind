{
  'target_defaults': {
    'includes': [
      # This comes from node_modules/node-addon-api
      'except.gypi'
    ]
  },
  'targets': [
    {
      'target_name': 'hello',
      'sources': [ 'hello.cc', 'monster.cc' ],
      'include_dirs': [
        '<!@(node -p "require(\'node-addon-api\').include")',
        '<!@(node -p "require(\'nobind17\').include")'
      ],
      'configurations': {
        'Debug': {
          # RTTI is only for easier debugging of the templates
          'cflags!': [ '-fno-rtti' ],
          'cflags_cc!': [ '-fno-rtti' ],
          'cflags_cc': [ '-frtti' ],
          'xcode_settings': {
            'OTHER_CPLUSPLUSFLAGS': [
              '-frtti',
              '-ftemplate-backtrace-limit=0'
            ]
          }
        },
        'Release': {}
      }
    },
    {
      'target_name': 'action_after_build',
      'type': 'none',
      'dependencies': [ 'hello' ],
      'copies': [
        {
          'files': [
            '<(PRODUCT_DIR)/hello.node'
          ],
          'destination': 'lib'
        }
      ]
    },
    {
      'target_name': 'hello.d.ts',
      'type': 'none',
      'dependencies': [ 'hello' ],
      'actions': [
        {
          'action_name': 'typescript_bindings',
          'inputs': [ '<(PRODUCT_DIR)/hello.node' ],
          'outputs': [ 'lib/hello.d.ts' ],
          'action': [
            'node',
            'gen_typescript.js',
            '<@(_inputs)',
            '<@(_outputs)'
          ]
        }
      ]
    }
  ]
}

{
  'target_defaults': {
    'includes': [
      # This comes from node_modules/node-addon-api
      'except.gypi'
    ]
  },
  'targets': [
    {
      'target_name': 'example',
      'sources': [ 'hello.cc' ],
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
    # This target copies either build/Debug/module.node
    # or build/Release/module.node to ./lib
    {
      'target_name': 'action_after_build',
      'type': 'none',
      'dependencies': [ 'example' ],
      'copies': [
        {
          'files': [
            '<(PRODUCT_DIR)/example.node'
          ],
          'destination': 'lib'
        }
      ]
    },
    # This target extracts the TypeScript type
    # definitions
    {
      'target_name': 'example.d.ts',
      'type': 'none',
      'dependencies': [ 'example' ],
      'actions': [
        {
          'action_name': 'typescript_bindings',
          'inputs': [ '<(PRODUCT_DIR)/example.node' ],
          'outputs': [ 'lib/example.d.ts' ],
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

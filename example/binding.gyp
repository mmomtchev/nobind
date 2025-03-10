{
  'target_defaults': {
    'includes': [
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
          'outputs': [ '<(PRODUCT_DIR)/hello.d.ts' ],
          'action': [
            'node',
            '--eval=\'fs.writeFileSync("<(PRODUCT_DIR)/hello.d.ts", require("<(PRODUCT_DIR)/hello.node").__typescript)\''
          ]
        }
      ]
    }
  ]
}

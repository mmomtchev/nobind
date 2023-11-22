{
  'target_defaults': {
    'includes': [
      'except.gypi'
    ]
  },
  'targets': [
    {
      'target_name': 'hello',
      'sources': [ 'hello.cc' ],
      'include_dirs': [
        '<!@(node -p "require(\'node-addon-api\').include")',
        '<!@(node -p "require(\'nobind17\').include")'
      ]
    }
  ]
}

{
  'target_defaults': {
    'includes': [
      'except.gypi'
    ]
  },
  'targets': [
    {
      'target_name': 'test_package',
      'sources': [ 'test_package.cc' ],
      'include_dirs': [
        '<!@(node -p "require(\'node-addon-api\').include")',
        '<!@(node -p "require(\'nobind\').include")'
      ]
    }
  ]
}

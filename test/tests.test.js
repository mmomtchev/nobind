const framework = require('./framework');
const { mocha_typescript } = require('./opts');

describe('nobind', function () {
  const tests = framework.list();
  for (const t of tests) {
    describe(t, () => {
      before('configure', () => framework.configure(t, undefined, process.env.ENABLE_ASAN && ['--debug', '--enable_asan'] || []));
      before('build', () => framework.build());
      before('load', () => framework.load(t, process.env.ENABLE_ASAN && 'Debug'));
      if (mocha_typescript()) {
        before('generate types', () => framework.gen_typescript());
        it('check types', () => framework.check_typescript(t));
      }
      framework.register(t);
      after('GC', global.gc);
    });
  }
});

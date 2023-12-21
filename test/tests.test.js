const framework = require('./framework');

describe('nobind', function () {
  const tests = framework.list();
  for (const t of tests) {
    describe(t, () => {
      framework.register(t);
      before('clean', () => framework.clean(t));
      before('configure', () => framework.configure(t, undefined, process.env.ENABLE_ASAN && ['--debug', '--enable_asan']));
      before('build', () => framework.build());
      before('load', () => framework.load(t, process.env.ENABLE_ASAN && 'Debug'));
      after('GC', global.gc);
    });
  }
});

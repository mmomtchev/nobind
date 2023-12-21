const framework = require('./framework');

describe('nobind', function () {
  const tests = framework.list();
  for (const t of tests) {
    describe(t, () => {
      framework.register(t);
      try {
        before('clean', () => framework.clean(t));
      } catch (e) {
        console.log('cleaning the build failed, module not unloaded on Windows?');
      }
      before('configure', () => framework.configure(t, undefined, process.env.ENABLE_ASAN && ['--debug', '--enable_asan']));
      before('build', () => framework.build());
      before('load', () => framework.load(t, process.env.ENABLE_ASAN && 'Debug'));
      after('GC', global.gc);
    });
  }
});

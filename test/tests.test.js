const framework = require('./framework');
const { mocha_typescript } = require('./opts');

describe('nobind', function () {
  const tests = framework.list();
  for (const t of tests) {
    describe(t, () => {
      before('configure', () => framework.configure(t, undefined, process.env.ENABLE_ASAN && ['--debug', '--enable_asan'] || ['--debug']));
      before('build', () => framework.build());
      before('load', () => framework.load(t, 'Debug'));
      if (mocha_typescript()) {
        before('generate types', () => framework.gen_typescript());
        it('check types', function () {
          this.repeats(10);
          framework.check_typescript(t);
        });
      }
      framework.register(t);
      after('GC', global.gc);
    });
  }
});

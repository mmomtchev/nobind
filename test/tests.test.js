const framework = require('./framework');

describe('nobind', function () {
  const tests = framework.list();
  for (const t of tests) {
    describe(t, () => {
      before('configure', () => framework.configure(t, undefined, process.env.ENABLE_ASAN && ['--debug', '--enable_asan', '--enable_typescript'] || ['--enable-typescript']));
      before('build', () => framework.build());
      before('load', () => framework.load(t, process.env.ENABLE_ASAN && 'Debug'));
      before('generate types', () => framework.gen_typescript());
      it('check types', () => framework.check_typescript(t));
      framework.register(t);
      after('GC', global.gc);
    });
    describe(`${t} w/o TypeScript`, () => {
      before('configure', () => framework.configure(t, undefined, process.env.ENABLE_ASAN && ['--debug', '--enable_asan', '--enable_typescript=false'], `${t}-notypescript`));
      it('build', () => framework.build());
    });
  }
});

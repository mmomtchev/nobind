const framework = require('./framework');
const { mocha_typescript } = require('./opts');

describe('nobind', function () {
  const tests = framework.list();
  const debug = !!process.env.ENABLE_DEBUG || !!process.env.ENABLE_ASAN;
  const build_opts = [];
  if (debug) build_opts.push('--debug');
  if (process.env.ENABLE_ASAN) build_opts.push('--enable_asan');
  for (const t of tests) {
    describe(`${t} ${build_opts.join(' ')}`, () => {
      before('configure', () => framework.configure(t, undefined, build_opts));
      before('build', () => framework.build());
      before('load', () => framework.load(t, debug ? 'Debug' : 'Release'));
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

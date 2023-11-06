const framework = require('./framework');

describe('nobind', function () {
  const tests = framework.list();
  for (const t of tests) {
    describe(t, () => {
      framework.register(t);
      before('configure', () => framework.configure(t));
      before('build', () => framework.build());
      before('load', () => framework.load(t));
      after('GC', global.gc);
    });
  }
});

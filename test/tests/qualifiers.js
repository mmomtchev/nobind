const { assert } = require('chai');

it('sync', () => {
  const o = new dll.Qualified;
  assert.strictEqual(o.get1(), 1);
  assert.strictEqual(o.get2(), 2);
  assert.strictEqual(o.get3(), 3);
});

it('async', (done) => {
  const o = new dll.Qualified;
  o.get1Async()
    .then((r) => assert.strictEqual(r, 1))
    .then(() => o.get2Async())
    .then((r) => assert.strictEqual(r, 2))
    .then(() => o.get3Async())
    .then((r) => assert.strictEqual(r, 3))
    .then(() => done())
    .catch(done);
});

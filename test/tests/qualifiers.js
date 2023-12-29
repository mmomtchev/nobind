const { assert } = require('chai');

it('sync', () => {
  const o = new dll.Qualified;
  assert.strictEqual(o.get1(), 1);
  assert.strictEqual(o.get2(), 2);
  assert.strictEqual(o.get3(), 3);
  assert.strictEqual(dll.Qualified.get4(), 4);
  assert.strictEqual(dll.globalQualified(), 0);
});

it('async', (done) => {
  const o = new dll.Qualified;
  o.get1Async()
    .then((r) => assert.strictEqual(r, 1))
    .then(() => o.get2Async())
    .then((r) => assert.strictEqual(r, 2))
    .then(() => o.get3Async())
    .then((r) => assert.strictEqual(r, 3))
    .then(() => dll.Qualified.get4Async())
    .then((r) => assert.strictEqual(r, 4))
    .then(() => dll.globalQualified())
    .then((r) => assert.strictEqual(r, 0))
    .then(() => done())
    .catch(done);
});

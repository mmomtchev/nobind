const { assert } = require('chai');

it('nominal', () => {
  const o = new dll.Qualified;
  assert.strictEqual(o.get1(), 1);
  assert.strictEqual(o.get2(), 2);
  assert.strictEqual(o.get3(), 3);
});

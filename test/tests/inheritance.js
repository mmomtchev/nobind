const { assert } = require('chai');

it('nominal', () => {
  const b = new dll.Base(12);
  assert.instanceOf(b, dll.Base);
  assert.strictEqual(b.get(), 12);
  assert.strictEqual(b.base_get(), 12);

  const d = new dll.Derived(10);
  assert.instanceOf(d, dll.Derived);
  // currently b instanceOf dll.Base is not true
  assert.strictEqual(d.get(), 11);
  assert.strictEqual(d.derived_get(), 11);
});

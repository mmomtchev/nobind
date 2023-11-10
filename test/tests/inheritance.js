const { assert } = require('chai');

it('nominal', () => {
  // instanceof does not work with this
  dll.Derived.prototype.__proto__ = dll.Base.prototype;

  const b = new dll.Base(12);
  assert.instanceOf(b, dll.Base);
  assert.strictEqual(b.get(), 12);
  assert.strictEqual(b.base_get(), 12);

  const d = new dll.Derived(10);
  assert.instanceOf(d, dll.Derived);
  assert.instanceOf(d, dll.Base);
  assert.strictEqual(d.get(), 11);
  assert.strictEqual(d.virtual_base_get(), 11);
  assert.strictEqual(d.base_get(), 10);
  assert.strictEqual(d.derived_get(), 11);
});

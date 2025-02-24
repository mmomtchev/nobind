const { assert } = require('chai');

it('nominal', () => {
  const b = new dll.Base(12);
  assert.instanceOf(b, dll.Base);
  assert.notInstanceOf(b, dll.Derived);
  assert.strictEqual(b.get(), 12);
  assert.strictEqual(b.base_get(), 12);

  const d = new dll.Derived(10);
  assert.instanceOf(d, dll.Derived);
  assert.instanceOf(d, dll.Base);
  assert.strictEqual(d.get(), 11);
  assert.strictEqual(d.virtual_base_get(), 11);
  assert.strictEqual(d.base_get(), 10);
  assert.strictEqual(d.derived_get(), 11);

  assert.throws(() => {
    // @ts-expect-error
    b.derived_get();
  });

  assert.strictEqual(d.ret1(), 1);
  assert.strictEqual(d.ret2(), 2);
});

it('abstract', () => {
  const d = new dll.DerivedAbstract(17);
  assert.instanceOf(d, dll.DerivedAbstract);
  assert.instanceOf(d, dll.Abstract);
  assert.strictEqual(d.id(), 17);

  assert.throws(() => {
    // @ts-expect-error
    new dll.Abstract;
  });
});

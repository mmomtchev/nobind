const { assert } = require('chai');
const { mocha_object_store } = require('../opts');

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
  }, /b\.derived_get is not a function/);

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
  }, /create an object of abstract/);
});

it('automatic upcasting', () => {
  const derived = new dll.Derived(10);

  // Derived is accepted as Base but works as Derived
  assert.strictEqual(dll.requireBase(derived), 11);
});

it('type distinction by the object store', () => {
  if (!mocha_object_store())
    return;
  const derived = new dll.Derived(12);

  // returnBase returns const Base & which creates a new wrapper
  // which is not of the Derived type
  const ret = dll.returnBase(derived);
  assert.instanceOf(ret, dll.Base);
  assert.notInstanceOf(ret, dll.Derived);

  // However the C++ object inside is Derived
  assert.strictEqual(ret.get(), 13);
});

it('object store object reuse', () => {
  if (!mocha_object_store())
    return;

  const base = new dll.Base(16);

  const ret = dll.returnBase(base);
  assert.instanceOf(ret, dll.Base);
  assert.notInstanceOf(ret, dll.Derived);

  // returnBase should not create a new wrapper
  assert.strictEqual(ret, base);

  assert.strictEqual(ret.get(), 16);
});

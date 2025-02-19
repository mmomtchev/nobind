const { assert } = require('chai');

describe('class method', () => {
  it('nominal', () => {
    const o = new dll.WithNative;
    const r = o.method_native('hello');
    assert.isString(r);
    assert.strictEqual(r, "hello from method_native");
  });

  it('exception', () => {
    const o = new dll.WithNative;
    assert.throws(() => {
      o.method_native(1337);
    }, /Expected a string/);
  });

  it('native extension', () => {
    const o = new dll.WithNative;
    assert.isFunction(o.method_native);
    assert.strictEqual(o.native_extension(), o);
  });
});

describe('global method', () => {
  it('nominal', () => {
    const r = dll.global_native('hello');
    assert.isString(r);
    assert.strictEqual(r, "hello from global_native");
  });

  it('exception', () => {
    assert.throws(() => {
      dll.global_native(1337);
    }, /Expected a string/);
  });
});

describe('function w/o arguments', () => {
  it('nominal', () => {
    assert.isString(dll.get_string());
    assert.strictEqual(dll.get_string(), "hello from get_string");
  });
});

describe('per isolate data', () => {
  it('retrieve stored data', () => {
    // @ts-ignore
    assert.isBoolean(dll.get_exports().debug_build);
    // @ts-ignore
    assert.strictEqual(dll.get_exports(), dll);
  });
});

it('manually set constant', () => {
  assert.isBoolean(dll.debug_build);
});

it('basic finalizers', () => {
  assert.isBoolean(dll.basic_finalizers);
  console.log('::notice title=Basic Finalizers::Synchronous finalizers are', dll.basic_finalizers ? 'enabled' : 'disabled');
});

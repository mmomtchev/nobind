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

it('manually set constant', () => {
  assert.isBoolean(dll.debug_build);
});
const { assert } = require('chai');

describe('constructor', () => {
  it('nominal', () => {
    const o = new dll.Hello('Garga');
    assert.instanceOf(o, dll.Hello);
  });

  it('throws', () => {
    assert.throws(() => {
      new dll.Hello;
    }, /No constructor with 0 arguments found/);

    assert.throws(() => {
      new dll.Hello(2);
    }, /Not a string/);
  });
});

describe('methods', () => {
  it('nominal', () => {
    const o = new dll.Hello('Garga');
    assert.isNumber(o.id());
    assert.isString(o.greet('Mr'));
    assert.strictEqual(o.greet('Mr'), 'hello Mr Garga');
  });

  it('throws', () => {
    const o = new dll.Hello('Garga');

    assert.throws(() => {
      o.id(2);
    }, /Expected 0 arguments, got 1/);
    assert.throws(() => {
      o.greet(2);
    }, /Not a string/);
  });
})

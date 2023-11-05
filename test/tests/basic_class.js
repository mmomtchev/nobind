const { assert } = require('chai');

describe('constructor', () => {
  it('nominal', () => {
    const o = new dll.Hello('Garga');
    assert.instanceOf(o, dll.Hello);
  });

  it('exception', () => {
    assert.throws(() => {
      new dll.Hello;
    }, /No constructor with the given 0 arguments found/);

    assert.throws(() => {
      new dll.Hello(2);
    }, /No constructor with the given 1 arguments found/);
  });
});

describe('methods', () => {
  it('nominal', () => {
    const o = new dll.Hello('Garga');
    assert.isNumber(o.get_id());
    assert.isString(o.greet('Mr'));
    assert.strictEqual(o.greet('Mr'), 'hello Mr Garga');
    assert.isUndefined(o.nothing());
  });

  it('exception', () => {
    const o = new dll.Hello('Garga');

    assert.throws(() => {
      o.get_id(2);
    }, /Expected 0 arguments, got 1/);
    assert.throws(() => {
      o.greet(2);
    }, /Not a string/);
  });
});

describe('getters', () => {
  it('nominal', () => {
    const o = new dll.Hello('Garga');
    assert.isNumber(o.id);
  });
});

describe('setters', () => {
  it('nominal', () => {
    const o = new dll.Hello('Garga');
    o.id = 0x1717;
    assert.strictEqual(o.id, 0x1717);
  });

  it('exception', () => {
    const o = new dll.Hello('Garga');

    assert.throws(() => {
      o.id = 'invalid';
    }, /Not a number/);
  });
});

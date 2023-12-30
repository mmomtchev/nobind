const { assert } = require('chai');

describe('constructor', () => {
  it('nominal', () => {
    const o = new dll.Hello('Garga');
    assert.instanceOf(o, dll.Hello);
  });

  it('exception', () => {
    assert.throws(() => {
      new dll.Hello;
    }, /No constructor with 0 arguments found/);

    assert.throws(() => {
      new dll.Hello(2);
    }, /Expected a string/);
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
    }, /Expected a string/);
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
    o.var = 1717;
    assert.strictEqual(o.var, 1717);
  });

  it('readOnly properties', () => {
    const o = new dll.Hello('Garga');
    assert.isNumber(o.id);
    o.id = 7777;
    assert.notEqual(o.id, 7777);
  });

  it('exception', () => {
    const o = new dll.Hello('Garga');

    assert.throws(() => {
      o.var = 'invalid';
    }, /Expected a number/);
  });
});

describe('extension', () => {
  it('nominal', () => {
    const o = new dll.Hello('Garga');
    assert.isString(o.toString());
  });

  it('exception', () => {
    const o = new dll.Hello('Garga');

    assert.throws(() => {
      o.toString(2);
    }, /Expected 0 arguments, got 1/);

    assert.throws(() => {
      dll.Hello.prototype.toString();
    }, /Illegal invocation/);
  });
});

describe('extension w/args', () => {
  it('nominal', () => {
    const o = new dll.Hello('Garga');
    assert.isString(o.toStringWithArg(2));
  });

  it('exception', () => {
    const o = new dll.Hello('Garga');

    assert.throws(() => {
      o.toStringWithArg();
    }, /Expected a number/);

    assert.throws(() => {
      dll.Hello.prototype.toStringWithArg();
    }, /Illegal invocation/);
  });
});

describe('Factory', () => {
  it('nominal', () => {
    const o = new dll.Hello.factory('Garga');
    assert.instanceOf(o, dll.Hello);
    assert.isNumber(o.id);
  });
});

describe('Static object getter', () => {
  it('nominal', () => {
    const o1 = new dll.Hello.staticObject();
    const o2 = new dll.Hello.staticObject();
    assert.instanceOf(o1, dll.Hello);
    assert.instanceOf(o2, dll.Hello);
    assert.strictEqual(o1.id, o2.id);
  });
});

describe('Well-known symbol getter', () => {
  it('nominal', () => {
    assert.isBoolean(dll.Hello[Symbol.isConcatSpreadable]);
  });
});

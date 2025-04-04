const { assert } = require('chai');

describe('constructor', () => {
  it('nominal', () => {
    const o = new dll.Hello('Garga');
    assert.instanceOf(o, dll.Hello);
  });

  it('exception', () => {
    assert.throws(() => {
      // @ts-expect-error
      new dll.Hello;
    }, /No constructor with 0 arguments found/);

    assert.throws(() => {
      // @ts-expect-error
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
      // @ts-expect-error
      o.get_id(2);
    }, /Expected 0 arguments, got 1/);
    assert.throws(() => {
      // @ts-expect-error
      o.greet(2);
    }, /Expected a string/);
  });

  it('undeclared class', () => {
    const o = new dll.Hello('Garga');
    assert.throws(() => {
      // TypeScript won't notice this error - the argument
      // is of unknown type
      dll.takeUndeclared(o);
    }, 'Expected a <unknown to nobind17 class>');
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

  it('symbol named method', () => {
    const o = new dll.Hello('Garga');
    assert.isFunction(o[Symbol.split]);
    assert.strictEqual(o[Symbol.split]('Mr'), o.greet('Mr'));
  });

  it('readOnly properties', () => {
    const o = new dll.Hello('Garga');
    assert.isNumber(o.id);
    // @ts-expect-error
    o.id = 7777;
    assert.notEqual(o.id, 7777);
  });

  it('exception', () => {
    const o = new dll.Hello('Garga');

    assert.throws(() => {
      // @ts-expect-error
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
      // @ts-expect-error
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
      // @ts-expect-error
      o.toStringWithArg();
    }, /Expected a number/);

    assert.throws(() => {
      // @ts-expect-error
      dll.Hello.prototype.toStringWithArg();
    }, /Illegal invocation/);

    assert.throws(() => {
      // @ts-ignore
      const a = [...o];
    }, /Not iterable/);
  });
});

describe('Factory', () => {
  it('nominal', () => {
    const o = dll.Hello.factory('Garga');
    assert.instanceOf(o, dll.Hello);
    assert.isNumber(o.id);
  });
});

describe('Static object getter', () => {
  it('nominal', () => {
    const o1 = dll.Hello.staticObject();
    const o2 = dll.Hello.staticObject();
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

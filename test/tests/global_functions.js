const { assert } = require('chai');

describe('int, int -> int', () => {
  it('nominal', () => {
    assert.isNumber(dll.add(1, 2));
  });

  it('exception', () => {
    assert.throws(() => {
      dll.add('2', 1);
    }, /Expected a number/);

    assert.throws(() => {
      dll.add(2);
    }, /Expected a number/);
  });
});

describe('int, int -> bool', () => {
  it('nominal', () => {
    assert.isBoolean(dll.gte(1, 2));
  });

  it('exception', () => {
    assert.throws(() => {
      dll.gte('2', 1);
    }, /Expected a number/);

    assert.throws(() => {
      dll.gte(2);
    }, /Expected a number/);
  });
});

describe('bool -> int', () => {
  it('nominal', () => {
    assert.isNumber(dll.test(true));
  });

  it('exception', () => {
    assert.throws(() => {
      dll.test('2');
    }, /Expected a boolean/);

    assert.throws(() => {
      dll.test();
    }, /Expected a boolean/);
  });
});

describe('double, double -> double', () => {
  it('nominal', () => {
    assert.isNumber(dll.pow(2.5, 0.75));
  });

  it('exception', () => {
    assert.throws(() => {
      dll.pow('2', 1);
    }, /Expected a number/);

    assert.throws(() => {
      dll.pow(2);
    }, /Expected a number/);
  });
});

describe('string -> string', () => {
  it('nominal', () => {
    assert.isString(dll.hello('test'));
  });

  it('exception', () => {
    assert.throws(() => {
      dll.hello(1);
    }, /Expected a string/);

    assert.throws(() => {
      dll.hello('test', 1);
    }, /Expected 1 arguments, got 2/);
  });
});

describe('void -> void', () => {
  it('nominal', () => {
    assert.isUndefined(dll.nothing());
  });

  it('exception', () => {
    assert.throws(() => {
      dll.nothing(1);
    }, /Expected 0 arguments, got 1/);
  });
});

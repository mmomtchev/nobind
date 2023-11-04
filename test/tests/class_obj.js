const { assert } = require('chai');

describe('reference', () => {
  it('nominal', () => {
    const o = new dll.Hello('pointer');
    assert.isNumber(dll.hello_ref(o));
  });

  it('exception', () => {
    assert.throws(() => {
      dll.hello_ref(2);
    }, /Not an object/);

    assert.throws(() => {
      dll.hello_ref(new dll.TwoCons(1, 2));
    }, /Not a Hello/);

    assert.throws(() => {
      dll.hello_ref({ a: 0 });
    }, /Not a Hello/);

    assert.throws(() => {
      dll.hello_ref();
    }, /Expected 1 arguments, got 0/);
  });
});

describe('pointer', () => {
  it('nominal', () => {
    const o = new dll.Hello('pointer');
    assert.isNumber(dll.hello_ptr(o));

    const b = new dll.TwoCons(1, 2);
  });

  it('exception', () => {
    assert.throws(() => {
      dll.hello_ptr(2);
    }, /Not an object/);

    assert.throws(() => {
      dll.hello_ptr(new dll.TwoCons(1, 2));
    }, /Not a Hello/);

    assert.throws(() => {
      dll.hello_ptr({a: 0});
    }, /Not a Hello/);

    assert.throws(() => {
      dll.hello_ptr();
    }, /Expected 1 arguments, got 0/);
  });
});

describe('const reference', () => {
  it('nominal', () => {
    const o = new dll.Hello('pointer');
    assert.isNumber(dll.hello_const_ref(o));
  });

  it('exception', () => {
    assert.throws(() => {
      dll.hello_const_ref(2);
    }, /Not an object/);

    assert.throws(() => {
      dll.hello_const_ref();
    }, /Expected 1 arguments, got 0/);
  });
});

describe('const pointer', () => {
  it('nominal', () => {
    const o = new dll.Hello('pointer');
    assert.isNumber(dll.hello_const_ptr(o));
  });

  it('exception', () => {
    assert.throws(() => {
      dll.hello_const_ptr(2);
    }, /Not an object/);

    assert.throws(() => {
      dll.hello_const_ptr();
    }, /Expected 1 arguments, got 0/);
  });
});

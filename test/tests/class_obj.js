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
      dll.hello_ref(new dll.TwoCons());
    }, /Not a Hello/);

    assert.throws(() => {
      dll.hello_ref({ a: 0 });
    }, /Not a Hello/);

    assert.throws(() => {
      dll.hello_ref();
    }, /Expected 1 arguments, got 0/);
  });

  describe('null', () => {
    it('null argument when not allowed', () => {
      assert.throws(() => {
        new dll.Hello(null);
      }, /No constructor with the given 1 arguments found/);
      assert.throws(() => {
        assert.isNumber(dll.hello_ref(null));
      }, /Not an object/);
    });

    it('null allowed as return value', () => {
      const o = dll.Hello.factory_tolerant('');
      assert.isNull(o);
    });

    it('null throws when returned', () => {
      assert.throws(() => {
        dll.Hello.factory_strict('');
      }, /Returned nullptr/);
    });
  });

  it('undefined', () => {
    assert.throws(() => {
      new dll.Hello(null);
    }, /No constructor with the given 1 arguments found/);
    assert.throws(() => {
      assert.isNumber(dll.hello_ref(undefined));
    }, /Not an object/);
  });
});

describe('pointer', () => {
  it('nominal', () => {
    const o = new dll.Hello('pointer');
    assert.isNumber(dll.hello_ptr(o));

    const b = new dll.TwoCons();
  });

  it('exception', () => {
    assert.throws(() => {
      dll.hello_ptr(2);
    }, /Not an object/);

    assert.throws(() => {
      dll.hello_ptr(new dll.TwoCons());
    }, /Not a Hello/);

    assert.throws(() => {
      dll.hello_ptr({ a: 0 });
    }, /Not a Hello/);

    assert.throws(() => {
      dll.hello_ptr();
    }, /Expected 1 arguments, got 0/);
  });

  it('null', () => {
    assert.throws(() => {
      assert.isNumber(dll.hello_ptr(null));
    }, /Not an object/);
  });

  it('undefined', () => {
    assert.throws(() => {
      assert.isNumber(dll.hello_ptr(undefined));
    }, /Not an object/);
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

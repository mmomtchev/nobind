const chai = require('chai');
const chaiAsPromised = require('chai-as-promised');
chai.use(chaiAsPromised);
const { assert } = chai;

describe('reference', () => {
  it('nominal', () => {
    const o = new dll.Hello('pointer');
    assert.isNumber(dll.hello_ref(o));
  });

  it('exception', () => {
    assert.throws(() => {
      dll.hello_ref(2);
    }, /Expected an object/);

    assert.throws(() => {
      dll.hello_ref(new dll.TwoCons());
    }, /Expected a Hello/);

    assert.throws(() => {
      dll.hello_ref({ a: 0 });
    }, /Expected a Hello/);

    assert.throws(() => {
      dll.hello_ref();
    }, /Expected an object/);
  });

  describe('null', () => {
    it('null argument when Expected allowed', () => {
      assert.throws(() => {
        new dll.Hello(null);
      }, /Expected a string/);
      assert.throws(() => {
        assert.isNumber(dll.hello_ref(null));
      }, /Expected an object/);
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

    it('null rejects when returned in async mode', () => {
      return assert.isRejected(dll.Hello.factoryAsync_strict(''), /Returned nullptr/);
    });
  });

  it('undefined', () => {
    assert.throws(() => {
      new dll.Hello(null);
    }, /Expected a string/);
    assert.throws(() => {
      assert.isNumber(dll.hello_ref(undefined));
    }, /Expected an object/);
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
    }, /Expected an object/);

    assert.throws(() => {
      dll.hello_ptr(new dll.TwoCons());
    }, /Expected a Hello/);

    assert.throws(() => {
      dll.hello_ptr({ a: 0 });
    }, /Expected a Hello/);

    assert.throws(() => {
      dll.hello_ptr();
    }, /Expected an object/);
  });

  it('null', () => {
    assert.throws(() => {
      assert.isNumber(dll.hello_ptr(null));
    }, /Expected an object/);
  });

  it('undefined', () => {
    assert.throws(() => {
      assert.isNumber(dll.hello_ptr(undefined));
    }, /Expected an object/);
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
    }, /Expected an object/);

    assert.throws(() => {
      dll.hello_const_ref();
    }, /Expected an object/);
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
    }, /Expected an object/);

    assert.throws(() => {
      dll.hello_const_ptr();
    }, /Expected an object/);
  });
});

describe('POD type', () => {
  it('support', () => {
    const i = new dll.IntObject(5);
    assert.strictEqual(dll.hello_pod(i), 5);
  });
});

const chai = require('chai');
const chaiAsPromised = require('chai-as-promised');
chai.use(chaiAsPromised);
const { assert } = chai;

describe('shared_ptr', () => {
  describe('take_shared_ptr', () => {
    it('plain', () => {
      const o = new dll.Hello('Mulder');
      assert.isNumber(dll.takeSharedPtr(o));
    });

    it('const', () => {
      const o = new dll.Hello('Mulder');
      assert.isNumber(dll.takeConstSharedPtr(o));
    });

    it('exception', () => {
      assert.throws(() => {
        // @ts-expect-error
        dll.takeSharedPtr({});
      }, /Expected a Hello/);
    });
  });

  describe('take_shared_ptr Async', () => {
    it('plain', (done) => {
      const o = new dll.Hello('Mulder');
      dll.takeSharedPtrAsync(o).then((r) => {
        assert.isNumber(r);
        done();
      }).catch(done);
    });

    it('const', (done) => {
      const o = new dll.Hello('Mulder');
      dll.takeConstSharedPtrAsync(o).then((r) => {
        assert.isNumber(r);
        done();
      }).catch(done);
    });

    it('exception', () =>
      // @ts-expect-error
      assert.isRejected(dll.takeSharedPtrAsync({}), /Expected a Hello/)
    );
  });

  describe('return_shared_ptr', () => {
    it('nominal', () => {
      const o = dll.returnSharedPtr('Scully');
      assert.instanceOf(o, dll.Hello);
      assert.strictEqual(o.greet('Agent'), 'hello Agent Scully');
    });

    it('exception', () => {
      assert.throws(() => {
        // @ts-expect-error
        dll.returnSharedPtr(2);
      }, /Expected a string/);
    });
  });

  describe('return_shared_ptr Async', () => {
    it('nominal', (done) => {
      dll.returnSharedPtrAsync('Scully').then((o) => {
        assert.instanceOf(o, dll.Hello);
        assert.strictEqual(o.greet('Agent'), 'hello Agent Scully');
        done();
      }).catch(done);
    });

    it('exception', () =>
      // @ts-expect-error
      assert.isRejected(dll.returnSharedPtrAsync(2), /Expected a string/)
    );
  });

  it.only('object store interaction', () => {
    const o1 = dll.returnSharedPtr('Doggett');
    const o2 = dll.takeAndReturnSharedPtr(o1);
    const o3 = dll.takeAndReturnSharedPtr(o2);
    const o4 = dll.takeAndReturnSharedPtr(o1);
    assert.strictEqual(o1, o2);
    assert.strictEqual(o1, o3);
    assert.strictEqual(o1, o4);
  });
});

describe('unique_ptr', () => {
  it('sync', () => {
    const u = dll.returnUniquePtr('Mulder and Scully');
    assert.instanceOf(u, dll.HelloUPtr);
    assert.strictEqual(u.greet('Agents'), 'hello Agents Mulder and Scully');
  });

  it('exception', () => {
    assert.throws(() => {
      // @ts-expect-error
      dll.returnUniquePtr(12);
    }, /Expected a string/);
  });

  it('async', (done) => {
    dll.returnUniquePtrAsync('Mulder and Scully').then((u) => {
      assert.instanceOf(u, dll.HelloUPtr);
      assert.strictEqual(u.greet('Agents'), 'hello Agents Mulder and Scully');
      done();
    }).catch(done);
  });

  it('async', (done) => {
    dll.returnUniquePtrAsync('Mulder and Scully').then((u) => {
      assert.instanceOf(u, dll.HelloUPtr);
      assert.strictEqual(u.greet('Agents'), 'hello Agents Mulder and Scully');
      done();
    }).catch(done);
  });

  it('async exception', () =>
    // @ts-expect-error
    assert.isRejected(dll.returnUniquePtrAsync(12), 'Expected a string')
  );
});

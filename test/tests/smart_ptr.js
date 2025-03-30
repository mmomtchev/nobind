const { assert } = require('chai');

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

  describe('return_shared_ptr', () => {
    it('nominal', () => {
      const o = dll.returnSharedPtr('Scully');
      assert.instanceOf(o, dll.Hello);
      assert.strictEqual(o.greet('Agent'), 'hello Agent Scully');
    });
  });
});

describe('unique_ptr', () => {
  it('create and use unique_ptr from JS', () => {
    const u = dll.returnUniquePtr('Mulder and Scully');
    assert.instanceOf(u, dll.HelloUPtr);
    assert.strictEqual(u.greet('Agents'), 'hello Agents Mulder and Scully');
  });
});

const { assert } = require('chai');

describe('shared_ptr', () => {
  describe('take_shared_ptr', () => {
    it('nominal', () => {
      const o = new dll.Hello('Mulder');
      assert.isNumber(dll.takeSharedPtr(o));
    });

    it('exception', () => {
      assert.throws(() => {
        dll.takeSharedPtr({});
      }, /Expected a Hello/);
    });
  });

  describe('return_shared_ptr', () => {
    it('nominal', () => {
      const o = dll.returnSharedPtr('Sully');
      assert.instanceOf(o, dll.Hello);
      assert.strictEqual(o.greet('Agent'), 'hello Agent Sully');
    });
  });
});

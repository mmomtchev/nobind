const { assert } = require('chai');

describe('shared_ptr', () => {
  describe('take_shared_ptr', () => {
    it('nominal', () => {
      const o = new dll.Hello('Mulder');
      assert.isNumber(dll.takeSharedPtr(o));
    });
  });
});

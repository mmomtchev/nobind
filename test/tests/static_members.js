const { assert } = require('chai');

describe('StaticMembers', () => {
  it('static member function', () => {
    const o = new dll.StaticMembers;
    assert.instanceOf(o, dll.StaticMembers);

    assert.strictEqual(dll.StaticMembers.get_static(), 2);
    assert.isUndefined(o.get_static);

    assert.isUndefined(dll.StaticMembers.nothing_static());
    assert.isUndefined(o.nothing_static);
  });

  it('static member getter', () => {
    const o = new dll.StaticMembers;
    assert.instanceOf(o, dll.StaticMembers);

    assert.strictEqual(dll.StaticMembers.static_member, 2);
    assert.isUndefined(o.static_member);
  });

  describe('static member setter', () => {
    it('nominal', () => {
      const o = new dll.StaticMembers;
      assert.instanceOf(o, dll.StaticMembers);

      dll.StaticMembers.static_member = 1717;
      assert.strictEqual(dll.StaticMembers.static_member, 1717);
      dll.StaticMembers.static_member = 2;
    });

    it('readOnly', () => {
      const o = new dll.StaticMembers;
      assert.instanceOf(o, dll.StaticMembers);

      dll.StaticMembers.static_readonly = 2727;
      assert.notEqual(dll.StaticMembers.static_member, 2727);
    });

    it('exception', () => {
      assert.throws(() => {
        dll.StaticMembers.static_member = 'invalid';
      }, /Not a number/);
    });
  });
});

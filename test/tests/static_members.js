const { assert } = require('chai');

describe('StaticMembers', () => {
  it('static member function', () => {
    const o = new dll.StaticMembers;
    assert.instanceOf(o, dll.StaticMembers);

    assert.strictEqual(dll.StaticMembers.get_static(), 2);
    // @ts-expect-error
    assert.isUndefined(o.get_static);

    assert.isUndefined(dll.StaticMembers.nothing_static());

    // @ts-expect-error
    assert.isUndefined(o.nothing_static);
  });

  it('static member getter', () => {
    const o = new dll.StaticMembers;
    assert.instanceOf(o, dll.StaticMembers);

    assert.strictEqual(dll.StaticMembers.static_member, 2);

    // @ts-expect-error
    assert.isUndefined(o.static_member);
  });

  it('global getter', () => {
    assert.isString(dll.version);
    assert.strictEqual(dll.version, '0.0.7');
  });

  it('global getter of class type', () => {
    assert.instanceOf(dll.global_object, dll.StaticMembers);
    assert.strictEqual(dll.global_object.get_instance(), 12);
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

      // @ts-expect-error
      dll.StaticMembers.static_readonly = 2727;
      assert.notEqual(dll.StaticMembers.static_member, 2727);
    });

    it('exception', () => {
      assert.throws(() => {
        // @ts-expect-error
        dll.StaticMembers.static_member = 'invalid';
      }, /Expected a number/);
    });
  });
});

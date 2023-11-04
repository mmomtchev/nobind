const { assert } = require('chai');

describe('StaticMembers', () => {
  it('static member function', () => {
    const o = new dll.StaticMembers;
    assert.instanceOf(o, dll.StaticMembers);
    assert.strictEqual(dll.StaticMembers.get_static(), 2);
    assert.isUndefined(o.get_static);
  });

  it('static member getter', () => {
    const o = new dll.StaticMembers;
    assert.instanceOf(o, dll.StaticMembers);
    assert.strictEqual(dll.StaticMembers.static_member, 2);
    assert.isUndefined(o.static_member);
  });
});

const { assert } = require('chai');

describe('iterators', () => {
  it('scalar values', () => {
    const o = new dll.Range_10_20;
    assert.isFunction(o[Symbol.iterator]);
    const r = [];
    for (const i of o) {
      r.push(i);
    }
    assert.sameOrderedMembers(r, [10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20]);
  });

  it('objects by reference', () => {
    const o = new dll.HelloList;
    assert.isFunction(o[Symbol.iterator]);
    const r = [];
    o.push_back(new dll.Hello('Gargantua'));
    o.push_back(new dll.Hello('Pantagruel'));
    for (const i of o) {
      r.push(i.greet('Mr'));
    }
    assert.sameOrderedMembers(r, ['hello Mr Gargantua', 'hello Mr Pantagruel']);
  });
});

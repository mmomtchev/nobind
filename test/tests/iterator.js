const { assert } = require('chai');
const { mocha_object_store } = require('../opts');

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
    const o = new dll.HelloPtrList;
    assert.isFunction(o[Symbol.iterator]);

    const el1 = new dll.Hello('Gargantua');
    const el2 = new dll.Hello('Pantagruel');
    o.push_back(el1);
    o.push_back(el2);

    const greets = [];
    const objects = [];
    for (const i of o) {
      objects.push(i);
      greets.push(i.greet('Mr'));
    }
    assert.sameOrderedMembers(greets, ['hello Mr Gargantua', 'hello Mr Pantagruel']);

    // This tests the object store
    if (!mocha_object_store())
      return;
    assert.sameOrderedMembers(objects, [el1, el2]);
  });

  it('pointers', () => {
    const o = new dll.HelloPtrList;
    assert.isFunction(o[Symbol.iterator]);

    const el1 = new dll.Hello('Gargantua');
    const el2 = new dll.Hello('Pantagruel');
    o.push_back(el1);
    o.push_back(el2);

    const greets = [];
    const objects = [];
    for (const i of o) {
      objects.push(i);
      greets.push(i.greet('Mr'));
    }
    assert.sameOrderedMembers(greets, ['hello Mr Gargantua', 'hello Mr Pantagruel']);

    // This tests the object store
    if (!mocha_object_store())
      return;
    assert.sameOrderedMembers(objects, [el1, el2]);
  });
});

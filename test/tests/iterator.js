const { assert } = require('chai');

it('nominal', () => {
  const o = new dll.Range_10_20;
  const r = [];
  // @ts-ignore
  for (const i of o) {
    r.push(i);
  }
  assert.sameOrderedMembers(r, [10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20]);
});

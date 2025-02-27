const { assert } = require('chai');

it('nominal', () => {
  const alice = new dll.Chained;
  assert.instanceOf(alice, dll.Chained);
  assert.strictEqual(alice.inc1().inc10().inc100(), alice);
  assert.strictEqual(alice.get(), 111);
});

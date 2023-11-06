const { assert } = require('chai');

it('nominal', () => {
  const alice = new dll.Chained;
  assert.instanceOf(alice, dll.Chained);
  alice.inc1().inc10().inc100();
  assert.strictEqual(alice.get(), 111);
});

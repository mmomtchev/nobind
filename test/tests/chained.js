const { assert } = require('chai');
const { mocha_object_store } = require('../opts');

it('nominal', () => {
  const alice = new dll.Chained;
  assert.instanceOf(alice, dll.Chained);
  assert.strictEqual(alice.inc1().inc10().inc100().get(), 111);
  assert.strictEqual(alice.get(), 111);

  // Object store
  if (!mocha_object_store())
    return;
  assert.strictEqual(alice.inc1().inc10().inc100(), alice);
});

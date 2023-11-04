const { assert } = require('chai');

describe('constructor', () => {
  it('nominal', () => {
    const o = new dll.int(17);
    assert.instanceOf(o, dll.int);
    assert.strictEqual(o.get(), 17);
  });
});

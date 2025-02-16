const { assert } = require('chai');

describe('constructor', () => {
  it('throws', () => {
    assert.throws(() => {
      // @ts-expect-error
      new dll.Undestructible;
    });
  });
});

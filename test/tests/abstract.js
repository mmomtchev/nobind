const { assert } = require('chai');

describe('undestructible', () => {
  it('throws', () => {
    assert.throws(() => {
      // @ts-expect-error
      new dll.Undestructible;
    });
  });
});

describe('class w/o JS constructor', () => {
  it('throws', () => {
    assert.throws(() => {
      // @ts-expect-error
      new dll.Unconstructible;
    });
  });
});

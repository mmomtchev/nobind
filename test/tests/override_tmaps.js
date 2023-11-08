const { assert } = require('chai');

describe('add with strings', () => {
  it('nominal', () => {
    const r = dll.add('2', '3');
    assert.isString(r);
  });

  it('exception', () => {
    assert.throws(() => {
      dll.add(2, 3);
    }, /Not a string/);
  });
});

const { assert } = require('chai');

describe('overloaded constructor', () => {
  it('nominal w/string', () => {
    const o = new dll.TwoCons('1');
    assert.instanceOf(o, dll.TwoCons);
    assert.isNumber(o.x);
    assert.strictEqual(o.x, 1);
  });

  it('nominal w/number', () => {
    const o = new dll.TwoCons(2);
    assert.instanceOf(o, dll.TwoCons);
    assert.isNumber(o.x);
    assert.strictEqual(o.x, 2);
  });

  it('exception', () => {
    assert.throws(() => {
      new dll.TwoCons(1, 2, 3);
    }, /No constructor with the given 3 arguments found/);

    assert.throws(() => {
      new dll.TwoCons(null);
    }, /No constructor with the given 1 arguments found/);
  });
});

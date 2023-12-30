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

  it('original C++ exception', () => {
    assert.throws(() => {
      new dll.TwoCons(false);
    }, /wrong constructor/);
  });

  it('exception', () => {
    assert.throws(() => {
      new dll.TwoCons(1, 2, 3);
    }, /No constructor with 3 arguments found/);

    assert.throws(() => {
      new dll.TwoCons(null);
    }, /All constructors with 1 arguments tried: \[Expected a string, Expected a number, Expected a boolean\]/);
  });
});

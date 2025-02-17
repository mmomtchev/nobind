const { assert } = require('chai');

describe('add with strings', () => {
  it('nominal', () => {
    const r = dll.add('2', '3');
    assert.isString(r);
  });

  it('exception', () => {
    assert.throws(() => {
      // @ts-expect-error
      dll.add(2, 3);
    }, /Expected a string/);
  });
});

describe('hidden string arguments', () => {
  it('nominal', () => {
    const r = dll.hello();
    assert.isString(r);
    assert.strictEqual(r, 'hello Static string');
  });

  it('exception', () => {
    assert.throws(() => {
      // @ts-expect-error
      dll.hello('invalid');
    }, /Expected 0 arguments, got 1/);
  });
});

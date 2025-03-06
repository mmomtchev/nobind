const { assert } = require('chai');
const { mocha_object_store } = require('../opts');

describe('forward declaration', () => {
  it('construct', () => {
    const h = new dll.Hello('Pantagruel');
    assert.instanceOf(h, dll.Hello);
    const d = new dll.Dependant(h);
    assert.instanceOf(d, dll.Dependant);
    const h2 = d.get();
    assert.instanceOf(h2, dll.Hello);
    assert.strictEqual(h2.greet('Mr'), 'hello Mr Pantagruel');

    // test the object store
    if (!mocha_object_store())
      return;
    assert.strictEqual(h2, d.get());
  });
});

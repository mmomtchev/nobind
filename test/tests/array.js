const { assert } = require('chai');
const { mocha_locking } = require('../opts');

describe('std::vector', () => {
  it('get array of object pointers', () => {
    const array = dll.get_ptr_array(10);
    assert.isArray(array);
    assert.lengthOf(array, 10);
    assert.instanceOf(array[0], dll.Hello);
  });

  it('transform array of object pointers', () => {
    const array = dll.put_ptr_array('comrade', [new dll.Hello('Chapai'), new dll.Hello('Petka')]);
    assert.deepStrictEqual(array, ['hello comrade Chapai', 'hello comrade Petka']);
  });

  it('get array of objects', () => {
    const array = dll.get_obj_array(10);
    assert.isArray(array);
    assert.lengthOf(array, 10);
    assert.instanceOf(array[0], dll.Hello);
  });

  it('transform array of objects', () => {
    const array = dll.put_obj_array('comrade', [new dll.Hello('Chapai'), new dll.Hello('Petka')]);
    assert.deepStrictEqual(array, ['hello comrade Chapai', 'hello comrade Petka']);
  });

  it('exception', () => {
    assert.throws(() => {
      // @ts-expect-error
      dll.put_ptr_array('comrade', 'invalid');
    }, /Expected an array/);

    assert.throws(() => {
      // @ts-expect-error
      dll.put_ptr_array('comrade', [{ field: 'value' }]);
    }, /Expected a Hello/);
  });

  it('recursive locking', function (done) {
    if (!mocha_locking())
      return;
    this.slow(5000);
    this.timeout(10000);

    const array = [];
    for (let i = 0; i < 10; i++)
      array.push(new dll.Critical);

    const q = [];
    let counter = 0;
    const inc = 100;
    for (let i = 0; i < 1e4; i++) {
      q.push(dll.incrementCritical(array, inc));
      counter += inc;
    }

    Promise.all(q)
      .then(() => {
        for (const el of array) {
          assert.strictEqual(el.get(), counter);
        }
        done();
      })
      .catch(done);
  });
});

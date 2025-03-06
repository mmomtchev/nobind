const { assert } = require('chai');
const { mocha_locking } = require('../opts');

describe('std::map', () => {
  it('get map of object pointers', () => {
    const map = dll.get_ptr_map(10);
    assert.isObject(map);
    assert.lengthOf(Object.keys(map), 10);
    assert.instanceOf(map['element 7'], dll.Hello);
  });

  it('transform map of object pointers', () => {
    const map = dll.put_ptr_map('comrade', { Chapai: new dll.Hello('Chapai'), Petka: new dll.Hello('Petka') });
    assert.deepStrictEqual(Object.keys(map), ['Chapai', 'Petka']);
    assert.deepStrictEqual(Object.values(map), ['hello comrade Chapai', 'hello comrade Petka']);
  });

  it('get map of objects', () => {
    const map = dll.get_obj_map(10);
    assert.isObject(map);
    assert.lengthOf(Object.keys(map), 10);
    assert.instanceOf(map['element 7'], dll.Hello);
  });

  it('transform map of objects', () => {
    const map = dll.put_obj_map('comrade', { Chapai: new dll.Hello('Chapai'), Petka: new dll.Hello('Petka') });
    assert.deepStrictEqual(Object.keys(map), ['Chapai', 'Petka']);
    assert.deepStrictEqual(Object.values(map), ['hello comrade Chapai', 'hello comrade Petka']);
  });

  it('exception', () => {
    assert.throws(() => {
      // @ts-expect-error
      dll.put_ptr_map('comrade', 'invalid');
    }, /Expected an object/);

    assert.throws(() => {
      // @ts-expect-error
      dll.put_ptr_map('comrade', { key: { field: 'value' } });
    }, /Expected a Hello/);
  });

  it('recursive locking', function (done) {
    if (!mocha_locking)
      return;
    this.slow(5000);
    this.timeout(10000);

    const map = {};
    for (let i = 0; i < 10; i++)
      map[i.toString()] = new dll.Critical;

    const q = [];
    let counter = 0;
    const inc = 100;
    for (let i = 0; i < 1e4; i++) {
      q.push(dll.incrementCritical(map, inc));
      counter += inc;
    }

    Promise.all(q)
      .then(() => {
        for (const el of Object.keys(map)) {
          assert.strictEqual(map[el].get(), counter);
        }
        done();
      })
      .catch(done);
  });
});

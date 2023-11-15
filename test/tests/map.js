const { assert } = require('chai');

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
      dll.put_ptr_map('comrade', 'invalid');
    }, /Expected an object/);

    assert.throws(() => {
      dll.put_ptr_map('comrade', { key: { field: 'value' } });
    }, /Expected a Hello/);
  });
});

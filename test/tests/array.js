const { assert } = require('chai');

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
      dll.put_ptr_array('comrade', 'invalid');
    }, /Expected an array/);

    assert.throws(() => {
      dll.put_ptr_array('comrade', [{ field: 'value' }]);
    }, /Expected a Hello/);
  });
});

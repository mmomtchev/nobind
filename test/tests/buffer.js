const { assert } = require('chai');

describe('get', () => {
  it('nominal', () => {
    const buf = dll.get_buffer();
    assert.instanceOf(buf, Buffer);
    assert.lengthOf(buf, 16);
    const typed = new Uint32Array(buf.buffer);
    assert.deepStrictEqual(typed, new Uint32Array([0x17, 0x17, 0x17, 0x17]));
  });
});

describe('put', () => {
  it('nominal', () => {
    const buf = Buffer.from(new Uint32Array([0x17, 0x17, 0x17, 0x17]).buffer);
    dll.put_buffer(buf);
  });
  it('exception', () => {
    assert.throws(() => {
      const buf = Buffer.from(new Uint32Array([0x17, 0x17, 0x11, 0x17]).buffer);
      dll.put_buffer(buf);
    }, /Invalid value/);
  });
});

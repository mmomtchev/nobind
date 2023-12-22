const { assert } = require('chai');

describe('nested objects', () => {
  it('constructors', () => {
    const time = new dll.Time(100);
    assert.instanceOf(time, dll.Time);

    const dt1 = new dll.DateTime(100);
    assert.instanceOf(dt1, dll.DateTime);

    const dt2 = new dll.DateTime(time);
    assert.instanceOf(dt2, dll.DateTime);
  });

  it('operators', () => {
    const time = new dll.Time(100);
    assert.isNumber(time.get());
    assert.strictEqual(time.get(), 100);

    const dt1 = new dll.DateTime(101);
    assert.instanceOf(dt1, dll.DateTime);
    assert.strictEqual(dt1.get().get(), 101);

    const dt2 = new dll.DateTime(time);
    assert.instanceOf(dt2, dll.DateTime);
    assert.strictEqual(dt2.get().get(), 100);
  });

  it('explicit nested references', () => {
    const dt = new dll.DateTime(new dll.Time(111));
    assert.instanceOf(dt, dll.DateTime);
    assert.strictEqual(dt.get().__nobind_parent_reference, dt);
    assert.strictEqual(dt.get().get(), 111);
  });

  it('implicit nested references', () => {
    const dt = new dll.DateTime(new dll.Time(117));
    assert.instanceOf(dt, dll.DateTime);
    assert.strictEqual(dt.time.__nobind_parent_reference, dt);
    assert.strictEqual(dt.get().get(), 117);
  });
});

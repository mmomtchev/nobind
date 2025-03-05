const { assert } = require('chai');
const { mocha_object_store } = require('../opts');

describe('stress tests', function () {
  this.timeout(20000);
  this.slow(20000);

  // Objects created from JS are destroyed
  // when their JS wrappers are GCed
  it('JavaScript created objects', async () => {
    const { Hello, DateTime, Time } = dll;

    // Two 1000 element arrays
    const hello = (new Array(1000)).fill(new Hello('Gen. Ripper'), 0, 1000);
    const dateTime = (new Array(1000)).fill(new DateTime(17), 0, 1000);

    const time = [];
    for (let i = 0; i < 5e4; i++) {
      const pick = Math.floor(Math.random() * 1000);

      // Keep nested references to some elements
      // they should survive until the end
      if (Math.random() < 0.1) {
        time.push(await dateTime[pick].get());
        if (time.length > 1000)
          time.shift();
      }

      if (Math.random() < 0.5) {
        // Randomly pick elements, check them and replace them
        assert.instanceOf(hello[pick], Hello);
        assert.isNumber(await hello[pick].get_id());
        hello[pick] = new Hello('Gen. Ripper');

        assert.instanceOf(dateTime[pick], DateTime);
        const v = await (await dateTime[pick].get()).get();
        assert.isNumber(v);
        assert.isTrue(v === 17 || v === pick);
        dateTime[pick] = new DateTime(pick);
      } else {
        // Randomly pick elements and replace them with ReturnSame
        hello[pick] = hello[pick].same();
        dateTime[pick] = dateTime[pick].same();
      }
    }

    // Check that these still work
    for (let i in time) {
      assert.instanceOf(time[i], Time);
      assert.isNumber(await time[i].get());
    }
  });

  // Objects created from C++ are preserved
  // when their JS wrappers are GCed
  it('C++ created objects', async () => {
    for (let i = 0; i < 5e4; i++) {
      const pick = Math.floor(Math.random() * 1000);
      const dt = dll.cpp_check_and_replace(pick, Math.random() < 0.5);
      assert.instanceOf(dt, dll.DateTime);
      const v = await (await dt.get()).get();
      assert.strictEqual(v, pick);
    }
  });

  it('object vector construction and deconstruction', async () => {
    let v = [];
    for (let i = 0; i < 100; i++)
      v.push(new dll.Hello(i.toString()));

    for (let i = 0; i < 10000; i++) {
      v = await dll.take_and_return_object_vector(v);
    }

    assert.lengthOf(v, 100);
    for (const i in v) {
      assert.instanceOf(v[i], dll.Hello);
      assert.strictEqual(v[i].greet('comrade'), `hello comrade ${i}`);
    }
  });

  it('pointer vector construction and deconstruction', async () => {
    const orig = [];
    for (let i = 0; i < 100; i++) {
      orig.push(new dll.Hello(i.toString()));
    }
    let v = [...orig];

    for (let i = 0; i < 10000; i++) {
      v = await dll.take_and_return_ptr_vector(v);
    }

    assert.lengthOf(v, 100);
    for (const i in v) {
      assert.instanceOf(v[i], dll.Hello);
      assert.strictEqual(v[i].greet('comrade'), `hello comrade ${i}`);
      // object store preserves the original objects
      if (!mocha_object_store())
        return;
      assert.strictEqual(v[i], orig[i]);
    }
  });
});

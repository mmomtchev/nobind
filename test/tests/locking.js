const chai = require('chai');
const chaiAsPromised = require('chai-as-promised');
const { mocha_locking } = require('../opts');
chai.use(chaiAsPromised);
const { assert } = chai;

describe('locking', function () {
  if (!mocha_locking())
    return;
  this.timeout(10000);
  this.slow(5000);

  it('member methods', (done) => {
    const [c1, c2] = [new dll.Critical, new dll.Critical];
    let count = 0;
    const inc = 100;
    const q = [];
    for (let i = 0; i < 2e4; i++) {
      q.push(c1.increment(inc));
      q.push(c2.increment(inc));
      count += inc;
    }
    Promise.all(q).then(() => {
      assert.strictEqual(c1.get(), count);
      assert.strictEqual(c2.get(), count);
      done();
    }).catch(done);
  });

  it('arguments', (done) => {
    const [c1, c2] = [new dll.Critical, new dll.Critical];
    let count = 0;
    const inc = 100;
    const q = [];
    for (let i = 0; i < 2e4; i++) {
      q.push(dll.increment(c1, inc));
      q.push(dll.increment(c2, inc));
      count += inc;
    }
    Promise.all(q).then(() => {
      assert.strictEqual(c1.get(), count);
      assert.strictEqual(c2.get(), count);
      done();
    }).catch(done);
  });

  it('getters and setters', (done) => {
    const c = new dll.Critical;
    const inc = 100;
    const q = [];
    for (let i = 0; i < 2e4; i++) {
      q.push(c.increment(inc));
      if (Math.random() < 0.25) {
        assert.isNumber(c.value);
        assert.isTrue(c.value % inc === 0);
        c.value = c.value;
      }
    }
    Promise.all(q).then(() => {
      assert.isNumber(c.value);
      assert.isTrue(c.value % inc === 0);
      done();
    }).catch(done);
  });

  it('sync class extension', (done) => {
    const c = new dll.Critical;
    const inc = 100;
    let counter = 0;
    const q = [];
    for (let i = 0; i < 2e4; i++) {
      q.push(c.increment(inc));
      counter += inc;
      if (Math.random() < 0.25) {
        assert.isNumber(c.value);
        assert.isTrue(c.value % inc === 0);
        c.ext(inc);
        counter += inc;
      }
    }
    Promise.all(q).then(() => {
      assert.isNumber(c.value);
      assert.strictEqual(c.value, counter);
      done();
    }).catch(done);
  });
});

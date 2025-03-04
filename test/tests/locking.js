const chai = require('chai');
const chaiAsPromised = require('chai-as-promised');
chai.use(chaiAsPromised);
const { assert } = chai;

describe('locking', () => {
  it('this object', (done) => {
    const [c1, c2] = [new dll.Critical, new dll.Critical];
    let count = 0;
    const inc = 100;
    const q = [];
    for (let i = 0; i < 1e4; i++) {
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

  it.only('arguments', (done) => {
    const [c1, c2] = [new dll.Critical, new dll.Critical];
    let count = 0;
    const inc = 100;
    const q = [];
    for (let i = 0; i < 1e4; i++) {
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
});

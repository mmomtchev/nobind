const chai = require('chai');
const chaiAsPromised = require('chai-as-promised');
chai.use(chaiAsPromised);
const { assert } = chai;

describe('int, int -> int', () => {
  it('nominal', () =>
    assert.isFulfilled(dll.add(1, 2).then((r) => {
      assert.isNumber(r);
    }))
  );

  it('exception', () =>
    // @ts-expect-error
    assert.isRejected(dll.add('2', 1), /Expected a number/)
      // @ts-expect-error
      .then(() => assert.isRejected(dll.add(2), /Expected a number/))
  );
});

describe('int, int -> bool', () => {
  it('nominal', () =>
    assert.isFulfilled(dll.gte(1, 2)).then((r) => assert.isBoolean(r))
  );

  it('exception', () =>
    // @ts-expect-error
    assert.isRejected(dll.gte('2', 1), /Expected a number/)
      // @ts-expect-error
      .then(() => assert.isRejected(dll.gte(2), /Expected a number/))
  );
});

describe('double, double -> double', () => {
  it('nominal', () =>
    assert.isFulfilled(dll.pow(2.5, 0.75)).then((r) => assert.isNumber(r))
  );

  it('exception', () =>
    // @ts-expect-error
    assert.isRejected(dll.pow('2', 1), /Expected a number/)
      // @ts-expect-error
      .then(() => assert.isRejected(dll.pow(2)), /Expected a number/)
  );
});


describe('string -> string', () => {
  it('nominal', () =>
    assert.isFulfilled(dll.hello('test')).then((r) => assert.isString(r))
  );

  it('exception', () =>
    // @ts-expect-error
    assert.isRejected(dll.hello(1), /Expected a string/)
      // @ts-expect-error
      .then(() => assert.isRejected(dll.hello('test', 1)), /Expected 1 arguments, got 2/)
  );
});

describe('void -> void', () => {
  it('nominal', () =>
    assert.isFulfilled(dll.nothing()).then((r) => assert.isUndefined(r))
  );

  it('exception', () =>
    // @ts-expect-error
    assert.isRejected(dll.nothing(1), /Expected 0 arguments, got 1/)
  );
});

describe('class method', () => {
  it('nominal', () => {
    const o = new dll.Hello('Garga');
    o.get_id()
      .then((r) => assert.isNumber(r))
      .then(() => o.greet('Mr'))
      .then((r) => assert.isString(r))
      .then(() => o.greet('Mr'))
      .then((r) => assert.strictEqual(r, 'hello Mr Garga'))
      .then(() => o.nothing())
      .then((r) => assert.isUndefined());
  });


  it('exception', () => {
    const o = new dll.Hello('Garga');

    // @ts-expect-error
    return assert.isRejected(o.get_id(2), /Expected 0 arguments, got 1/)
      // @ts-expect-error
      .then(() => assert.isRejected(o.greet(2), /Expected a string/));
  });

  describe('duplex definitions', () => {
    it('async versions', () =>
      assert.isFulfilled(dll.helloDuplexAsync('test')).then((r) => assert.isString(r))
    );

    it('sync versions', () => {
      assert.isString(dll.helloDuplexSync('test'));
    });
  });
});

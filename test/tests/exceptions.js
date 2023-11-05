const { assert } = require('chai');

it('global', () => {
  assert.throws(() => {
    dll.throws();
  }, /Global error/);

});

it('class method', () => {
  assert.throws(() => {
    new dll.Hello('test').throws();
  }, /Hello error/);
});

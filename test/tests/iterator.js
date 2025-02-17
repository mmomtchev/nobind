const { assert } = require('chai');

it('nominal', () => {
  const o = new dll.Range_10_20(1);
  for (const i of o) {
    console.log(i);
  }
});

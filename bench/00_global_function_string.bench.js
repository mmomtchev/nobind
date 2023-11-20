const b = require('benny');
const path = require('path');
const crypto = require('crypto');
const assert = require('assert');

const napi = require(path.resolve(__dirname, 'build', 'Release', 'napi.node'));
const nobind = require(path.resolve(__dirname, 'build', 'Release', 'nobind.node'));

const len = 16384;
const Data = crypto.randomBytes(len / 2).toString('hex');

module.exports = function () {
  return b.suite(
    `Global function strlen`,

    b.add('nobind', () => {
      assert(nobind.strlen(Data) === len, 'Data error');
    }),
    b.add('napi', () => {
      assert(napi.strlen(Data) === len, 'Data error');
    }),
    b.cycle(),
    b.complete()
  );
};

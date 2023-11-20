const b = require('benny');
const path = require('path');
const crypto = require('crypto');
const assert = require('assert');

const napi = require(path.resolve(__dirname, 'build', 'Release', 'napi.node'));
const nobind = require(path.resolve(__dirname, 'build', 'Release', 'nobind.node'));
const swig = require(path.resolve(__dirname, 'build', 'Release', 'swig.node'));

const len = 16384;
const Data = crypto.randomBytes(len / 2).toString('hex');

module.exports = function () {
  return b.suite(
    `Global function strlenAsync`,

    b.add('nobind', async () => {
      assert(await nobind.strlenAsync(Data) === len, 'Data error');
    }),
    b.add('napi', async () => {
      assert(await napi.strlenAsync(Data) === len, 'Data error');
    }),
    b.add('swig', async () => {
      assert(await swig.strlenAsync(Data) === len, 'Data error');
    }),
    b.cycle(),
    b.complete()
  );
};

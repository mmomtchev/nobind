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
    `Class method strlen`,

    b.add('nobind', () => {
      const object = new nobind.String(Data);
      assert(object.length() === len, 'Data error');
    }),
    b.add('napi', () => {
      const object = new napi.String(Data);
      assert(object.length() === len, 'Data error');
    }),
    b.add('swig', () => {
      const object = new swig.String(Data);
      assert(object.length() === len, 'Data error');
    }),
    b.cycle(),
    b.complete()
  );
};

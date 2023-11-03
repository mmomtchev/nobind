const fs = require('fs');
const path = require('path');
const framework = require('./framework');

function usage() {
  console.log('Usage: node single.js <configure|build|run> <test>');
  process.exit(1);
}

let test = process.argv[3];
if (!test) {
  try {
    const gypi = fs.readFileSync(path.resolve(__dirname, 'build', 'config.gypi'), 'utf-8').replace(/#.*/, '');
    const data = JSON.parse(gypi);
    test = data.variables.test;
    console.log('current test is', test);
  } catch { }
}
if (!test) {
  console.log('build a test first');
  usage();
}

switch (process.argv[2]) {
  case 'configure':
    framework.configure(test, 'inherit', ['--debug']);
    break;
  case 'build':
    framework.build('inherit');
    break;
  case 'run':
    try {
      framework.load(test);
    } catch {
      globalThis.dll = require(path.resolve(__dirname, 'build', 'Debug', `${test.split('.')[0]}.node`));
    }
    globalThis.describe = (name, fn) => {
      console.log(name);
      fn();
    };
    globalThis.it = globalThis.describe;
    framework.register(test);
    break;
  default:
    usage();
}

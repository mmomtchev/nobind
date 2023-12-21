const fs = require('fs');
const path = require('path');
const framework = require('./framework');
const Mocha = require('mocha');

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
  case 'clean':
    framework.clean(test, 'inherit');
    break;
  case 'configure':
    framework.configure(test, 'inherit', ['--debug']);
    break;
  case 'configure-asan':
    framework.configure(test, 'inherit', ['--debug', '--enable_asan']);
    break;
  case 'build':
    framework.build('inherit');
    break;
  case 'run':
    globalThis.dll = require(path.resolve(__dirname, 'build', 'Debug', `${test.split('.')[0]}.node`));
    const mocha = new Mocha({ ui: 'bdd' });
    mocha.addFile(path.resolve(__dirname, 'tests', test));
    mocha.run(function (failures) {
      process.on('exit', function () {
        process.exit(failures);
      });
    });
    break;
  default:
    usage();
}

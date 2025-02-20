const fs = require('fs');
const path = require('path');
const framework = require('./framework');
const Mocha = require('mocha');
const assert = require('assert');

function usage() {
  console.log('Usage: node single.js <configure|build|run|show-types|gen-types|check-types> <test>');
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
    framework.configure(test, 'inherit', ['--debug', '--enable_typescript']);
    break;
  case 'configure-tsdebug':
    framework.configure(test, 'inherit', ['--debug', '--enable_typescript', '--enable_typescript-debug']);
    break;
  case 'configure-nots':
    framework.configure(test, 'inherit', ['--debug', '--enable_typescript=false']);
    break;
  case 'configure-noos':
    framework.configure(test, 'inherit', ['--debug', '--enable_object_store=false']);
    break;
  case 'configure-asan':
    framework.configure(test, 'inherit', ['--debug', '--enable_asan']);
    break;
  case 'build':
    framework.build('inherit');
    break;
  case 'run':
    framework.load(test, 'Debug');
    const mocha = new Mocha({ ui: 'bdd' });
    mocha.addFile(path.resolve(__dirname, 'tests', test));
    mocha.run(function (failures) {
      process.on('exit', function () {
        console.log('Running GC');
        if (global.gc)
          global.gc();
        else
          console.warn('gc() not available');
        process.exit(failures);
      });
    });
    break;
  case 'gen-types':
    framework.load(test, 'Debug');
    framework.gen_typescript('Debug');
    break;
  case 'show-types':
    framework.load(test, 'Debug');
    assert(globalThis.dll.__typescript !== undefined, 'No TypeScript generator');
    console.log(globalThis.dll.__typescript);
    break;
  case 'check-types':
    framework.check_typescript(test, 'inherit');
    break;
  default:
    usage();
}

const fs = require('fs');
const path = require('path');
const { assert } = require('chai');
const { execFileSync } = require('child_process');

function build(test) {
  try {
    fs.rmSync(path.resolve(__dirname, 'build'), { recursive: true, force: true });

    let include;
    const fixtures = [];
    const match = /#include\s+<fixtures\/(.+)\.h>/g;
    const source = fs.readFileSync(path.resolve(__dirname, 'tests', `${test}.cc`), 'utf-8');
    while (include = match.exec(source)) {
      fixtures.push(include[1]);
    }
    execFileSync('npx', [
      'node-gyp',
      'configure',
      `--test=${test}`,
      `--fixtures="${fixtures.map((f) => `fixtures/${f}.cc`).join(' ')}"`
    ], { stdio: 'ignore', cwd: __dirname });
    execFileSync('npx', [
      'node-gyp',
      'build'
    ], { stdio: 'ignore', cwd: __dirname });
  } catch (e) {
    if (e.stdout) {
      console.error(e.stdout);
    }
    throw e;
  }
}

function load(test) {
  globalThis.dll = require(path.resolve(__dirname, 'build', 'Release', `${test.split('.')[0]}.node`));
}

function register(test) {
  require(path.resolve(__dirname, 'tests', `${test}.js`));
}

describe('nobind', function () {
  this.timeout(10000);
  const tests = fs.readdirSync(path.resolve(__dirname, 'tests'))
    .filter((t) => t.endsWith('.cc'))
    .map((t) => t.split('.')[0]);
  for (const t of tests) {
    describe(t, () => {
      register(t);
      before('build', () => build(t));
      before('load', () => load(t));
    });
  }
});

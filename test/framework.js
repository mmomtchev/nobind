const fs = require('fs');
const path = require('path');
const { assert } = require('chai');
const { execFileSync } = require('child_process');

function list() {
  return fs.readdirSync(path.resolve(__dirname, 'tests'))
    .filter((t) => t.endsWith('.cc'))
    .map((t) => t.split('.')[0]);
}

function configure(test, stdio, opts) {
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
      ...(opts || []),
      `--test=${test}`,
      `--fixtures=${fixtures.map((f) => `fixtures/${f}.cc`).join(' ')}`
    ], { stdio: stdio || 'pipe', cwd: __dirname });
  } catch (e) {
    if (e.stdout) {
      console.error(e.stdout);
    }
    throw e;
  }
}

function build(stdio) {
  execFileSync('npx', [
    'node-gyp',
    'build'
  ], { stdio: stdio || 'pipe', cwd: __dirname });
}

function load(test) {
  globalThis.dll = require(path.resolve(__dirname, 'build', 'Release', `${test.split('.')[0]}.node`));
}

function register(test) {
  require(path.resolve(__dirname, 'tests', `${test}.js`));
}

module.exports = {
  list,
  configure,
  build,
  load,
  register
};

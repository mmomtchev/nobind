const fs = require('fs');
const path = require('path');
const { execFileSync } = require('child_process');
const os = require('os');
const assert = require('assert');

const npx = os.platform() === 'win32' ? 'npx.cmd' : 'npx';
const env = { ...process.env };
// Do not pass asan when spawning processes
delete env.LD_PRELOAD;

function list() {
  return fs.readdirSync(path.resolve(__dirname, 'tests'))
    .filter((t) => t.endsWith('.cc'))
    .map((t) => t.split('.')[0]);
}

function clean(stdio) {
  execFileSync(npx, [
    'node-gyp',
    'clean'
  ], { stdio: stdio || 'pipe', cwd: __dirname, env, shell: true });
}

function configure(test, stdio, opts, output) {
  try {
    let include;
    const fixtures = [];
    const match = /#include\s+<fixtures\/(.+)\.h>/g;
    const source = fs.readFileSync(path.resolve(__dirname, 'tests', `${test}.cc`), 'utf-8');
    while (include = match.exec(source)) {
      fixtures.push(include[1]);
    }
    execFileSync(npx, [
      'node-gyp',
      'configure',
      ...(opts || []),
      `--test=${test}`,
      `--test_output=${output ?? test}`,
      `"--fixtures=${fixtures.map((f) => `fixtures/${f}.cc`).join(' ')}"`
    ], { stdio: stdio || 'pipe', cwd: __dirname, env, shell: true });
  } catch (e) {
    if (e.stdout && !stdio) {
      console.error(e.stdout.toString());
    }
    throw e;
  }
}

function build(stdio) {
  try {
    execFileSync(npx, [
      'node-gyp',
      'build'
    ], { stdio: stdio || 'pipe', cwd: __dirname, env, shell: true });
  } catch (e) {
    if (e.stdout && !stdio) {
      console.error(e.stdout.toString());
    }
    throw e;
  }
}

function gen_typescript() {
  assert(globalThis.dll.__typescript !== undefined);
  fs.writeFileSync(path.resolve(__dirname, 'build', 'dll.d.ts'), globalThis.dll.__typescript);
}

function check_typescript(test, stdio) {
  try {
    execFileSync(npx, [
      'tsc',
      '--types @types/mocha,node,./dll.d.ts',
      '--downlevelIteration', '--checkJs', '--noEmit', '--lib es2015',
      `tests/${test}.js`
    ], { stdio: stdio || 'pipe', cwd: __dirname, env, shell: true });
  } catch (e) {
    if (e.stdout && !stdio) {
      console.error(e.stdout.toString());
    }
    throw e;
  }
}

function load(test, build) {
  globalThis.dll = require(path.resolve(__dirname, 'build', build || 'Release', `${test.split('.')[0]}.node`));
}

function register(test) {
  require(path.resolve(__dirname, 'tests', `${test}.js`));
}

module.exports = {
  list,
  clean,
  configure,
  build,
  gen_typescript,
  check_typescript,
  load,
  register
};

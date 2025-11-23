// These are the usual steps for importing a native Node.js
// module directly from TypeScript when it is transpiled to ES6
import { createRequire } from 'node:module';
const require = createRequire(import.meta.url);
const hello = require('./lib/example.node') as typeof import('./lib/example');

const r = hello.handleMonster({
  name: 'Baba Yaga',
  eyes: 2,
  feature: 'horn',
  greeter: new hello.Hello('Baba Yaga')
});

console.log('JS received', r);

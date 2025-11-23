// These are the usual steps for importing a native Node.js
// module directly from TypeScript when it is transpiled to ES6
import { createRequire } from 'node:module';
const require = createRequire(import.meta.url);
const hello = require('./lib/example.node') as typeof import('./lib/example');

const obj = new hello.Hello('Baba Yaga');
console.log(obj.greet());

console.log('1 + 2', '=', hello.add(1, 2));

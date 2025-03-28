// These are the usual steps for importing a native Node.js
// module directly from TypeScript when it is transpiled to ES6
import { createRequire } from 'node:module';
const require = createRequire(import.meta.url);
const hello = require('./lib/hello.node') as typeof import('./lib/hello');

const obj = new hello.Hello('Baba Yaga');
console.log(obj.greet());

console.log('1 + 2', '=', hello.add(1, 2));

console.log(hello.handleMonster({
  name: 'Lamia',
  eyes: 2,
  feature: 'claws',
  greeter: new hello.Hello('Lamia')
}));

hello.handleMonsterPtr({
  name: 'Zmei',
  eyes: 2,
  feature: 'horn',
  greeter: new hello.Hello('Zmei')
});

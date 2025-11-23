// Load the built module and extract the TypeScript definitions

const fs = require('node:fs');
const path = require('node:path');
const input = path.resolve(process.argv[2]);
console.log('loading', input);
const dll = require(input);
const ts = dll.__typescript;
if (!ts) throw new Error('No TypeScript definitions found');
const typings = path.resolve(process.argv[3]);
console.log('writing', typings);
fs.writeFileSync(typings, ts);

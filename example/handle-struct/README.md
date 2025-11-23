# Named fields

An example with a function that accepts an object with named fields on the JS side and calls with a `struct` on the C++ side. 

Given:
```cpp
struct MonsterDefinition {
  std::string name;
  unsigned eyes;
  bool fangs;
  enum Feature { CLAWS, HORN } feature;

  // C++ gets a copy of the object held by JS
  Hello greeter;
};
MonsterDefinition handleMonster(MonsterDefinition v);
```

Generate:
```ts
export function handleMonster(arg0: {
  name: string;
  eyes: number;
  fangs?: boolean;
  feature: 'claws' | 'horn';
  greeter: Hello;
}): {
  name: string;
  eyes: number;
  fangs?: boolean;
  feature: 'claws' | 'horn';
  greeter: Hello;
};
```

To be used as:
```js
hello.handleMonster({
  name: 'Lamia',
  eyes: 2,
  feature: 'claws',
  greeter: new hello.Hello('Lamia')
});
```

## Install

(also automatically builds)

```shell
npm install
```

## Build

```shell
node-gyp configure build
```

## Run

```shell
npx tsx example.ts
```

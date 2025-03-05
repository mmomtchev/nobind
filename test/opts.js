function mocha_object_store() {
  if (!process.env.CXXFLAGS) return true;
  if (process.env.CXXFLAGS.match(/NOBIND_NO_OBJECT_STORE/)) {
    console.warn('Object store is disabled');
    return false;
  }
  return true;
}

function mocha_typescript() {
  if (!process.env.CXXFLAGS) return true;
  if (process.env.CXXFLAGS.match(/NOBIND_NO_TYPESCRIPT_GENERATOR/)) {
    console.warn('TypeScript is disabled');
    return false;
  }
  return true;
}

function mocha_locking() {
  if (!process.env.CXXFLAGS) return true;
  if (process.env.CXXFLAGS.match(/NOBIND_NO_ASYNC_LOCKING/)) {
    console.warn('Async locking is disabled');
    return false;
  }
  return true;
}

module.exports = {
  mocha_object_store, mocha_locking, mocha_typescript
}; 

#include <fixtures/basic_class.h>

#include <nobind.h>

#include <memory>

int take_shared_ptr(std::shared_ptr<Hello> in) { return in->Id(); }

int take_const_shared_ptr(std::shared_ptr<const Hello> in) { return in->Id(); }

std::shared_ptr<Hello> return_shared_ptr(const std::string &name) {
  return std::shared_ptr<Hello>(Hello::Factory(name));
}

int take_unique_ptr(std::unique_ptr<Hello> &in) { return in->Id(); }

std::unique_ptr<Hello> return_unique_ptr(const std::string &name) {
  return std::unique_ptr<Hello>(Hello::Factory(name));
}

// Helper for the unique_ptr class
std::string HelloGreet(const std::unique_ptr<Hello> &o, std::string s) { return o->Greet(s); }

// Stub to test the object store interaction
std::shared_ptr<Hello> take_and_return_shared_ptr(std::shared_ptr<Hello> in) { return in; }

NOBIND_MODULE(smart_ptr, m) {
  m.def<Hello>("Hello")
      .cons<std::string &>()
      .def<&Hello::Id>("get_id")
      .def<&Hello::Greet>("greet")
      .def<&Hello::id, Nobind::ReadOnly>("id")
      .def<&Hello::Factory>("factory");

  m.def<&take_shared_ptr>("takeSharedPtr", "takeSharedPtrAsync");
  m.def<&take_const_shared_ptr>("takeConstSharedPtr", "takeConstSharedPtrAsync");
  m.def<&return_shared_ptr>("returnSharedPtr", "returnSharedPtrAsync");

  m.def<std::unique_ptr<Hello>>("HelloUPtr").ext<&HelloGreet>("greet");
  m.def<&take_unique_ptr>("takeUniquePtr", "takeUniquePtrAsync");
  m.def<&return_unique_ptr>("returnUniquePtr", "returnUniquePtrAsync");

  m.def<&take_and_return_shared_ptr>("takeAndReturnSharedPtr");
}

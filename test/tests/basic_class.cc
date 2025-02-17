#include <exception>
#include <fixtures/basic_class.h>

#include <nobind.h>

// This() is const CLASS &
std::string ToString(const Hello &obj) {
  std::stringstream r;
  r << "Hello {id: " << obj.Id() << "}";
  return r.str();
}

// This() is CLASS &
std::string ToStringWithArg(Hello &obj, int id) {
  std::stringstream r;
  r << "Hello {id: " << obj.Id() << ":" << id << "}";
  return r.str();
}

// This() is CLASS &
void NotIterable(Hello &) { throw std::runtime_error("Not iterable"); }

NOBIND_MODULE(basic_class, m) {
  static constexpr bool False = false;

  m.def<Hello>("Hello")
      .cons<std::string &>()
      .def<&Hello::Id>("get_id")
      .def<&Hello::Greet>("greet")
      .def<&Hello::nothing>("nothing")
      .def<&Hello::id, Nobind::ReadOnly>("id")
      .def<&Hello::var>("var")
      .def<&Hello::Factory>("factory")
      .def<&Hello::StaticHello>("staticObject")
      .def<&False, Nobind::ReadOnly>(Napi::Symbol::WellKnown(m.Env(), "isConcatSpreadable"))
      .def<&Hello::Greet>(Napi::Symbol::WellKnown(m.Env(), "split"))
      .ext<&NotIterable>(Napi::Symbol::WellKnown(m.Env(), "iterator"))
      .ext<&ToString>("toString")
      .ext<&ToStringWithArg>("toStringWithArg");
}

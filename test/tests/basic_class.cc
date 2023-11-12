#include <fixtures/basic_class.h>
#include <exception>

#include <nobind.h>
#include <napi.h>

std::string ToString(Hello &obj) {
  std::stringstream r;
  r << "Hello {id: " << obj.Id() << "}";
  return r.str();
}

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
    .ext<&ToString>("toString");
}

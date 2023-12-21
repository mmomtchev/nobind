#include <fixtures/basic_class.h>
#include <fixtures/global_functions.h>

#include <nobind.h>

// In C++17, only static constexpr variables can be template parameters
// (this is relaxed in later standards)
constexpr auto factoryAttrs = Nobind::ReturnAsync | Nobind::ReturnOwned;

NOBIND_MODULE(async, m) {

  m.def<&add, Nobind::ReturnAsync>("add");
  m.def<&gte, Nobind::ReturnAsync>("gte");
  m.def<&power, Nobind::ReturnAsync>("pow");
  m.def<&hello, Nobind::ReturnAsync>("hello");
  m.def<&nothing, Nobind::ReturnAsync>("nothing");
  m.def<Hello>("Hello")
      .cons<std::string &>()
      .def<&Hello::Id, Nobind::ReturnAsync>("get_id")
      .def<&Hello::Greet, Nobind::ReturnAsync>("greet")
      .def<&Hello::nothing, Nobind::ReturnAsync>("nothing")
      .def<&Hello::Factory, factoryAttrs>("factory");
}

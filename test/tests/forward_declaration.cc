#include <fixtures/basic_class.h>
#include <fixtures/dependant_class.h>

#include <nobind.h>

NOBIND_MODULE(forward_declaration, m) {

  m.decl<Hello>("Hello");

  m.def<Dependant>("Dependant").cons<const Hello &>().def<&Dependant::Get>("get");

  m.def<Hello>("Hello").cons<std::string &>().def<&Hello::Greet>("greet");
}

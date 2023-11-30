#include <fixtures/template.h>

#include <nobind.h>

NOBIND_MODULE(template, m) { m.def<Template<int>>("int").cons<int>().def<&Template<int>::getter>("get"); }

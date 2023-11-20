%module swig

%include "std_string.i"

%rename Strlen strlen;
%rename Len length;

%feature("async", "Async") Strlen;

%include "string.h"

%{
#include "string.h"
%}

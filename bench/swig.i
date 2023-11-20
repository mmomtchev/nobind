%module swig

%include "std_string.i"

%rename Strlen strlen;
%rename Len length;

%include "string.h"

%{
#include "string.h"
%}

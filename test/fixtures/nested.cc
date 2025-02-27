#include "nested.h"

Time::Time(unsigned long v) : timestamp(v) {};
Time::operator unsigned long() const { return timestamp; };

DateTime::DateTime(Time v) : time(v) {};
DateTime::operator Time &() { return time; };
DateTime::operator Time *() { return &time; };

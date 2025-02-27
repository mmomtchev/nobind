class Time {
  unsigned long timestamp;

public:
  Time(unsigned long v);
  operator unsigned long() const;
};

class DateTime {
public:
  Time time;
  DateTime(Time v);
  operator Time &();
  operator Time *();
};

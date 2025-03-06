struct Critical {
  int counter;

  Critical();
  void Increment(int);
  int Get();
};

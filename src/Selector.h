#ifndef Selector_h
#define Selector_h

#include "Arduino.h"


class Selector {
  const int* pins;
public:
  Selector(const int* pins) : pins(pins) {}
  
  void initialize();
  int get();
};

#endif
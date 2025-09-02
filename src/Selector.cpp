#include "Selector.h"

void Selector::initialize() {
  for (int i=0;i<6;i++) {
    pinMode(pins[i], INPUT_PULLDOWN);
  }
}

int Selector::get() {
    for (int i=0;i<6;i++) {
      // Serial.print(digitalRead(pins[i]));
      // Serial.print(" ");
      if (digitalRead(pins[i]) == HIGH) {
        return (int)(i+1);
      }
    }
    // Serial.println();
    return 0;
}
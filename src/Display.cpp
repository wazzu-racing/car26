#include "Display.h"

void startsequence(Adafruit_7segment matrix1, Adafruit_7segment matrix2) {
  displayText("88888888", matrix1, matrix2);
  for (int i=0;i<7;i++) {
    digitalWrite(ALL_LEDS[(2*i)], HIGH);
    digitalWrite(ALL_LEDS[(2*i)+1], HIGH);
    delay(50);
  }
  digitalWrite(CHECK_ENGINE, HIGH);

  for (int j=0;j<3;j++) {
    for (int i=0;i<14;i++) {digitalWrite(ALL_LEDS[i], HIGH);}
    digitalWrite(CHECK_ENGINE, HIGH);
    delay(300);
    for (int i=0;i<14;i++) {digitalWrite(ALL_LEDS[i], LOW);}
    digitalWrite(CHECK_ENGINE, LOW);
    delay(300);
  }
  for (int i=0;i<14;i++) {digitalWrite(ALL_LEDS[i], LOW);}
  digitalWrite(CHECK_ENGINE, LOW);
  off(matrix1, matrix2);
}

void off(Adafruit_7segment matrix1, Adafruit_7segment matrix2) {
  for (int i=0;i<14;i++) {digitalWrite(ALL_LEDS[i], LOW);}
  displayText("        ", matrix1, matrix2);
}

void lightSequence(void) {
  int time = 5; // seconds
  int steps = 7 + 7 + 4 + 14 + 14 + 8;
  int step = (((millis() % (time * 1000)) / (time * 1000.0)) * steps);

  // Serial.print("...");
  // Serial.println(step);

  if (step < 7) {
    for (int i=0;i<7;i++) {
      digitalWrite(ALL_LEDS[2*i], (step==i) ? HIGH : LOW);
      digitalWrite(ALL_LEDS[(2*i)+1], (step==i) ? HIGH : LOW);
    }
  } else if (step < 14) {
    for (int i=0;i<7;i++) {
      digitalWrite(ALL_LEDS[2*i], ((step-7)==(6-i)) ? HIGH : LOW);
      digitalWrite(ALL_LEDS[(2*i)+1], ((step-7)==(6-i)) ? HIGH : LOW);
    }
  }
  else if (step < 18) {for (int i=0;i<14;i++) {digitalWrite(ALL_LEDS[i], (step%2) ? HIGH : LOW);}}
  else if (step < 32){
    for (int i=0;i<7;i++) {digitalWrite(ALL_LEDS[2*i], ((step-18)==i) ? HIGH : LOW);}
    for (int i=0;i<7;i++) {digitalWrite(ALL_LEDS[(2*i)+1], ((step-18)==((6-i)+7)) ? HIGH : LOW);}
  }
  else if (step < 46){
    for (int i=0;i<7;i++) {digitalWrite(ALL_LEDS[(2*i)+1], ((step-32)==i) ? HIGH : LOW);}
    for (int i=0;i<7;i++) {digitalWrite(ALL_LEDS[2*i], ((step-32)==((6-i)+7)) ? HIGH : LOW);}
  }
  else {for (int i=0;i<14;i++) {digitalWrite(ALL_LEDS[i], (step%2) ? HIGH : LOW);}}
}

// returns number of digits in num
int n_digits(int num) {
  if (num == 0) {
    return 1;
  }
  return floor(log10(abs(num))) + 1;
}

void displayText(String s, Adafruit_7segment matrix1, Adafruit_7segment matrix2) {
  if (s.length() > 4) {
    matrix2.print(s.substring(0, 4));
    matrix1.print(s.substring(4));
  } else {
    matrix2.print(s);
    matrix1.print("");
  }
  matrix1.writeDisplay();
  matrix2.writeDisplay();
}

void displayInt(int value, Adafruit_7segment matrix1, Adafruit_7segment matrix2) {
  matrix1.setDisplayState(true);
  matrix2.setDisplayState(true);

  int number_digits = n_digits(value);

  if (number_digits > 1) {
    int size2 = 0;
    int split = 0;
    int split2 = 0;
    int display1 = 0;
    int hold = 0;
    String display2 = "\0";

    char* temp1 = "";
    String temp2 = "";

    if (number_digits % 2 == 0) {
      split = split2 = number_digits/2;
    } else {
      split = (number_digits/2);
      split2 = number_digits/2 +1;
    }

    display1 = value / pow(10, split);
    
    hold = abs(value - (display1 * pow(10, split)));
    sprintf(temp1, "%d", hold);
    temp2 = temp1;

    int t = n_digits(hold);

    if (n_digits(hold) + n_digits(display1) < number_digits) {
      temp2 += "0";
      t++;
    }

    for (int i = 0; i < 4; i++) {
      if (i >= t) {
        temp2+= " ";
      }
    }
    
    matrix2.println(display1);
    matrix1.println(temp2);
    matrix1.writeDisplay();
    matrix2.writeDisplay();

  } else {

    String numnum = "";

    char* help = "";

    sprintf(help, "%d", value);

    numnum = help;
    for (int i = 0; i < 4; i++) {
      numnum += " ";
    }

    matrix1.println(numnum);
    matrix1.writeDisplay();
    matrix2.println();
    matrix2.writeDisplay();
  }

}

int error_bounces = 0;
int error_time = 0;
const int ERROR_DEBOUNCE_MIN = 40;
const int ERROR_FLASH_TIME = 1000;
bool displaying(MegaSquirt3 ecu, Adafruit_7segment matrix1, Adafruit_7segment matrix2) {
  int rpm = ecu.data.rpm;
  int coolant = ecu.data.clt;
  int oilPressure = ecu.data.sensors1;

  if (coolant > 230) {
    if (error_bounces++ < ERROR_DEBOUNCE_MIN) {return false;}
    // here we def have error after min # of bounces
    if (error_time == 0) {error_time = millis();}

    if ((millis() - error_time) < (ERROR_FLASH_TIME)) { displayText("CLNt tEP", matrix1, matrix2); }
    else if ((millis() - error_time) < (2*ERROR_FLASH_TIME)) { displayInt(coolant, matrix1, matrix2); }
    else if ((millis() - error_time) < (4*ERROR_FLASH_TIME)) { displayInt(rpm, matrix1, matrix2); }
    else { error_time = 0; error_bounces = 0;}
    return true;
  }
  else if (oilPressure  > 80 || oilPressure < -1) {
    if (error_bounces++ < ERROR_DEBOUNCE_MIN) {return false;}
    // here we def have error after min # of bounces
    if (error_time == 0) {error_time = millis();}

    if ((millis() - error_time) < (ERROR_FLASH_TIME)) { displayText("OIL PRES", matrix1, matrix2); }
    else if ((millis() - error_time) < (2*ERROR_FLASH_TIME)) { displayInt(oilPressure, matrix1, matrix2); }
    else if ((millis() - error_time) < (4*ERROR_FLASH_TIME)) { displayInt(rpm, matrix1, matrix2); }
    else { error_time = 0; error_bounces = 0;}
    return true;
  }
  else {
    return false;
  }
}

// SET RPM ==============================================================================================================================================================
bool ison = false;
bool carIsOn() {return ison;}
void set_rpm(int i) {
  if (i > 5000 && !ison) {
    ison = true;
  }
  if (i < 2500 && ison) {
    ison = false;
  }
  if (!ison) {
    for (int k=0;k<14;k++) {digitalWrite(ALL_LEDS[k], LOW);}
    return;
  }

  bool evenodd = ((int)floor(millis()/150)) % 2;

  if (i <= 6500) {
    digitalWrite(ALL_LEDS[0], evenodd ? HIGH : LOW);
    digitalWrite(ALL_LEDS[1], evenodd ? HIGH : LOW);
    for (int j = 2; j < 14; j++) {  
      digitalWrite(ALL_LEDS[j], LOW);
    }
  } 
  else if (i > 6500 && i < 9500) {
    int numleds = 7 * ((i - 6500) / (9500.0 - 6500.0));
    // Serial.println(numleds);
    for (int j = 0; j < 7; j++) {
      digitalWrite(ALL_LEDS[j*2], (j <= numleds) ? HIGH : LOW);
      digitalWrite(ALL_LEDS[(j*2)+1], (j <= numleds) ? HIGH : LOW);
    }
   
  } 
  else {
    for (int j = 0; j < 14; j++) {
      digitalWrite(ALL_LEDS[j], evenodd ? HIGH : LOW);
    }
  }
}

void brakepressure(int b1, int b2, Adafruit_7segment matrix1, Adafruit_7segment matrix2) {
  int a = floor(max(0, 5000*(((b1/5024.0)-0.1)/0.8)));
  int b = floor(max(0, 5000*(((b2/5024.0)-0.1)/0.8)));

  char* c1 = "";
  char* c2 = "";
  sprintf(c1, "%03d ", a);
  sprintf(c2, " %03d", b);

  String s1 = String(c1);
  String s2 = String(c2);

  displayText(s1 + s2, matrix1, matrix2);
}
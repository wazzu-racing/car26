// LIBRARIES ============================================================================================================================================================
#include "MegaSquirt3.h"
#include "BMI088.h"
#include <Wire.h>
#include <SD.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include "Selector.h"
#include "Display.h"

// SETTINGS =============================================================================================================================================================
bool is_dashboard = false;
const int WRITE_FREQ = 10; // ms
const int DATA_SWITCH = 28;
const int STATUS_A = 12;
const int STATUS_B = 11;
const int STATUS_C = 10;
const int STATUS_D = 9;
const int SELECTOR_PINS[] = {35, 36, 37, 38, 39, 40};

// INSTANCES ============================================================================================================================================================
Bmi088Accel accel(Wire,0x18);
Bmi088Gyro gyro(Wire,0x68);
SFE_UBLOX_GNSS myGNSS;
Selector selector(SELECTOR_PINS);
MegaSquirt3 ecu;
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can;
Adafruit_7segment matrix1 = Adafruit_7segment();
Adafruit_7segment matrix2 = Adafruit_7segment();

File file;
ROW tosend;

// GLOBAL VARS ==========================================================================================================================================================
int re;
uint32_t lastwrite = 0;
int mode = 0;
int mode_bounces = 0;
bool offset;

int TEST = 0;

void openfile(String filename) {
  // Serial.println(filename);
  int i = 0;
    while (SD.exists((String(i) + ".bin").c_str())) {
      i++;
    }
    file = SD.open((String(i) + ".bin").c_str(), FILE_WRITE);
  // file = SD.open(filename.c_str(), FILE_WRITE);
}

// MAIN INTERRUPT =======================================================================================================================================================
bool is_logging = true;
int is_logging_bounce = 0;
void handler(const CAN_message_t &msg) {
 
  // Serial.print("data switch: ");
  // Serial.println(digitalRead(DATA_SWITCH));

  if ((digitalRead(DATA_SWITCH)==HIGH) != is_logging) {
    // Serial.println(is_logging_bounce);
    if (is_logging_bounce++ > 40) {
      is_logging = !is_logging;
      if (is_logging) openfile("new");
      is_logging_bounce = 0;
    }
  }
  offset = !offset;
  if (!is_dashboard) {
    digitalWrite(STATUS_D, offset ? HIGH : LOW);
  }
  //  Serial.println("message");
   // log directly wired sensors:
   if (!is_dashboard) {
     // log accelerometer
     accel.readSensor();
     tosend.ax = accel.getAccelX_mss() * 1000;
     tosend.ay = accel.getAccelY_mss() * 1000;
     tosend.az = accel.getAccelZ_mss() * 1000;
     tosend.accel_millis = millis();
 
    //  // log gyro
     gyro.readSensor();
     tosend.imu_x = gyro.getGyroX_rads() * 1000;
     tosend.imu_y = gyro.getGyroY_rads() * 1000;
     tosend.imu_z = gyro.getGyroZ_rads() * 1000;
     tosend.imu_millis = millis();
 
     // log GPS
    //   Serial.print("+");
    //  if (myGNSS.getPVT(10)) {
    //   Serial.print("-");tosend.unixtime = myGNSS.getUnixEpoch();
    //   Serial.print("-");tosend.lat = myGNSS.getLatitude();
    //   Serial.print("-");tosend.lon = myGNSS.getLongitude();
    //   Serial.print("-");tosend.elev = myGNSS.getAltitude();
    //   Serial.print("-");tosend.ground_speed = myGNSS.getGroundSpeed();	
    //   Serial.print("-");tosend.gps_millis = millis();
    //  }
     Serial.print("done\n");
   }

    // ecu.data.rpm = 90;

    // Serial.println("This is the rpm");
    // Serial.println(ecu.data.rpm);

  if (ecu.decode(msg)) {
     tosend.rpm = ecu.data.rpm;
     tosend.time = ecu.data.seconds;
     tosend.afr = ecu.data.AFR1 * 1000;
     tosend.fuelload = ecu.data.fuelload * 1000;
     tosend.spark_advance = ecu.data.adv_deg * 1000;
     tosend.baro = ecu.data.baro * 1000;
     tosend.map = ecu.data.map * 1000;
     tosend.mat = ecu.data.mat * 1000;
     tosend.clnt_temp = ecu.data.clt * 1000;
     tosend.tps = ecu.data.tps * 1000;
     tosend.batt = ecu.data.batt * 1000;
     tosend.oil_press = ecu.data.sensors1 * 1000;
     tosend.syncloss_count = ecu.data.synccnt;
     TEST += ecu.data.synccnt;
    //  Serial.print("Sync loss total:\t");
    //  Serial.println(TEST);
     tosend.syncloss_code = ecu.data.syncreason;
     tosend.ltcl_timing = ecu.data.launch_timing * 1000;
     tosend.ve1 = ecu.data.ve1 * 1000;
     tosend.ve2 = ecu.data.ve2 * 1000;
     tosend.egt = ecu.data.egt1 * 1000;
     tosend.maf = ecu.data.MAF * 1000;
     tosend.in_temp = ecu.data.airtemp * 1000;
     tosend.ecu_millis = millis();
    // check engine
    bool evenodd = ((int)floor(millis()/150)) % 2;
    bool engine_bad = (ecu.data.clt > 215) || (ecu.data.sensors1 < 5) || (ecu.data.sensors1 > 125) || (ecu.data.batt > 15) || (ecu.data.batt < 7);
    bool engine_really_bad = (ecu.data.clt > 230) || (ecu.data.sensors1 > 150) || (ecu.data.sensors1 < 1);

    if (carIsOn()) {
      if (engine_really_bad) {
        digitalWrite(CHECK_ENGINE, evenodd);
      } else {
        digitalWrite(CHECK_ENGINE, engine_bad);
      }
    }
    // show displays
    // Serial.print(ecu.data.syncreason);
    // Serial.print(", ");
    // Serial.println(ecu.data.synccnt);
 
  } else if (msg.id == (935424 +20)) {
    // Serial.print("AnalogX 1\n");
    int a = ((msg.buf[0]) + (msg.buf[1]<<8));
    int b = ((msg.buf[2]) + (msg.buf[3]<<8));
    int c = ((msg.buf[4]) + (msg.buf[5]<<8));
    int d = ((msg.buf[6]) + (msg.buf[7]<<8));
    // Serial.print(a);Serial.print(" ");
    // Serial.print(b);Serial.print(" ");
    // Serial.print(c);Serial.print(" ");
    // Serial.print(d);Serial.print("\n");
    tosend.brake1 = a;
    tosend.brake2 = b;
    tosend.susp_pot_1 = c;
    tosend.susp_pot_2 = d;
    tosend.analogx1_millis = millis();
  } else if (msg.id == (935680 +20)) {
    // Serial.print("AnalogX 2\t");
    int a = ((msg.buf[0]) + (msg.buf[1]<<8));
    int b = ((msg.buf[2]) + (msg.buf[3]<<8));
    int c = ((msg.buf[4]) + (msg.buf[5]<<8));
    int d = ((msg.buf[6]) + (msg.buf[7]<<8));
    tosend.analogx2_millis = millis();
    // Serial.print(a);Serial.print(" ");
    // Serial.print(b);Serial.print(" ");
    // Serial.print(c);Serial.print(" ");
    // Serial.print(d);Serial.print("\n");
    tosend.susp_pot_4 = c;
    tosend.rad_in = b;
    tosend.rad_out = a;
    tosend.susp_pot_3 = d;

  } else if (msg.id == (935936 +20)) {
    // Serial.print("AnalogX 3\t");
    int a = ((msg.buf[0]) + (msg.buf[1]<<8));
    int b = ((msg.buf[2]) + (msg.buf[3]<<8));
    int c = ((msg.buf[4]) + (msg.buf[5]<<8));
    int d = ((msg.buf[6]) + (msg.buf[7]<<8));
    tosend.analogx3_millis = millis();
    // Serial.print(a);Serial.print(" ");
    // Serial.print(b);Serial.print(" ");
    // Serial.print(c);Serial.print(" ");
    // Serial.print(d);Serial.print("\n")
  }
  // else if (msg.id == (936192 +20)) {
  //   // Serial.print("AnalogX 4\t");
  //   int a = ((msg.buf[0]) + (msg.buf[1]<<8));
  //   int b = ((msg.buf[2]) + (msg.buf[3]<<8));
  //   int c = ((msg.buf[4]) + (msg.buf[5]<<8));
  //   int d = ((msg.buf[6]) + (msg.buf[7]<<8));
  //   // Serial.print(a);Serial.print(" ");
  //   // Serial.print(b);Serial.print(" ");
  //   // Serial.print(c);Serial.print(" ");
  //   // Serial.print(d);Serial.print("\n");
    
  // }
  // write to SD card if time in time
  if (is_logging) {
    if (!is_dashboard && millis() > lastwrite + WRITE_FREQ) {
      tosend.write_millis = millis();
      file.write((byte*) &tosend, sizeof(tosend));
      int ti = millis();
      Serial8.write((byte*) &(ti), sizeof(ti));
      Serial.printf("sent %d bytes to LoRa...\n", sizeof(ti));
      Serial8.printf("\n\n\n");
      // file.write("END\n");
      file.flush();
      Serial8.flush();
      // Serial.print("WROTE ############################################################## ");
      Serial.println(millis());
      lastwrite = millis();
    }
  }

  String MODE_NAMES[] = {"0    OFF", "GO COUGS", "1   NORN", "2    RPN", "3   CLNt", "4   OIL ", "5   BRAC"};

  if (is_dashboard) {
    int newmode = selector.get();
    // Serial.print("newmode: ");
    // Serial.print(newmode);
    // Serial.print(", ");
    // Serial.println(millis());
    if (newmode != mode) {
      mode_bounces++;
      if (mode_bounces > 70) {
        mode = newmode;
        mode_bounces = 0;
        displayText(MODE_NAMES[mode], matrix1, matrix2);
        delay(800);
      }
    }


    switch (mode) {
      case 0:
        // off
        off(matrix1, matrix2);
        break;
      case 1:
        // standby
        displayText("GO COUGS", matrix1, matrix2);
        lightSequence(); // non blocking
        break;
      // case 1:
      //   // mph
      //   // displayInt(round(tosend.ground_speed/ 447.04), matrix1, matrix2);
      //   displayInt(tosend., matrix1, matrix2);
      //   set_rpm(ecu.data.rpm);
      //   break;
      case 2:
        // standard running
        if (!carIsOn() || !displaying(ecu, matrix1, matrix2)) {
          set_rpm(ecu.data.rpm);
          displayInt(ecu.data.rpm, matrix1, matrix2);
        }
        break;
      case 3:
        // rpm only
        displayInt(ecu.data.rpm, matrix1, matrix2);
        set_rpm(ecu.data.rpm);
        break;
      case 4:
        // coolant temp
        displayInt(ecu.data.clt, matrix1, matrix2);
        set_rpm(ecu.data.rpm);
        break;
      case 5:
        // oil pressure
        displayInt(ecu.data.sensors1, matrix1, matrix2);
        set_rpm(ecu.data.rpm);
        break;
      case 6:
        // brake pressure
        // matrix1.printNumber(5000*((tosend.brake1-0.1)/0.8));
        // matrix2.printNumber(5000*((tosend.brake2-0.1)/0.8));
        // matrix1.writeDisplay();
        if (((int)floor(millis()/150)) % 2) {
          brakepressure(tosend.brake1, tosend.brake2, matrix1, matrix2);
        }
        // Serial.println(5000*((tosend.brake2-0.1)/0.8));
        // matrix2.writeDisplay();
        set_rpm(ecu.data.rpm);
        break;
      default:
        break;
    }
  }
}

// SETUP ================================================================================================================================================================
void setup() {
  Serial.begin(9600);
  Serial8.begin(9600);
  matrix1.begin(0x70, &Wire);
  matrix2.begin(0x70, &Wire1);

  if (is_dashboard) {

    pinMode(DATA_SWITCH, INPUT_PULLDOWN);
    for (int i = 0; i < 14; i++) {
      pinMode(ALL_LEDS[i], OUTPUT);
    }
    pinMode(CHECK_ENGINE, OUTPUT);
    selector.initialize();
    startsequence(matrix1, matrix2);
  } else {
    pinMode(STATUS_A, OUTPUT);
    digitalWrite(STATUS_A, LOW);
    pinMode(STATUS_B, OUTPUT);
    digitalWrite(STATUS_B, LOW);
    pinMode(STATUS_C, OUTPUT);
    digitalWrite(STATUS_C, LOW);
    pinMode(STATUS_D, OUTPUT);
    digitalWrite(STATUS_D, LOW);


    if (!SD.begin(BUILTIN_SDCARD)) {
      Serial.println(F("SD CARD FAILED, OR NOT PRESENT!"));
      while(1);
    }
    digitalWrite(STATUS_A, HIGH);

    Serial.println("@@@@@@@@@@@@@@@@@@");
    accel.begin();
    gyro.begin();

    pinMode(24, INPUT_PULLUP);
    pinMode(25, INPUT_PULLUP);

    Wire2.begin();
    if (myGNSS.begin(Wire2) == false) { //Connect to the u-blox module using Wire port {
      Serial.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing."));
      while (1);
    }
    digitalWrite(STATUS_B, HIGH);

    myGNSS.checkUblox();
    // file = SD.open("x", FILE_WRITE);
    String filename = String(myGNSS.getYear()) + "-" + String(myGNSS.getMonth()) + "-" + String(myGNSS.getDay()) +\
                    "-" + String(myGNSS.getHour()) + ":" + String(myGNSS.getMinute()) + ":" + String(myGNSS.getSecond()) + ".bin";
    openfile(filename);
    // file.write((byte*) &tosend, sizeof(tosend));
    // file.write("hello\n", 6);
    file.flush();

    // file.close();
    Serial.println(filename);
  }

  Can.begin();
  Can.setBaudRate(500000); //set to 500000 for normal Megasquirt usage - need to change Megasquirt firmware to change MS CAN baud rate
  Can.setMaxMB(16); //sets maximum number of mailboxes for FlexCAN_T4 usage
  Can.enableFIFO();
  Can.enableFIFOInterrupt();
  Can.mailboxStatus();
  Can.onReceive(handler); //when a CAN message is received, runs the canMShandler function
}
void loop(){
  // Serial.println(millis());
  if (!is_dashboard) {
    digitalWrite(STATUS_A, HIGH);
    digitalWrite(STATUS_B, HIGH);
    digitalWrite(STATUS_C, HIGH);
  }
  CAN_message_t a;
  // handler(a);
  // matrix2.print(selector.get(), DEC);
  // matrix2.writeDisplay();
  // delay(100);
  // Serial.println(millis());
  Can.events();
}
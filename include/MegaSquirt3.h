#ifndef MegaSquirt3_h
#define MegaSquirt3_h

#include "Arduino.h"
#include <FlexCAN_T4.h>
#include <MegaCAN.h>

#define CAN_BASE_ID 1512
#define CAN_ID CAN_BASE_ID + 1


 typedef struct row {
  int write_millis;
  int ecu_millis;
  int gps_millis;
  int imu_millis;
  int accel_millis;
  int analogx1_millis;
  int analogx2_millis;
  int analogx3_millis;
  int rpm;
  int time;
  int syncloss_count;
  int syncloss_code;
  int lat;
  int lon;
  int elev;
  int unixtime;
  int ground_speed;
  // floats...
  int afr;
  int fuelload;
  int spark_advance;
  int baro;
  int map;
  int mat;
  int clnt_temp;
  int tps;
  int batt;
  int oil_press;
  int ltcl_timing;
  int ve1;
  int ve2;
  int egt;
  int maf;
  int in_temp;
  int ax;
  int ay;
  int az;
  int imu_x;
  int imu_y;
  int imu_z;
  int susp_pot_1;
  int susp_pot_2;
  int susp_pot_3;
  int susp_pot_4;
  int rad_in;
  int rad_out;
  int amb_air_temp;
  int brake1;
  int brake2;
 } ROW;
 

class MegaSquirt3 {
  MegaCAN megaCAN;

public:
  MegaCAN_broadcast_message_t data;

  MegaSquirt3() : megaCAN(CAN_BASE_ID) {}
  void initialize();
  void eventloop();
  bool decode(const CAN_message_t &msg);
};

#endif
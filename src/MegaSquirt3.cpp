#include "MegaSquirt3.h"


void MegaSquirt3::initialize() {

}

void MegaSquirt3::eventloop() {
  // Can.events();
}

bool MegaSquirt3::decode(const CAN_message_t &msg) {
  if (!msg.flags.extended) {
    megaCAN.getBCastData(msg.id, msg.buf, data);
    if (msg.id <= 900000) {
          Serial.print("RPM:\t\t\t");Serial.print(data.rpm);Serial.println("\tRPM");
          Serial.print("Time:\t\t\t");Serial.print(data.seconds);Serial.println("\tseconds");
          Serial.print("AFR1:\t\t\t");Serial.print(data.AFR1);Serial.println("\tAFR");
          Serial.print("spark advance:\t\t");Serial.print(data.adv_deg);Serial.println("\tdeg BTDC");
          Serial.print("barometric pressure:\t");Serial.print(data.baro);Serial.println("\tkPa");
          Serial.print("manifold pressure:\t");Serial.print(data.map);Serial.println("\tkPa");
          Serial.print("manifold temp:\t\t");Serial.print(data.mat);Serial.println("\tdeg F");
          Serial.print("coolant temp:\t\t");Serial.print(data.clt);Serial.println("\tdeg F");
          Serial.print("throttle position:\t");Serial.print(data.tps);Serial.println("\t%");
          Serial.print("battery voltage:\t");Serial.print(data.batt);Serial.println("\tV");
          Serial.print("oil pressure (egov2):\t");Serial.print(data.sensors1);Serial.println("\t");
          Serial.print("sync loss counter:\t");Serial.print(data.synccnt);Serial.println("\tcount");
          Serial.print("sync reason:\t\t");Serial.print(data.syncreason);Serial.println("\t(code)");
          Serial.print("launch control timing:\t");Serial.print(data.launch_timing);Serial.println("\tdeg");
          Serial.print("VE1:\t\t\t");Serial.print(data.ve1);Serial.println("\t%");
          Serial.print("VE2:\t\t\t");Serial.print(data.ve2);Serial.println("\t%");
          Serial.print("EGT:\t\t\t");Serial.print(data.egt1);Serial.println("\tdeg F");
          Serial.print("Mass air flow:\t\t");Serial.print(data.MAF);Serial.println("\tg/s");
          Serial.print("Intake air temp:\t");Serial.print(data.airtemp);Serial.println("\tdeg F");
          Serial.print("Gear:\t\t\t");Serial.print(data.gear);Serial.println("\t");
          Serial.print("\n\n");Serial.println("\t()");
          Serial.print(millis());
          Serial.println();

          


    // Serial.println(data.map);
      return true;
    }
  }
  return false;
}
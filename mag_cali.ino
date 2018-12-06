//#include <Serial.h>
#include <Arduino.h>
#include <EEPROM.h>

#include "eep_addr.h"

#define MOTION_CAL_EEP_ADDR    1
#define NXP_MOTION_CAL_SIZE    66
#define MOTION_CAL_VALID       0
enum {
  CALI_STATE_IDLE,
  CALI_STATE_START,
  CALI_STATE_RX_DATA,
  CALI_STATE_CHECK_CRC,
  CALI_STATE_SAVE
};
float offset;
int state = CALI_STATE_IDLE;
byte cal[66]; // buffer to receive magnetic calibration data
int i;

uint8_t caliParmIsValid()
{
  return (EEPROM.read(MOTION_CAL_VALID) == 0x5A);
}
boolean status = false;
//void saveCaliParm(byte cal, int size)
//{
//  for (i = 0; i < size; i++)
//    EEPROM.write(MOTION_CAL_EEADDR + i, cal[i]);
//}

void MagCaliIdle(void)
{
  byte c;
  
  if (Serial.available()) {
    c = Serial.read();
//    Serial.print(c);
    switch (state) {
      case CALI_STATE_IDLE:
        if (c == 117) {
          state++;
        }
        break;

      case CALI_STATE_START:
        if (c == 84) {
          state++;
          i = 0;
        } else {
          state = CALI_STATE_IDLE;
        }
        break;
        
      case CALI_STATE_RX_DATA:
        cal[i] = c;
        EEPROM.write(MOTION_CAL_EEP_ADDR + i, cal[i]);
        i++;
        if (i == 66) {
          state++;
        }
        break;
        
      case CALI_STATE_CHECK_CRC:
        //TODO:check crc
		    //offset = cal[0]
        //digitalWrite(LED_BUILTIN, HIGH);
        state++;
        break;

      case CALI_STATE_SAVE:
//        EEPROM.put(EEP_ADDR_MAG_X_OFFSET, cal[0]);
//        EEPROM.put(EEP_ADDR_MAG_Y_OFFSET, cal[1]);
//        EEPROM.put(EEP_ADDR_MAG_Z_OFFSET, cal[2]);
//        saveCaliParm(cal, sizeof(cal));
//        for (i = 0; i < sizeof(cal); i++)
//          EEPROM.write(MOTION_CAL_EEP_ADDR + i, cal[i]);
//        EEPROM.write(MOTION_CAL_VALID, 0x5A);
        state = CALI_STATE_IDLE;
        digitalWrite(LED_BUILTIN, HIGH);
        break;
         
      default:
        state = CALI_STATE_IDLE;
        break;
    }
  }
}

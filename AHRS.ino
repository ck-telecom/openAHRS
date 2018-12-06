/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Blink
*/
#include <Wire.h>
#include <EEPROM.h>
#include <LIS3MDL.h>
#include <LSM6DS0.h>
#include <MadgwickAHRS.h>

Madgwick filter;

#define G_PER_COUNT 0.0001220703125F  // = 1/8192
#define DEG_PER_SEC_PER_COUNT 0.0625F  // = 1/16
#define UT_PER_COUNT 0.1F // 1/10

LIS3MDL mag;
LSM6DS0 ag;
int eeAddr;
uint8_t mag_cali_valid;

int16_t mx, my, mz;
int16_t ax, ay, az;
int16_t gx, gy, gz;
float vec[9];
float roll, pitch, heading;
  
float mag_cali[3];

float mag_offset[3] = {
  0.16, -0.29, 3.12
};
#if 0
float mag_softiron_matrix[3][3] = {
  { 2.899341, -0.068472, -0.026199 },
  { 0, 2.86926, -0.05707 },
  { 0, 0, 1.62849}
};
#else // magneto 1.2
float mag_softiron_matrix[3][3] = {
  { 1.227, 0.034, 0.030 },
  { 0.034, 1.189, -0.027 },
  { 0.029, -0.028, 0.0689 }
};
#endif

void apply_mag_cali(float vec[3], float out[3])
{
  float x = vec[0] - mag_offset[0];
  float y = vec[1] - mag_offset[1];
  float z = vec[2] - mag_offset[2];

  out[0] = x * mag_softiron_matrix[0][0] + y * mag_softiron_matrix[0][1] + z * mag_softiron_matrix[0][2];
  out[1] = x * mag_softiron_matrix[1][0] + y * mag_softiron_matrix[1][1] + z * mag_softiron_matrix[1][2];
  out[2] = x * mag_softiron_matrix[2][0] + y * mag_softiron_matrix[2][1] + z * mag_softiron_matrix[2][2];
}
// the setup function runs once when you press reset or power the board
void setup() {

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  Wire.begin();

  mag.enableDefault();
  ag.setA_ODR(0);
  ag.setG_ODR(0);

  mag_cali_valid = caliParmIsValid();
  if (mag_cali_valid) {
    // load mag cali parameter
  }
}

// the loop function runs over and over again forever
void loop() {
    int i;
  float val;
    for (i = 0, eeAddr = 25; i < 7; i++, eeAddr += sizeof(float)) {
    EEPROM.get(eeAddr, val);
    Serial.println(val);
  }
  MagCaliIdle();
  mx = mag.readX();
  my = mag.readY();
  mz = mag.readZ();
  
  gx = ag.readG_X();
  gy = ag.readG_Y();
  gz = ag.readG_Z();
  
  ax = ag.readA_X();
  ay = ag.readA_Y();
  az = ag.readA_Z();

  vec[0] = ag.getScaled_XL(ax);
  vec[1] = ag.getScaled_XL(ay);
  vec[2] = ag.getScaled_XL(az);
#if 1 // adjust MotionCal scale
  ax = vec[0] * 8192;
  ay = vec[1] * 8192;
  az = vec[2] * 8192;
#endif
  vec[3] = ag.getScaled_G(gx);
  vec[4] = ag.getScaled_G(gy);
  vec[5] = ag.getScaled_G(gz);
#if 1 // adjust MotionCal scale
  gx = vec[3] * 16;
  gy = vec[3] * 16;
  gz = vec[4] * 16;
#endif
  vec[6] = mag.getScaledVal(mx);
  vec[7] = mag.getScaledVal(my);
  vec[8] = mag.getScaledVal(mz);
#if 1 // adjust MotionCal scale
  mx = vec[6] * 1000;
  my = vec[7] * 1000;
  mz = vec[8] * 1000;
#endif  
  apply_mag_cali(&vec[6], mag_cali);  
//  Serial.print(vec[0]);  Serial.print(' ');
//  Serial.print(vec[1]);  Serial.print(' ');
//  Serial.print(vec[2]);

  if (0) {
  Serial.print("Raw:");
  Serial.print(ax);
  Serial.print(',');
  Serial.print(ay);
  Serial.print(',');
  Serial.print(az);
  Serial.print(',');  

  Serial.print(gx);
  Serial.print(',');
  Serial.print(gy);
  Serial.print(',');
  Serial.print(gz);
  Serial.print(',');

  Serial.print(mx);
  Serial.print(',');
  Serial.print(my);
  Serial.print(',');
  Serial.print(mz);
  Serial.println();
  } else {
    
  }
//  Serial.print(vec[6]);
//  Serial.print(' ');
//  Serial.print(vec[7]);
//  Serial.print(' ');
//  Serial.print(vec[8]);
  
//  Serial.print(ax);
//  Serial.print(' ');
//  Serial.print(ay);
//  Serial.print(' ');
//  Serial.print(az);

//  Serial.print(mag_cali[0]);
//  Serial.print(' ');
//  Serial.print(mag_cali[1]);
//  Serial.print(' ');
//  Serial.print(mag_cali[2]);
//  Serial.println();
  
  filter.update(vec[3],vec[4],vec[5],vec[0],vec[1],vec[2], mag_cali[0], mag_cali[1], mag_cali[2]);
  // print the heading, pitch and roll
  roll = filter.getRoll();
  pitch = filter.getPitch();
  heading = filter.getYaw();
//  Serial.print("Orientation: ");
//  Serial.print(heading);
//  Serial.print(" ");
//  Serial.print(pitch);
//  Serial.print(" ");
//  Serial.println(roll);
          
//  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(50);                       // wait for a second
//  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
}

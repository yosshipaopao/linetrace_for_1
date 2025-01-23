#include <Arduino.h>
#include "sensor.h"
#include "motor.h"
#include "core.h"

const int M1_L = 3, M1_R = 4, M1_P = 5, M2_L = 8, M2_R = 9 , M2_P = 10;
int sp[4] = {A0, A1, A2, A3};
Motor m1(M1_L, M1_R, M1_P);
Motor m2(M2_L, M2_R, M2_P);
Sensor s(sp, 4, 500);

void setup()
{
  m1.setup();
  m2.setup();
  s.setup();
}
const int pow_l = 255, pow_m = 102, pow_s = -51;
int pre_dir = 0;

void loop()
{
  s.read();
  int l_power, r_power;
  cal_motor_power(&l_power, &r_power, pow_l, pow_m, pow_s, s.state(), &pre_dir);
  m1.run(l_power);
  m2.run(r_power);
  delay(10);
}

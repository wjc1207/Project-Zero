#include <Wire.h>
#define LIS3DHTR_ADDR 0x19

void setup() {
  Wire.begin();
  Serial.begin(9600);
  Wire.beginTransmission(LIS3DHTR_ADDR);
  Wire.write(0x20);  //选择控制寄存器1
  Wire.write(0x47);  //开启所有轴，设置数据率为50Hz
  Wire.endTransmission();
}

void loop() {
  Wire.beginTransmission(LIS3DHTR_ADDR);
  Wire.write(0x28 | 0x80);  //选择X轴数据寄存器，并设置自动增量寻址模式
  Wire.endTransmission(false);
  Wire.requestFrom(LIS3DHTR_ADDR, 6, true);  //从LIS3DHTR读取6个字节的数据
  int16_t x = Wire.read() | (Wire.read() << 8);
  int16_t y = Wire.read() | (Wire.read() << 8);
  int16_t z = Wire.read() | (Wire.read() << 8);
  Serial.print("X");
  Serial.println(x);
  Serial.print("Y");
  Serial.println(y);
  Serial.print("Z");
  Serial.println(z);
  if (x > (8000))
  {
      Serial.println("right!");
      delay(500);
  }
  delay(10);
}

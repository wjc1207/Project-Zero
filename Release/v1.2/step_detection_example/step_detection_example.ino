//LIS3DHTR example
//read the data from the LIS3DHTR sensor
#include <Wire.h>
#include <U8g2lib.h>



#define LIS3DHTR_ADDR 0x19
#define SHUTDOWN (23)
#define THRESHOLD (12000)
#define LOWTHRESHOLD (11000)
#define NONPEAKSTATE 1
#define PEAKSTATE 0
#define BUFFERSIZE 6



uint16_t stepNum = 0;
uint16_t i = 0;
uint8_t dataState = NONPEAKSTATE;
int16_t accelerationBuffer[BUFFERSIZE] = {0,0,0,0,0,0}; 

U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // ESP32 Thing, pure SW emulated I2C

void setup() {
  pinMode(SHUTDOWN, OUTPUT);
  digitalWrite(SHUTDOWN, HIGH);
  Serial.begin(115200);
  
  Wire.begin();
  Wire.setClock(400000); // 设置 I2C 总线时钟速度为 400kHz
  
  Wire.beginTransmission(LIS3DHTR_ADDR);
  Wire.write(0x20);  //选择控制寄存器1
  Wire.write(0x57);  //开启所有轴，设置数据率为HR / Normal / Low-power mode (100 Hz)
  Wire.endTransmission();
  Wire.beginTransmission(LIS3DHTR_ADDR);
  Wire.write(0x23);  //选择控制寄存器4
  Wire.write(0x10);  //开启所有轴，设置数据率为+4g
  Wire.endTransmission();
  
  oled.begin();
  oled.enableUTF8Print();
  oled.setFont(u8g2_font_wqy12_t_gb2312);
  oled.setFontDirection(0);
  
  showStepNum(stepNum);
}

void loop() {
  Wire.beginTransmission(LIS3DHTR_ADDR);
  Wire.write(0x29);  //选择X轴数据寄存器，并设置自动增量寻址模式
  Wire.endTransmission();
  Wire.requestFrom(LIS3DHTR_ADDR, 2, true);  //从LIS3DHTR读取6个字节的数据
  int16_t x = Wire.read() | (Wire.read() << 8);
  
  Wire.beginTransmission(LIS3DHTR_ADDR);
  Wire.write(0x2B);  //选择X轴数据寄存器，并设置自动增量寻址模式
  Wire.endTransmission();
  Wire.requestFrom(LIS3DHTR_ADDR, 2, true);  //从LIS3DHTR读取6个字节的数据
  int16_t y = Wire.read() | (Wire.read() << 8);
  
  Wire.beginTransmission(LIS3DHTR_ADDR);
  Wire.write(0x2D);  //选择X轴数据寄存器，并设置自动增量寻址模式
  Wire.endTransmission();
  int16_t z = Wire.read() | (Wire.read() << 8);

  int16_t acceleration = sqrt(x * x + y * y + z * z);
  accelerationBuffer[i] = acceleration;
  if (i < BUFFERSIZE)
    i += 1;
  else
  {
    i = 0;
    int32_t accelerationSum = 0;
    for (int n = 0; n < BUFFERSIZE; n += 1)
    {
      accelerationSum += accelerationBuffer[n];
    }
    int16_t accelerationAvg = accelerationSum / BUFFERSIZE;
    if (dataState == NONPEAKSTATE)
    {
      if (accelerationAvg > THRESHOLD)
      {
        Serial.println(accelerationAvg);
        stepNum = stepNum + 1;
        showStepNum(stepNum);
        dataState = PEAKSTATE;
        delay(50);
        Serial.println("Peak State");

      }
    }
    if (dataState == PEAKSTATE)
    {
      if (accelerationAvg <= LOWTHRESHOLD)
      {
        dataState = NONPEAKSTATE;
        Serial.println("Non-Peak State");
      }
    }
  }
  delay(10);
}



void showStepNum(uint16_t stepNum)
{
  oled.clearBuffer();
  oled.setDrawColor(1);
  oled.setCursor(5, 15);
  oled.print("step: ");
  oled.print(stepNum);
  oled.sendBuffer();
}

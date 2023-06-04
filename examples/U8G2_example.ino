 /**
  * The MIT License (MIT)
  *用于SSD1306点阵液晶屏显示
  *通过串口发送字符，将字符实时显示在液晶屏上
  */
#include <Arduino.h>
//#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
//#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`
#include <U8g2lib.h>
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // ESP32 Thing, pure SW emulated I2C
 
void setup() {
  Serial.begin(115200);
  Serial.println("led test.");
  u8g2.begin();
  u8g2.enableUTF8Print();
  u8g2.setFont(u8g2_font_wqy12_t_gb2312);
  u8g2.setFontDirection(0);
  u8g2.clearBuffer();
  u8g2.setCursor(0, 15);
  u8g2.print("Hello World!");
  u8g2.setCursor(0, 40);
  u8g2.print("你好ESP32");    // Chinese "Hello World" 
  u8g2.sendBuffer();
  
  delay(1000);
 
}
 
void loop() { 
  String comdata = "";
  while (Serial.available() > 0)  
  {
      comdata += char(Serial.read());
      delay(2);
  }
  if (comdata.length() > 0)
  {
      
      Serial.print(comdata);
      u8g2.setFontDirection(0);
      u8g2.clearBuffer();
      u8g2.setCursor(0, 15);    
      u8g2.print(comdata);
      u8g2.sendBuffer();
      comdata = "";
  }
  delay(100);
  }

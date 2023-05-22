#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(115200);
  delay(1000);
  Serial.println("Scanning I2C bus...");
  byte count = 0;
  for (byte i = 1; i < 127; i++) {
    Wire.beginTransmission(i);
    byte error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (i < 16) {
        Serial.print("0");
      }
      Serial.print(i, HEX);
      Serial.println(" !");
      count++;
      delay(10);  // 等待设备回复，避免影响其他操作
    }
  }
  if (count == 0) {
    Serial.println("No I2C devices found.");
  } else {
    Serial.println("I2C scanning completed.");
  }
}

void loop() {}

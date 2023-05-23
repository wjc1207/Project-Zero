//LTR-553ALS example
//read the data from the LTR-553ALS sensor
#include <LTR55x.h>

#include <Wire.h>
LTR55x ltr55x;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  ltr55x.begin();

}

void loop() {
  Serial.printf("als_value=%d ps_value=%d\n", ltr55x.get_als_value(), ltr55x.get_ps_value());
  delay(1000);
}

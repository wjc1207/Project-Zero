#define SW1 (16)
#define SW2 (3)
void setup() {
  // put your setup code here, to run once:
  pinMode(SW1,INPUT);
  pinMode(SW2,INPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (digitalRead(SW1) == HIGH)
  {
    Serial.println("16");
    delay(500);  
  }
  if (digitalRead(SW2) == HIGH)
  {
    Serial.println("3");  
    delay(500);  
  }
}

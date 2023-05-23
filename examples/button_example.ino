//This a simple button test example.
#define SW1 (16)
#define SW2 (3)

void setup() {
  // put your setup code here, to run once:
  //set GPIO 3,16 to input mode
  pinMode(SW1,INPUT);
  pinMode(SW2,INPUT);
  //initial the UART
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (digitalRead(SW1) == HIGH)
  //When the SW1 is pressed, the UART will output "16"
  {
    Serial.println("16");
    delay(500);  
  }
  if (digitalRead(SW2) == HIGH)
  //When the SW2 is pressed, the UART will output "3"
  {
    Serial.println("3");  
    delay(500);  
  }
}

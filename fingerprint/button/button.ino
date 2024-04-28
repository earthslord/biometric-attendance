#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>   // SoftwareSerial(rxPin, txPin, inverse_logic)
SoftwareSerial mySerial(12, 13); //you can change them if it is not working on 2 or 3

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
int button=5;
int b2 = 6;
int a;
int b;
void setup()
{
  Serial.begin(9600);
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println("\n\nFingerprint sensor enrollment");

  // set the data rate for the sensor serial port
  finger.begin(57600);
  
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }
  
  pinMode(button, INPUT_PULLUP);
  pinMode(b2, INPUT_PULLUP);

}
void loop()
 {  
 a = digitalRead(button); 
 b = digitalRead(b2);
Serial.print(" Value of button");    
Serial.println(a);
Serial.println("________________");
Serial.print(" Value of button");    
Serial.println(b);
Serial.println("________________");
delay(1000);
 }
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>   // SoftwareSerial(rxPin, txPin, inverse_logic)

// Define pins
#define enrollButtonPin 5  // Push button for enrollment
#define attendanceButtonPin 6  // Push button for attendance
SoftwareSerial mySerial(12, 13); //you can change them if it is not working on 2 or 3

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// Variables
int enroll;
int att;
int enrollMode = 0;
int attMode = 0;
uint8_t id;
uint8_t ver_id;
unsigned long attendanceTime = 1700; // Set time for attendance (5:00 PM in this example, modify as needed)
int users[10];
void setup() {
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
  
  pinMode(enrollButtonPin, INPUT_PULLUP);
  pinMode(attendanceButtonPin, INPUT_PULLUP);
}
uint8_t readnumber(void) {
  uint8_t num = 0;
  
  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

void loop() {
  // Check for enrollment button press
  enroll=digitalRead(enrollButtonPin);
  if (enroll==0) {
    enrollMode = 1;
  }
  // Check for attendance button press at specific time
  if (!digitalRead(attendanceButtonPin)) {
    attMode = 1;
  }

  // Enroll new user if in enrollment mode
  if (enrollMode) {
    Serial.println("Enrolling new user...");
    Serial.println("Please type in the ID # (from 1 to 10) you want to save this finger as...");
  
    id = readnumber();
    if (id == 0) {// ID #0 not allowed, try again!
      return;
    }
    Serial.print("Enrolling ID #");
    Serial.println(id);
  
    while (!getFingerprintEnroll());
    Serial.println("Fingerprint enrolled!");
    enrollMode = 0;
  }
  if(attMode){
    ver_id = getFingerprintIDez();
    if(ver_id!=255){
      Serial.print("VER: ");
      Serial.println(ver_id);
    }
  }
}


void takeAttendance() {
  Serial.println("Taking attendance...");
  if (finger.verifyPassword()) {
    Serial.println("Fingerprint recognized!");
    // Add logic to store attendance data (e.g., Serial print ID, store in SD card)
  } else {
    Serial.println("Fingerprint not recognized!");
  }
}
uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
}

int getFingerprintIDez()
{
uint8_t p = finger.getImage();
 
if (p != FINGERPRINT_OK)
return -1;
 
p = finger.image2Tz();
if (p != FINGERPRINT_OK)
return -1;
 
p = finger.fingerFastSearch();
if (p != FINGERPRINT_OK)
{
Serial.println("Finger Not Found");
Serial.println("Try Later");
delay(2000);
return -1;
}
// found a match!
Serial.print("Found ID #");
ver_id = finger.fingerID;
Serial.println(ver_id);
return ver_id;
}
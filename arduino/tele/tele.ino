// =======================================================================================
//                                TELE Final Project
// =======================================================================================
//                          Last Revised Date: 05/04/2019
//                  Revised By: Jack Mazanec, Kyle Miller, Prof McLaughlin
// =======================================================================================
// ---------------------------------------------------------------------------------------
//                          Libraries
// ---------------------------------------------------------------------------------------
#include <PS3BT.h>
#include <usbhub.h>
#include <Sabertooth.h>
#include <Servo.h>
#include <Adafruit_VS1053.h>
#include <SD.h>
#include <Adafruit_TLC5947.h>
#include <Adafruit_Fingerprint.h>
#include <SPI.h>
#include <WiFi101.h>
#include <stdlib.h>
#include <ArduinoJson.h>

// ---------------------------------------------------------------------------------------
//                       Debug - Verbose Flags
// ---------------------------------------------------------------------------------------
#define SHADOW_DEBUG       //uncomment this for console DEBUG output

// ---------------------------------------------------------------------------------------
//                 Setup for USB, Bluetooth Dongle, & PS3 Controller
// ---------------------------------------------------------------------------------------
USB Usb;
BTD Btd(&Usb);
PS3BT *PS3Controller=new PS3BT(&Btd);

// ---------------------------------------------------------------------------------------
//                 Setup for WiFi
// ---------------------------------------------------------------------------------------
int status = WL_IDLE_STATUS;
char storageServer[] = "https://firebasestorage.googleapis.com";
char databaseServer[] = "https://firestore.googleapis.com";
char databaseURI[] = "/v1/projects/teleui/databases/(default)/documents/users/";
char ssid[] = "ND-guest";

WiFiSSLClient client;
// ---------------------------------------------------------------------------------------
//                 Setup for Fingerprint Reader
// ---------------------------------------------------------------------------------------
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial2);
// ---------------------------------------------------------------------------------------
//    Arm Movement Setup
// ---------------------------------------------------------------------------------------
Servo arm;
int armpos = 100;

// ---------------------------------------------------------------------------------------
//    Output String for Serial Monitor Output
// ---------------------------------------------------------------------------------------
char output[300];

// ---------------------------------------------------------------------------------------
//    Deadzone range for joystick to be considered at neutral
// ---------------------------------------------------------------------------------------
byte joystickDeadZoneRange = 15;

// ---------------------------------------------------------------------------------------
//    Used for PS3 Fault Detection
// ---------------------------------------------------------------------------------------
uint32_t msgLagTime = 0;
uint32_t lastMsgTime = 0;
uint32_t currentTime = 0;
uint32_t lastLoopTime = 0;
int badPS3Data = 0;

boolean isPS3ControllerInitialized = false;
boolean mainControllerConnected = false;
boolean WaitingforReconnect = false;
boolean isFootMotorStopped = true;

// ---------------------------------------------------------------------------------------
//    Used for PS3 Controller Click Management
// ---------------------------------------------------------------------------------------
boolean extraClicks = false;
long previousMillis = millis();

// ---------------------------------------------------------------------------------------
//    Dome Movement Setup
// ---------------------------------------------------------------------------------------
const byte DOME_MAX = 127;
const int  DOME_DELAY = 10;
const int  SYREN_ADDR = 129;
Sabertooth * SYR = new Sabertooth(SYREN_ADDR, Serial1);

// ---------------------------------------------------------------------------------------
//    Leg Movement Setup
// ---------------------------------------------------------------------------------------
const byte LEG_MAX_DRIVE = 127; //For Speed Setting (Normal): set this to whatever speeds works for you. 0-stop, 127-full speed.
const byte LEG_MAX_TURN = 75; // Recommend beginner: 40 to 50, experienced: 50+, I like 75
const byte LEG_DRIVE_DEADZONE = 10; // Used to set the Sabertooth DeadZone for foot motors
const int  LEG_INC_TURN = 3;
const int  LEG_INC_DRIVE = 3;
const int  LEG_DELAY = 10;
const int  SABERTOOTH_ADDR = 128; // Serial Address for Foot Sabertooth (Dip Switches!)
Sabertooth *ST=new Sabertooth(SABERTOOTH_ADDR, Serial1); // SAME Serial Port as the Dome Motor
int leftValueX = 0;
int leftValueY = 0;
int rightValueX = 0;
bool noInput = true;
bool noInputDome = true;


// ---------------------------------------------------------------------------------------
//    Music Player Setup
// ---------------------------------------------------------------------------------------
// These are the pins used for the music maker shield
#define SHIELD_RESET  -1      // VS1053 reset pin (unused!)
#define SHIELD_CS     8      // VS1053 chip select pin (output)
#define SHIELD_DCS    6      // VS1053 Data/command select pin (output)

// These are common pins between breakout and shield
#define CARDCS 9     // Card chip select pin
#define WIFICARD 4  // WIFI Board Chip Select
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3       // VS1053 Data request, ideally an Interrupt pin

Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);

// ---------------------------------------------------------------------------------------
//    Dome Lights Setup
// ---------------------------------------------------------------------------------------
#define NUM_LIGHTS 24
#define data 45
#define clk 46
#define latch 47

Adafruit_TLC5947 domeTLC = Adafruit_TLC5947(1, clk, data, latch);

int ledMaxBright = 4095; // 4095 is MAX brightness
long int time_val;
int wait_val = 3000;
int light_vals[NUM_LIGHTS];
long lightsMillis = millis();

int yellow_lights[] = {0, 3, 6, 9, 12, 15, 18, 21};
int green_lights[] = {1, 4, 7, 10, 13, 16, 19, 22};
int red_lights[] = {2, 5, 8, 11, 14, 17, 20, 23};

void clear_all();
void turn_everything_on();
void write_lights();

void lights_yellow_on();
void lights_green_on();
void lights_red_on();

int getFingerprintIDez();
void playMessage();
int getUserId();
uint8_t getFingerprintEnroll(int);

// ---------------------------------------------------------------------------------------
//    Ad selling setup
// ---------------------------------------------------------------------------------------
int currentAd = 0;
int NUM_ADS = 1;

// =======================================================================================
//                                 Main Program
// =======================================================================================
// =======================================================================================
//                          Initialize - Setup Function
// =======================================================================================
void setup()
{

    // Debug Serial for use with USB Debugging
    Serial.begin(115200);
    while (!Serial);

    if (Usb.Init() == -1)
    {
        Serial.print(F("\r\nOSC did not start"));
        while (1); // halt
    }

    strcpy(output, "");

    Serial.print(F("\r\nBluetooth Library Started"));

    // Setup for MP3 Player
    if (! musicPlayer.begin()) { // initialise the music player
        Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
        while (1);
    }
    Serial.println(F("VS1053 found"));
    musicPlayer.setVolume(20,20);
    musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);

    // Setup legs motor
    Serial1.begin(9600);
    ST->autobaud(); // Send the autobaud command to the Sabertooth controller(s).
    ST->setTimeout(10); //DMB: How low can we go for safety reasons? multiples of 100ms
    ST->setDeadband(LEG_DRIVE_DEADZONE);

    // Setup dome motor
    SYR->autobaud();
    SYR->setTimeout(20); // 200ms increments
    SYR->stop();

    // Setup Arm Servo
    arm.attach(44);
    arm.write(armpos);

    // Setup Light Controller
    domeTLC.begin(); //Start the LED Controller
    randomSeed(analogRead(0)); // random number seed for dome automation

    /* Set all lights to 0 */
    clear_all();

    /* Init SD Card */

    if (!SD.begin(CARDCS)) {
      Serial.println("SD initialization failed!");
      while(1);
    }
    Serial.println("SD initialization done.");


    //Setup Fingerprint Sensor

    finger.begin(57600);
    delay(2000);
    if(finger.verifyPassword()) {
      Serial.println("Found fingerprint sensor!");
    } else {
      Serial.println("Did not find fingerprint sensor:(");
      while(1);
    }


    if (WiFi.status() == WL_NO_SHIELD) {
        Serial.println("WiFi shield not present");
        // don't continue:
        while (true);
    }

    //Set Lights to Red to Indicate Connecting to Wifi
    lights_red_on();

    //Setup WiFi
    while(status != WL_CONNECTED) {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(ssid);
      status = WiFi.begin(ssid);
      //wait 10 seconds for connection
      delay(10000);
    }

    //Set Lights to Green to Indicate Wifi Connected
    lights_green_on();
    lightsMillis = millis();

    // Setup for PS3 Controller
    PS3Controller->attachOnInit(onInitPS3Controller); // onInitPS3Controller is called upon a new connection


    //musicPlayer.startPlayingFile("welcome.mp3");

}

// =======================================================================================
//           Main Program Loop - This is the recurring check loop for entire sketch
// =======================================================================================
void loop()
{
   if (!readUSB()) {
     printOutput(output); return; // Fault condition; dont process controller data
   }

   respondToInput();
   adjustLegMotors(leftValueX, leftValueY);
   adjustDomeMotor(rightValueX);
   if (extraClicks)
   {
      if ((previousMillis + 500) < millis())
      {
          extraClicks = false;
      }
   }
   if ((lightsMillis + 5000) < millis()) {
    clear_all();
   }
}

// =======================================================================================
// Function Responds to the input from PS3 Controller
// =======================================================================================
void respondToInput() {
     noInput = true;
     noInputDome = true;
     /* Left Joystick: Leg Motor Control */
     if (PS3Controller->PS3Connected && ((abs(PS3Controller->getAnalogHat(LeftHatY)-128) > joystickDeadZoneRange) || (abs(PS3Controller->getAnalogHat(LeftHatX)-128) > joystickDeadZoneRange))) {
            leftValueX = PS3Controller->getAnalogHat(LeftHatX);
            leftValueY = PS3Controller->getAnalogHat(LeftHatY);
            noInput = false;
     }

     /* Right Joystick: Dome Motor Control */
     if (PS3Controller->PS3Connected && ((abs(PS3Controller->getAnalogHat(RightHatY)-128) > joystickDeadZoneRange) || (abs(PS3Controller->getAnalogHat(RightHatX)-128) > joystickDeadZoneRange))) {
            rightValueX = PS3Controller->getAnalogHat(RightHatX) - 128;
            noInputDome = false;
     }

     /* X: Extend out arm (start routine) or return arm in */
     if(PS3Controller->PS3Connected && PS3Controller->getButtonPress(X) && !extraClicks) {
          clear_all();
          lights_yellow_on();
          moveArm();
          Serial.println("Getting Fingerprint");
          int p = getFingerprintIDez();
          if (p == -1) {
            Serial.println("Finger Print Rejected");
            clear_all();
            lights_red_on();
            lightsMillis = millis();
            musicPlayer.startPlayingFile("denied.mp3");
          }
          else {
            Serial.print("Finger Accecpted with ");
            Serial.println(p);
            clear_all();
            lights_green_on();
            lightsMillis = millis();
            playMessage(p);
          }
          moveArm();
          extraClicks = true;
          previousMillis = millis();
     }

     /* O: Register new user */
     if(PS3Controller->PS3Connected && PS3Controller->getButtonPress(CIRCLE) && !extraClicks) {
          clear_all();
          lights_yellow_on();
          moveArm();
          lightsMillis = millis();
          Serial.println("Enrolling New User");
          int id = getUserId();
          Serial.println(id);
          getFingerprintEnroll(id);
          moveArm();
          extraClicks = true;
          previousMillis = millis();
     }

     /* Triangle: Sell ads */
     if(PS3Controller->PS3Connected && PS3Controller->getButtonPress(TRIANGLE) && !extraClicks) {
        sellAds();
     }

     /* Square: Delete message message */
     if(PS3Controller->PS3Connected && PS3Controller->getButtonPress(SQUARE) && !extraClicks) {
        musicPlayer.playFullFile("cleared.mp3");
        extraClicks = true;
        previousMillis = millis();
     }

     /* Need to do something or other with lights */

     /* Maybe to a dial up routine - kind of like a modem */
}

int getUserId() {
  int user = 1;
  char buf[10];
  musicPlayer.startPlayingFile("1.mp3");
  while (true) {
     if (!readUSB()) {
        printOutput(output); return; // Fault condition; dont process controller data
     }
     if (extraClicks) {
        if ((previousMillis + 500) < millis()) {
            extraClicks = false;
        }
     }
     if(PS3Controller->PS3Connected && PS3Controller->getButtonPress(UP) && !extraClicks) {
        user += 1;
        if (user > 20) {
          user = 1;
        }
        for (int i = 0; i < 10; i++) {
          buf[i] = '\0';
        }
        itoa(user, buf, 10);
        strcat(buf, ".mp3");
        Serial.println(buf);
        musicPlayer.playFullFile(buf);
        previousMillis = millis();
        extraClicks = true; 
     }
     if (PS3Controller->PS3Connected && PS3Controller->getButtonPress(DOWN) && !extraClicks) {
        user -= 1;
        if (user < 1) {
          user = 20;
        }
        for (int i = 0; i < 10; i++) {
          buf[i] = '\0';
        }
        itoa(user, buf, 10);
        strcat(buf, ".mp3");
        Serial.println(buf);
        musicPlayer.playFullFile(buf);
        previousMillis = millis();
        extraClicks = true; 
     }
     if (PS3Controller->PS3Connected && PS3Controller->getButtonPress(X) && !extraClicks) {
        previousMillis = millis();
        extraClicks = true;
        break;
     }
  }
  return user;
}

uint8_t getFingerprintEnroll(int id) {
  int p = -1;
  clear_all();
  lights_yellow_on();
  lightsMillis = millis();
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      //Serial.println(".");
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
  clear_all();
  lights_green_on();

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      clear_all();
      lights_red_on();
      lightsMillis = millis();
      musicPlayer.playFullFile("rejected.mp3");
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      clear_all();
      lights_red_on();
      lightsMillis = millis();
      musicPlayer.playFullFile("rejected.mp3");
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      clear_all();
      lights_red_on();
      lightsMillis = millis();
      musicPlayer.playFullFile("rejected.mp3");
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      clear_all();
      lights_red_on();
      lightsMillis = millis();
      musicPlayer.playFullFile("rejected.mp3");
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      clear_all();
      lights_red_on();
      lightsMillis = millis();
      musicPlayer.playFullFile("rejected.mp3");
      Serial.println("Unknown error");
      return p;
  }

  musicPlayer.playFullFile("accepted.mp3");
  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  clear_all();
  lights_yellow_on();
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
      //Serial.print(".");
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

  clear_all();
  lights_green_on();
  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      clear_all();
      lights_red_on();
      lightsMillis = millis();
      musicPlayer.playFullFile("rejected.mp3");
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      clear_all();
      lights_red_on();
      lightsMillis = millis();
      musicPlayer.playFullFile("rejected.mp3");
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      clear_all();
      lights_red_on();
      lightsMillis = millis();
      musicPlayer.playFullFile("rejected.mp3");
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      clear_all();
      lights_red_on();
      lightsMillis = millis();
      musicPlayer.playFullFile("rejected.mp3");
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      clear_all();
      lights_red_on();
      lightsMillis = millis();
      musicPlayer.playFullFile("rejected.mp3");
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    clear_all();
    lights_red_on();
    lightsMillis = millis();
    musicPlayer.playFullFile("rejected.mp3");
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    clear_all();
    lights_red_on();
    lightsMillis = millis();
    musicPlayer.playFullFile("rejected.mp3");
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    clear_all();
    lights_red_on();
    lightsMillis = millis();
    musicPlayer.playFullFile("rejected.mp3");
    Serial.println("Unknown error");
    return p;
  }   
  
  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    musicPlayer.playFullFile("accepted.mp3");
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    clear_all();
    lights_red_on();
    lightsMillis = millis();
    musicPlayer.playFullFile("rejected.mp3");
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    clear_all();
    lights_red_on();
    lightsMillis = millis();
    musicPlayer.playFullFile("rejected.mp3");
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    clear_all();
    lights_red_on();
    lightsMillis = millis();
    musicPlayer.playFullFile("rejected.mp3");
    Serial.println("Error writing to flash");
    return p;
  } else {
    clear_all();
    lights_red_on();
    lightsMillis = millis();
    musicPlayer.playFullFile("rejected.mp3");
    Serial.println("Unknown error");
    return p;
  }   
}

void playMessage(int fingerId) {
  const size_t capacity = 2*JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + 400;
  StaticJsonDocument<capacity> doc;
  char buf[4];
  itoa(fingerId, buf, 10);
  char uri[100];
  strcpy(uri, databaseURI);
  strcat(uri, buf);
  Serial.println(uri);
  if (client.connect(databaseServer, 443)){
    client.print("GET ");
    client.print(uri);
    client.println(" HTTP/1.1");
    client.println("Host: firestore.googleapis.com");
    client.println("Connection: close");
    client.println();
  }
  else {
    Serial.println("Getting Database info failed");
    clear_all();
    lights_red_on();
    lightsMillis = millis();
    return;
  }
  if (SD.exists("t.txt")) {
    SD.remove("t.txt");
  }
  bool endOfHeaders = false;
  File myFile = SD.open("t.txt", FILE_WRITE);
  while(client.connected()) {
    if(myFile) {
      while(client.available()){
        byte c = client.read();
        if (c == '\r' && !endOfHeaders) {
          c = client.read();
          if (c == '\n') {
            c = client.read();
            if (c == '\r') {
              c = client.read();
              if (c == '\n') {
                endOfHeaders = true;
                continue;
              }
            }
          }
        }
        if (endOfHeaders) {
          myFile.write(c);
        }
      }
    }
  }
  myFile.close();
  myFile = SD.open("t.txt");
  DeserializationError err = deserializeJson(doc,  myFile);
  if (err) {
    Serial.print("Deserializing JSON Failed: ");
    Serial.println(err.c_str());
    clear_all();
    lights_red_on();
    lightsMillis = millis();
    return;
  }
  client.stop();
  myFile.close();
  const char * msg = doc["fields"]["msgURL"]["stringValue"];
  if (msg == nullptr) {
    musicPlayer.startPlayingFile("nomsg.mp3");
    return;
  }
  if (client.connect(storageServer, 443)) {
    client.print("GET ");
    client.print(msg);
    client.println(" HTTP/1.1");
    client.println("Host: firebasestorage.googleapis.com");
    client.println("Connection: close");
    client.println();
  }
  else {
    clear_all();
    lights_red_on();
    lightsMillis = millis();
    return;
  }
  if (SD.exists("msg.mp3")) {
    SD.remove("msg.mp3");
  }
  musicPlayer.startPlayingFile("dialup.mp3");
  myFile = SD.open("msg.mp3", FILE_WRITE);
  while(client.connected()) {
    if(myFile) {
      while(client.available()) {
        byte c = client.read();
        myFile.write(c);
      }
    }
    else {
      client.stop();
      return;
    }
  }

  if (myFile) {
    Serial.println("myFile was opened");
    musicPlayer.stopPlaying();
    myFile.close();
    delay(3000);
    musicPlayer.startPlayingFile("msg.mp3");
  }

  client.stop();
}

/* Get Fingerprint Match */
int getFingerprintIDez() {
  uint8_t p;
  do {
    p = finger.getImage();
  } while(p == FINGERPRINT_NOFINGER);
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
  return finger.fingerID;
}

/* Arm Control */
void moveArm() {
  int targetpos;
  if (armpos == 100) {
    targetpos = 178;
  }
  else if (armpos == 178) {
    targetpos = 100;
  }
  else {
    return;
  }
  long armdelay = millis();
  while (armpos != targetpos) {
    if ((armdelay + 15) < millis()) {
      armpos = (targetpos > armpos) ? armpos + 1 :  armpos - 1;
      arm.write(armpos);
      armdelay = millis();
    }
  }
}

/* Leg Motor Control */
void adjustLegMotors(float joystickValueX, float joystickValueY) {

  static long motorTimer = millis();
  static float turnVal = 0, driveVal = 0;

  int requestedTurnVal  = map(joystickValueX, 0, 255, -1*LEG_MAX_TURN, LEG_MAX_TURN);
  int requestedDriveVal = -1*map(joystickValueY, 0, 255, -1*LEG_MAX_DRIVE, LEG_MAX_DRIVE);

  if (noInput) {
    requestedTurnVal = 0;
    requestedDriveVal = 0;
  }

  if (motorTimer + LEG_DELAY < millis()) {
   if (turnVal < requestedTurnVal)
     turnVal = (turnVal + LEG_INC_TURN < requestedTurnVal) ? turnVal + LEG_INC_TURN : requestedTurnVal;
   else
     turnVal = (turnVal - LEG_INC_TURN > requestedTurnVal) ? turnVal - LEG_INC_TURN : requestedTurnVal;

   if (driveVal < requestedDriveVal)
     driveVal = (driveVal + LEG_INC_DRIVE < requestedDriveVal) ? driveVal + LEG_INC_DRIVE : requestedDriveVal;
   else
     driveVal = (driveVal - LEG_INC_DRIVE > requestedDriveVal) ? driveVal - LEG_INC_DRIVE : requestedDriveVal;

   motorTimer = millis();
  }

  ST->turn(turnVal);
  ST->drive(driveVal);
}

/* Dome Motor Control */
void adjustDomeMotor(float joystickValueX) {
  static long motorTimer = millis();
  static float domeRotationSpeed = 0;

  if (motorTimer + DOME_DELAY < millis()) {
    domeRotationSpeed = (joystickValueX/128)*DOME_MAX;
    if (noInputDome) {
      domeRotationSpeed = 0;
    }
    motorTimer = millis();
  }

  SYR->motor(domeRotationSpeed);
}

void clear_all() {
    for (int i = 0; i < NUM_LIGHTS; i++) {
      light_vals[i] = 0;
    }
    write_lights();
}

void lights_green_on() {
  clear_all();
  for (int i = 0; i < NUM_LIGHTS/3; i++) {
    light_vals[green_lights[i]] = ledMaxBright;
  }
  write_lights();
}

void lights_red_on() {
  clear_all();
  for (int i = 0; i < NUM_LIGHTS/3; i++) {
    light_vals[red_lights[i]] = ledMaxBright;
  }
  write_lights();
}

void lights_yellow_on() {
  clear_all();
  for (int i = 0; i < NUM_LIGHTS/3; i++) {
    light_vals[yellow_lights[i]] = ledMaxBright;
  }
  write_lights();
}

void turn_everything_on() {
    for (int i = 0; i < NUM_LIGHTS; i++) {
      light_vals[i] = ledMaxBright;
    }
    write_lights();
}

void write_lights() {
  for (int i = 0; i < NUM_LIGHTS; i++) {
    domeTLC.setPWM(i, light_vals[i]);
  }
  domeTLC.write();
}


void sellAds() {
  /* Play next ad */
  char num_string[3];

  if (currentAd > NUM_ADS)
    currentAd = 0;
  itoa(currentAd++, num_string);

  char ad[12];
  strcat(ad, "ad");
  strcat(ad, num_string);
  strcat(ad, ".mp3");
  musicPlayer.playFullFile(ad);
  musicPlayer.playFullFile("purchase.mp3");

  /* Offer user the chance to buy the ad */
  while (1) {
    if (!readUSB()) {
      printOutput(output); return; // Fault condition; dont process controller data
    }
    if(PS3Controller->PS3Connected && PS3Controller->getButtonPress(X)) {
      moveArm();
retry_purchase:
      if (getFingerprintIDez() != -1) {
        musicPlayer.startPlayingFile("accepted.mp3");
        moveArm();
      } else {
        musicPlayer.startPlayingFile("rejected.mp3");
        goto: retry_purchase;
      }
      break;
    } else if(PS3Controller->PS3Connected && PS3Controller->getButtonPress(TRIANGLE)) {
      break;
    }
  }
  extraClicks = true;
  previousMillis = millis();
}


// =======================================================================================
//           PPS3 Controller Device Mgt Functions
// =======================================================================================
// =======================================================================================
//           Initialize the PS3 Controller Trying to Connect
// =======================================================================================
void onInitPS3Controller()
{
    Serial.println("Got to the Connect Function");
    PS3Controller->setLedOn(LED1);
    isPS3ControllerInitialized = true;
    badPS3Data = 0;

    mainControllerConnected = true;
    WaitingforReconnect = true;

    #ifdef SHADOW_DEBUG
       strcat(output, "\r\nWe have the controller connected.\r\n");
    #endif
}

// =======================================================================================
//           Determine if we are having connection problems with the PS3 Controller
// =======================================================================================
boolean criticalFaultDetect()
{
    if (PS3Controller->PS3Connected)
    {

        currentTime = millis();
        lastMsgTime = PS3Controller->getLastMessageTime();
        msgLagTime = currentTime - lastMsgTime;

        if (WaitingforReconnect)
        {

            if (msgLagTime < 200)
            {

                WaitingforReconnect = false;

            }

            lastMsgTime = currentTime;

        }

        if ( currentTime >= lastMsgTime)
        {
              msgLagTime = currentTime - lastMsgTime;

        } else
        {

             msgLagTime = 0;
        }

        if (msgLagTime > 300 && !isFootMotorStopped)
        {
            #ifdef SHADOW_DEBUG
              strcat(output, "It has been 300ms since we heard from the PS3 Controller\r\n");
              strcat(output, "Shut down motors and watching for a new PS3 message\r\n");
            #endif

//          You would stop all motors here
            isFootMotorStopped = true;
        }

        if ( msgLagTime > 10000 )
        {
            #ifdef SHADOW_DEBUG
              strcat(output, "It has been 10s since we heard from Controller\r\n");
              strcat(output, "\r\nDisconnecting the controller.\r\n");
            #endif

//          You would stop all motors here
            isFootMotorStopped = true;

            PS3Controller->disconnect();
            WaitingforReconnect = true;
            return true;
        }

        //Check PS3 Signal Data
        if(!PS3Controller->getStatus(Plugged) && !PS3Controller->getStatus(Unplugged))
        {
            //We don't have good data from the controller.
            //Wait 15ms - try again
            delay(15);
            Usb.Task();
            lastMsgTime = PS3Controller->getLastMessageTime();

            if(!PS3Controller->getStatus(Plugged) && !PS3Controller->getStatus(Unplugged))
            {
                badPS3Data++;
                #ifdef SHADOW_DEBUG
                    strcat(output, "\r\n**Invalid data from PS3 Controller. - Resetting Data**\r\n");
                #endif
                return true;
            }
        }
        else if (badPS3Data > 0)
        {

            badPS3Data = 0;
        }

        if ( badPS3Data > 10 )
        {
            #ifdef SHADOW_DEBUG
                strcat(output, "Too much bad data coming from the PS3 Controller\r\n");
                strcat(output, "Disconnecting the controller and stop motors.\r\n");
            #endif

//          You would stop all motors here
            isFootMotorStopped = true;

            PS3Controller->disconnect();
            WaitingforReconnect = true;
            return true;
        }
    }
    else if (!isFootMotorStopped)
    {
        #ifdef SHADOW_DEBUG
            strcat(output, "No PS3 controller was found\r\n");
            strcat(output, "Shuting down motors and watching for a new PS3 message\r\n");
        #endif

//      You would stop all motors here
        isFootMotorStopped = true;

        WaitingforReconnect = true;
        return true;
    }

    return false;
}

// =======================================================================================
//           USB Read Function - Supports Main Program Loop
// =======================================================================================
boolean readUSB()
{

     Usb.Task();

    //The more devices we have connected to the USB or BlueTooth, the more often Usb.Task need to be called to eliminate latency.
    if (PS3Controller->PS3Connected)
    {
        if (criticalFaultDetect())
        {
            //We have a fault condition that we want to ensure that we do NOT process any controller data!!!
            printOutput(output);
            return false;
        }

    } else if (!isFootMotorStopped)
    {
        #ifdef SHADOW_DEBUG
            strcat(output, "No controller was found\r\n");
            strcat(output, "Shuting down motors, and watching for a new PS3 foot message\r\n");
        #endif

//      You would stop all motors here
        isFootMotorStopped = true;

        WaitingforReconnect = true;
    }

    return true;
}

// =======================================================================================
//          Print Output Function
// =======================================================================================

void printOutput(const char *value)
{
    if ((strcmp(value, "") != 0))
    {
        if (Serial) Serial.println(value);
        strcpy(output, ""); // Reset output string
    }
}

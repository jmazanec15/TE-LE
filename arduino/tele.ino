// =======================================================================================
//                                TELE Final Project
// =======================================================================================
//                          Last Revised Date: 05/01/2019
//                  Revised By: Jack Mazanec, Kyle Miller, Prof McLaughlin
// =======================================================================================
// ---------------------------------------------------------------------------------------
//                          Libraries
// ---------------------------------------------------------------------------------------
#include <PS3BT.h>
#include <usbhub.h>
#include <Sabertooth.h>

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

    // Setup for PS3 Controller
    PS3Controller->attachOnInit(onInitPS3Controller); // onInitPS3Controller is called upon a new connection

    // Setup legs motor
    ST->autobaud(); // Send the autobaud command to the Sabertooth controller(s).
    ST->setTimeout(10); //DMB: How low can we go for safety reasons? multiples of 100ms
    ST->setDeadband(LEG_DRIVE_DEADZONE);

    // Setup dome motor
    Serial1.begin(9600);
    SYR->autobaud();
    SYR->setTimeout(20); // 200ms increments
    SYR->stop();
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
}

// =======================================================================================
// Function Responds to the input from PS3 Controller
// =======================================================================================
void respondToInput() {
     /* Left Joystick: Leg Motor Control */
     if (PS3Controller->PS3Connected && ((abs(PS3Controller->getAnalogHat(LeftHatY)-128) > joystickDeadZoneRange) || (abs(PS3Controller->getAnalogHat(LeftHatX)-128) > joystickDeadZoneRange))) {
            adjustLegMotors(PS3Controller->getAnalogHat(LeftHatX), PS3Controller->getAnalogHat(LeftHatY));
     }

     /* Right Joystick: Dome Motor Control */
     if (PS3Controller->PS3Connected && ((abs(PS3Controller->getAnalogHat(RightHatY)-128) > joystickDeadZoneRange) || (abs(PS3Controller->getAnalogHat(RightHatX)-128) > joystickDeadZoneRange))) {
            adjustDomeMotor(PS3Controller->getAnalogHat(RightHatX) - 128);
     }

     /* X: Extend out arm (start routine) or return arm in */

     /* O: Register new user */

     /* Triangle: Next message */
     
     /* Square: Delete message message */

     /* Need to do something or other with lights */

     /* Maybe to a dial up routine - kind of like a modem */
}

/* Leg Motor Control */
void adjustLegMotors(int joystickValueX, int joystickValueY) {
  int requestedTurnVal  = map(joystickValueX, 0, 255, -1*LEG_MAX_TURN, LEG_MAX_TURN);
  int requestedDriveVal = -1*map(joystickValueY, 0, 255, -1*LEG_MAX_DRIVE, LEG_MAX_DRIVE);

  static long motorTimer = millis();
  static int turnVal = 0, driveVal = 0;

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
void adjustDomeMotor(int joystickValueX) {
  static long motorTimer = millis();
  static int domeRotationSpeed = 0;

  if (motorTimer + DOME_DELAY < millis()) {
    domeRotationSpeed = (joystickValueX/128)*DOME_MAX;
    motorTimer = millis();
  }

  SYR->motor(domeRotationSpeed);
}









// =======================================================================================
//           PPS3 Controller Device Mgt Functions
// =======================================================================================
// =======================================================================================
//           Initialize the PS3 Controller Trying to Connect
// =======================================================================================
void onInitPS3Controller()
{
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

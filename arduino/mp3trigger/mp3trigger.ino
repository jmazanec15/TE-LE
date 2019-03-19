#include <usbhub.h>

#include <MP3Trigger.h>
#include <Servo.h>
#include <PS3BT.h>

#define SHADOW_DEBUG       //uncomment this for console DEBUG output


/* Debugging purposes */
char output[300];

/* Global definitions */
// Servo
//Servo myservo;
//boolean forward = true;
//int pos = 0;
boolean isFootMotorStopped = true;

// MP3
MP3Trigger MP3Trigger; // For controlling MP#

USB Usb;
BTD Btd(&Usb);

// PS3 Controller
PS3BT *PS3Controller=new PS3BT(&Btd);
byte joystickDeadZoneRange = 15;
int badPS3Data = 0;
boolean isPS3ControllerInitialized = false;
boolean mainControllerConnected = false;
boolean WaitingforReconnect = false;
boolean extraClicks = false;

// Time keeping
uint32_t msgLagTime = 0;
uint32_t lastMsgTime = 0;
uint32_t currentTime = 0;
uint32_t lastLoopTime = 0;
long previousMillis = millis();





void setup() {
  // Setup USB
  Serial.begin(115200);
  while (!Serial);

  if (Usb.Init() == -1)
  {
      Serial.print(F("\r\nOSC did not start"));
      while (1); //halt
  }

  // Init for debugging
  strcpy(output, "");
  Serial.print(F("\r\nBluetooth Library Started"));

   // Setup for MP3
  MP3Trigger.setup(&Serial2);
  Serial2.begin(MP3Trigger::serialRate());
  
  // Setup for PS3 Controller
  PS3Controller->attachOnInit(onInitPS3Controller); // onInitPS3Controller is called upon a new connection

  // Setup servo to pin 9
  //myservo.attach(9);

}

void loop() {

    if ( !readUSB() )
    {
      //We have a fault condition that we want to ensure that we do NOT process any controller data!!!
      printOutput(output);
      return;
    }
    
   checkController();
   if (extraClicks)
   {
      if ((previousMillis + 50) < millis())
      {
          extraClicks = false;
      }
   }
  
   printOutput(output);
   MP3Trigger.update();
}

void checkController()
{
     // Move Servo on Cross
     if (PS3Controller->PS3Connected && PS3Controller->getButtonPress(CROSS) && !extraClicks)
     {
            #ifdef SHADOW_DEBUG
                strcat(output, "Button: CROSS Selected.\r\n");
            #endif       
            
            /*
            // On X click, rotate servo
            if (pos == 180)
              forward = false;
            else if (pos == 0)
              forward = true;

            if (forward)
              pos += 5;
            else
              pos -= 5;
            
            myservo.write(pos);
            //delay(10);
            */
            previousMillis = millis();
            extraClicks = true;
              
     }

     // Make camera sound on Circle
     if (PS3Controller->PS3Connected && PS3Controller->getButtonPress(CIRCLE) && !extraClicks)
     {
            #ifdef SHADOW_DEBUG
                strcat(output, "Button: CIRCLE Selected.\r\n");
            #endif       
            // On O click, make sound
            MP3Trigger.trigger(1);
            
            previousMillis = millis();
            extraClicks = true;
     }
     // Make light saber sound on Triangle
     if (PS3Controller->PS3Connected && PS3Controller->getButtonPress(TRIANGLE) && !extraClicks)
     {
            #ifdef SHADOW_DEBUG
                strcat(output, "Button: TRIANGLE Selected.\r\n");
            #endif       
            // On TRIANGLE click, make sound
            MP3Trigger.trigger(3);
            
            previousMillis = millis();
            extraClicks = true;
              
     }
     
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

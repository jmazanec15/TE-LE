#include <Adafruit_TLC5947.h>

//***************************************************
// For LED control, define the data ports
//***************************************************
#define data 4
#define clk 5
#define latch 6
//***************************************************
// Initialize the LED Controller
//***************************************************
Adafruit_TLC5947 domeTLC = Adafruit_TLC5947(1, clk, data, latch);
//***************************************************
// Set the Max brightness value of the LEDs
//***************************************************
int ledMaxBright = 4000; // 4095 is MAX brightness

long int time_val;
int switch_const = 1000;
int light_val;
void setup() {
  domeTLC.begin(); //Start the LED Controller
  randomSeed(analogRead(0)); // random number seed for dome automation
  time_val = millis();
  light_val = ledMaxBright;
}



void loop() {
  if (millis() - time_val > switch_const) {
    light_val = (light_val == 0) ? ledMaxBright : 0;
    domeTLC.setPWM(10, light_val);
    domeTLC.write();
    time_val = millis();
  }
}

#include <Adafruit_TLC5947.h>

//***************************************************
// For LED control, define the data ports
//***************************************************
// yellow: 45 --> data
// orange: 46 --> clk
// brown: 47 --> latch
#define data 45
#define clk 46
#define latch 47
//***************************************************
// Initialize the LED Controller
//***************************************************
Adafruit_TLC5947 domeTLC = Adafruit_TLC5947(1, clk, data, latch);
//***************************************************
// Set the Max brightness value of the LEDs
//***************************************************
#define NUM_LIGHTS 24

int ledMaxBright = 4095; // 4095 is MAX brightness
long int time_val;
int wait_val = 3000;
int light_vals[NUM_LIGHTS];




int yellow_lights[] = {0, 3, 6, 9, 12, 15, 18, 21};
int green_lights[] = {1, 4, 7, 10, 13, 16, 19, 22};
int red_lights[] = {2, 5, 8, 11, 14, 17, 20, 23};


void clear_all();
void turn_everything_on();
void write_lights();

void lights_yellow_on();
void lights_green_on();
void lights_red_on();

void setup() { 
  domeTLC.begin(); //Start the LED Controller
  randomSeed(analogRead(0)); // random number seed for dome automation

  /* Set all lights to 0 */
  clear_all();

  /* Start timer and wait 3 seconds */
  time_val = millis();
  
}


int counter = 0;
void loop() {
    if (millis() - time_val > wait_val) {
      if (counter == 0) {
        lights_green_on();
      } else if (counter == 1) {
        lights_red_on();
      } else if(counter == 2) {
        lights_yellow_on();  
        counter = -1;
      } 

      counter++;
      time_val = millis();
    }
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

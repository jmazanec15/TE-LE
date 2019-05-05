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
#define NUM_LIGHTS 20

int ledMaxBright = 4000; // 4095 is MAX brightness
long int time_val;
int wait_val = 2000;

int light_vals[NUM_LIGHTS];
int letter = 0;

const byte numChars = 32;
char receivedChars[101] = {0};

boolean newData = false;

void displayString();
void display_letter();
void clear_all();
void display_showing();
void turn_everything_on();
void write_lights();
void recvWithEndMarker();




void setup() {
  /* Vars */
  int i;

  Serial.begin(9600);
  Serial.println("Please enter your message: ");
  
  domeTLC.begin(); //Start the LED Controller
  randomSeed(analogRead(0)); // random number seed for dome automation

  /* Set all lights to 0 */
  clear_all();

  /* Start timer and wait 3 seconds */
  time_val = millis();
  
}



void loop() {
    clear_all();
    recvWithEndMarker();
    if (newData == true) {
      displayString();
    }
 }

void displayString() {
  int i = 0;
  while (receivedChars[i] != '\0') {
    if (millis() - time_val > wait_val) {
      if (receivedChars[i] == ' ') {
        letter = 26;
      }
      else {
        char l = toupper(receivedChars[i]);
        letter = l - 'A';
      }
      display_letter();
      time_val = millis();
      i++;
    }
  }
  newData = false;
}
void display_letter() {
  clear_all();
  if (letter == 0) {
    light_vals[1] = ledMaxBright;
    light_vals[2] = ledMaxBright;
    light_vals[4] = ledMaxBright;
    light_vals[7] = ledMaxBright;
    light_vals[8] = ledMaxBright;
    light_vals[9] = ledMaxBright;
    light_vals[10] = ledMaxBright;
    light_vals[11] = ledMaxBright;
    light_vals[12] = ledMaxBright;
    light_vals[15] = ledMaxBright;
    light_vals[16] = ledMaxBright;
    light_vals[19] = ledMaxBright;
  }
  else if (letter == 1) {
    light_vals[0] = ledMaxBright;
    light_vals[1] = ledMaxBright;
    light_vals[2] = ledMaxBright;
    light_vals[4] = ledMaxBright;
    light_vals[6] = ledMaxBright;
    light_vals[8] = ledMaxBright;
    light_vals[9] = ledMaxBright;
    light_vals[10] = ledMaxBright;
    light_vals[12] = ledMaxBright;
    light_vals[14] = ledMaxBright;
    light_vals[16] = ledMaxBright;
    light_vals[17] = ledMaxBright;
    light_vals[18] = ledMaxBright;
  }
  else if (letter == 2) {
    // C
    light_vals[0] = ledMaxBright;
    light_vals[1] = ledMaxBright;
    light_vals[2] = ledMaxBright;
    light_vals[3] = ledMaxBright;
    light_vals[4] = ledMaxBright;
    light_vals[8] = ledMaxBright;
    light_vals[12] = ledMaxBright;
    light_vals[16] = ledMaxBright;
    light_vals[17] = ledMaxBright;
    light_vals[18] = ledMaxBright;
    light_vals[19] = ledMaxBright;
  }
  else if (letter == 3) {
    // D
    light_vals[0] = ledMaxBright;
    light_vals[1] = ledMaxBright;
    light_vals[2] = ledMaxBright;
    light_vals[4] = ledMaxBright;
    light_vals[7] = ledMaxBright;
    light_vals[8] = ledMaxBright;
    light_vals[11] = ledMaxBright;
    light_vals[12] = ledMaxBright;
    light_vals[15] = ledMaxBright;
    light_vals[16] = ledMaxBright;
    light_vals[17] = ledMaxBright;
    light_vals[18] = ledMaxBright;
  }
  else if (letter == 4) {
    // E
    light_vals[0] = ledMaxBright;
    light_vals[1] = ledMaxBright;
    light_vals[2] = ledMaxBright;
    light_vals[3] = ledMaxBright;
    light_vals[4] = ledMaxBright;
    light_vals[8] = ledMaxBright;
    light_vals[9] = ledMaxBright;
    light_vals[10] = ledMaxBright;
    light_vals[12] = ledMaxBright;
    light_vals[16] = ledMaxBright;
    light_vals[17] = ledMaxBright;
    light_vals[18] = ledMaxBright;
    light_vals[19] = ledMaxBright;
  }
  else if (letter == 5) {
    // F
    light_vals[0] = ledMaxBright;
    light_vals[1] = ledMaxBright;
    light_vals[2] = ledMaxBright;
    light_vals[3] = ledMaxBright;
    light_vals[4] = ledMaxBright;
    light_vals[8] = ledMaxBright;
    light_vals[9] = ledMaxBright;
    light_vals[10] = ledMaxBright;
    light_vals[12] = ledMaxBright;
    light_vals[16] = ledMaxBright;
  }
  else if (letter == 6) {
    // G
    light_vals[0] = ledMaxBright;
    light_vals[1] = ledMaxBright;
    light_vals[2] = ledMaxBright;
    light_vals[3] = ledMaxBright;
    light_vals[4] = ledMaxBright;
    light_vals[8] = ledMaxBright;
    light_vals[10] = ledMaxBright;
    light_vals[11] = ledMaxBright;
    light_vals[12] = ledMaxBright;
    light_vals[15] = ledMaxBright;
    light_vals[16] = ledMaxBright;
    light_vals[17] = ledMaxBright;
    light_vals[18] = ledMaxBright;
    light_vals[19] = ledMaxBright;
  }
  else if (letter == 7) {
    // H
    light_vals[0] = ledMaxBright;
    light_vals[3] = ledMaxBright;
    light_vals[4] = ledMaxBright;
    light_vals[7] = ledMaxBright;
    light_vals[8] = ledMaxBright;
    light_vals[9] = ledMaxBright;
    light_vals[10] = ledMaxBright;
    light_vals[11] = ledMaxBright;
    light_vals[12] = ledMaxBright;
    light_vals[15] = ledMaxBright;
    light_vals[16] = ledMaxBright;
    light_vals[19] = ledMaxBright;
  }
  else if (letter == 8) {
    // I
    light_vals[0] = ledMaxBright;
    light_vals[1] = ledMaxBright;
    light_vals[2] = ledMaxBright;
    light_vals[3] = ledMaxBright;
    light_vals[5] = ledMaxBright;
    light_vals[6] = ledMaxBright;
    light_vals[9] = ledMaxBright;
    light_vals[10] = ledMaxBright;
    light_vals[13] = ledMaxBright;
    light_vals[14] = ledMaxBright;
    light_vals[16] = ledMaxBright;
    light_vals[17] = ledMaxBright;
    light_vals[18] = ledMaxBright;
    light_vals[19] = ledMaxBright;
  }
  else if (letter == 9) {
    // J
    light_vals[2] = ledMaxBright;
    light_vals[6] = ledMaxBright;
    light_vals[10] = ledMaxBright;
    light_vals[12] = ledMaxBright;
    light_vals[14] = ledMaxBright;
    light_vals[16] = ledMaxBright;
    light_vals[17] = ledMaxBright;
    light_vals[18] = ledMaxBright;
  }
  else if (letter == 10) {
    // K
    light_vals[0] = ledMaxBright;
    light_vals[3] = ledMaxBright;
    light_vals[4] = ledMaxBright;
    light_vals[6] = ledMaxBright;
    light_vals[8] = ledMaxBright;
    light_vals[9] = ledMaxBright;
    light_vals[12] = ledMaxBright;
    light_vals[14] = ledMaxBright;
    light_vals[16] = ledMaxBright;
    light_vals[19] = ledMaxBright;
  }
  else if (letter == 11) {
    // L
    light_vals[0] = ledMaxBright;
    light_vals[4] = ledMaxBright;
    light_vals[8] = ledMaxBright;
    light_vals[12] = ledMaxBright;
    light_vals[16] = ledMaxBright;
    light_vals[17] = ledMaxBright;
    light_vals[18] = ledMaxBright;
    light_vals[19] = ledMaxBright;
  }
  else if (letter == 12) {
    light_vals[0] = ledMaxBright;
    light_vals[3] = ledMaxBright;
    light_vals[4] = ledMaxBright;
    light_vals[5] = ledMaxBright;
    light_vals[6] = ledMaxBright;
    light_vals[7] = ledMaxBright;
    light_vals[8] = ledMaxBright;
    light_vals[9] = ledMaxBright;
    light_vals[10] = ledMaxBright;
    light_vals[11] = ledMaxBright;
    light_vals[12] = ledMaxBright;
    light_vals[15] = ledMaxBright;
    light_vals[16] = ledMaxBright;
    light_vals[19] = ledMaxBright;
  }
  else if (letter == 13) {
    light_vals[0] = ledMaxBright;
    light_vals[4] = ledMaxBright;
    light_vals[8] = ledMaxBright;
    light_vals[12] = ledMaxBright;
    light_vals[5] = ledMaxBright;
    light_vals[10] = ledMaxBright;
    light_vals[15] = ledMaxBright;
    light_vals[3] = ledMaxBright;
    light_vals[7] = ledMaxBright;
    light_vals[11] = ledMaxBright;
  }
  else if (letter == 14) {
    light_vals[1] = ledMaxBright;
    light_vals[2] = ledMaxBright;
    light_vals[4] = ledMaxBright;
    light_vals[8] = ledMaxBright;
    light_vals[12] = ledMaxBright;
    light_vals[7] = ledMaxBright;
    light_vals[11] = ledMaxBright;
    light_vals[15] = ledMaxBright;
    light_vals[17] = ledMaxBright;
    light_vals[18] = ledMaxBright;
  }
  else if (letter == 15) {
    // P
    light_vals[0] = ledMaxBright;
    light_vals[1] = ledMaxBright;
    light_vals[2] = ledMaxBright;
    light_vals[3] = ledMaxBright;
    light_vals[4] = ledMaxBright;
    light_vals[7] = ledMaxBright;
    light_vals[8] = ledMaxBright;
    light_vals[9] = ledMaxBright;
    light_vals[10] = ledMaxBright;
    light_vals[11] = ledMaxBright;
    light_vals[12] = ledMaxBright;
    light_vals[16] = ledMaxBright;
  }
  else if (letter == 16) {
    // Q
    light_vals[9] = ledMaxBright;
    light_vals[10] = ledMaxBright;
    light_vals[13] = ledMaxBright;
    light_vals[14] = ledMaxBright;
    light_vals[19] = ledMaxBright;
  }
  else if (letter == 17) {
    // R  
    light_vals[0] = ledMaxBright;
    light_vals[1] = ledMaxBright;
    light_vals[2] = ledMaxBright;
    light_vals[3] = ledMaxBright;
    light_vals[4] = ledMaxBright;
    light_vals[7] = ledMaxBright;
    light_vals[8] = ledMaxBright;
    light_vals[9] = ledMaxBright;
    light_vals[10] = ledMaxBright;
    light_vals[11] = ledMaxBright;
    light_vals[12] = ledMaxBright;
    light_vals[14] = ledMaxBright;
    light_vals[16] = ledMaxBright;
    light_vals[19] = ledMaxBright;
  }
  else if (letter == 18) {
    // S
    light_vals[0] = ledMaxBright;
    light_vals[1] = ledMaxBright;
    light_vals[2] = ledMaxBright;
    light_vals[3] = ledMaxBright;
    light_vals[4] = ledMaxBright;
    light_vals[8] = ledMaxBright;
    light_vals[9] = ledMaxBright;
    light_vals[10] = ledMaxBright;
    light_vals[11] = ledMaxBright;
    light_vals[15] = ledMaxBright;
    light_vals[16] = ledMaxBright;
    light_vals[17] = ledMaxBright;
    light_vals[18] = ledMaxBright;
    light_vals[19] = ledMaxBright;
  
    
  }
  else if (letter == 19) {
    // T
    light_vals[0] = ledMaxBright;
    light_vals[1] = ledMaxBright;
    light_vals[2] = ledMaxBright;
    light_vals[3] = ledMaxBright;
    light_vals[5] = ledMaxBright;
    light_vals[6] = ledMaxBright;
    light_vals[9] = ledMaxBright;
    light_vals[10] = ledMaxBright;
    light_vals[13] = ledMaxBright;
    light_vals[14] = ledMaxBright;
    light_vals[17] = ledMaxBright;
    light_vals[18] = ledMaxBright;
    
  }
  else if (letter == 20) {
    // U
    light_vals[0] = ledMaxBright;
    light_vals[3] = ledMaxBright;
    light_vals[4] = ledMaxBright;
    light_vals[7] = ledMaxBright;
    light_vals[8] = ledMaxBright;
    light_vals[11] = ledMaxBright;
    light_vals[12] = ledMaxBright;
    light_vals[15] = ledMaxBright;
    light_vals[17] = ledMaxBright;
    light_vals[18] = ledMaxBright;
  }
  else if (letter == 21) {
    // V
    light_vals[0] = ledMaxBright;
    light_vals[2] = ledMaxBright;
    light_vals[4] = ledMaxBright;
    light_vals[6] = ledMaxBright;
    light_vals[8] = ledMaxBright;
    light_vals[10] = ledMaxBright;
    light_vals[12] = ledMaxBright;
    light_vals[14] = ledMaxBright;
    light_vals[17] = ledMaxBright;
  }
  else if (letter == 22) {
    // W
    light_vals[0] = ledMaxBright;
    light_vals[3] = ledMaxBright;
    light_vals[4] = ledMaxBright;
    light_vals[7] = ledMaxBright;
    light_vals[8] = ledMaxBright;
    light_vals[9] = ledMaxBright;
    light_vals[10] = ledMaxBright;
    light_vals[11] = ledMaxBright;
    light_vals[12] = ledMaxBright;
    light_vals[13] = ledMaxBright;
    light_vals[14] = ledMaxBright;
    light_vals[15] = ledMaxBright;
    light_vals[16] = ledMaxBright;
    light_vals[19] = ledMaxBright;
  }
  else if (letter == 23) {
    // X
    light_vals[0] = ledMaxBright;
    light_vals[3] = ledMaxBright;
    light_vals[4] = ledMaxBright;
    light_vals[5] = ledMaxBright;
    light_vals[6] = ledMaxBright;
    light_vals[7] = ledMaxBright;
    light_vals[9] = ledMaxBright;
    light_vals[10] = ledMaxBright;
    light_vals[12] = ledMaxBright;
    light_vals[13] = ledMaxBright;
    light_vals[14] = ledMaxBright;
    light_vals[15] = ledMaxBright;
    light_vals[16] = ledMaxBright;
    light_vals[19] = ledMaxBright;
  }
  else if (letter == 24) {
    // Y
    light_vals[0] = ledMaxBright;
    light_vals[3] = ledMaxBright;
    light_vals[4] = ledMaxBright;
    light_vals[5] = ledMaxBright;
    light_vals[6] = ledMaxBright;
    light_vals[7] = ledMaxBright;
    light_vals[9] = ledMaxBright;
    light_vals[10] = ledMaxBright;
    light_vals[13] = ledMaxBright;
    light_vals[14] = ledMaxBright;
    light_vals[17] = ledMaxBright;
    light_vals[18] = ledMaxBright;
  }
  else if (letter == 25) {
    // Z
    light_vals[4] = ledMaxBright;
    light_vals[5] = ledMaxBright;
    light_vals[6] = ledMaxBright;
    light_vals[7] = ledMaxBright;
    light_vals[10] = ledMaxBright;
    light_vals[13] = ledMaxBright;
    light_vals[16] = ledMaxBright;
    light_vals[17] = ledMaxBright;
    light_vals[18] = ledMaxBright;
    light_vals[19] = ledMaxBright;
  }
  else if (letter == 26) {
    clear_all();
  }
  write_lights();
  
}

void clear_all() {
    for (int i = 0; i < NUM_LIGHTS; i++) {
      light_vals[i] = 0;
    }
    write_lights();
}

void turn_everything_on() {
    for (int i = 0; i < NUM_LIGHTS; i++) {
      light_vals[i] = ledMaxBright;
    }
    write_lights();
}

void display_showing() {
   clear_all();

}

void write_lights() {
  for (int i = 0; i < NUM_LIGHTS; i++) {
    domeTLC.setPWM(i, light_vals[i]);
  }
  domeTLC.write();
}

void recvWithEndMarker() {
 static byte ndx = 0;
 char endMarker = '\n';
 char rc;
 
 // if (Serial.available() > 0) {
 while (Serial.available() > 0 && newData == false) {
   rc = Serial.read();
  
   if (rc != endMarker) {
   receivedChars[ndx] = rc;
   ndx++;
   if (ndx >= numChars) {
    ndx = numChars - 1;
   }
 }
 else {
   receivedChars[ndx] = '\0'; // terminate the string
   ndx = 0;
   newData = true;
  }
 }
}

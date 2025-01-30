#include "AS5600.h"
#include "Wire.h"
#include "Servo.h"


AS5600 as5600[5];
Servo servos[5]; // Array of Servo objects for each finger
// Initiate encoder values
int maxFingers[5] = {-10000, -10000, -10000, -10000, -10000};
int minFingers[5] = {10000, 10000, 10000, 10000, 10000};

// Pins for servomotors
const int servoPins[5] = {6, 5, 4, 3, 2};

// Coefficient sizes
const float S_m = 0.6;
const float M_m = 0.8;

// Initial servo positions for each finger
const int initialServoPositions[5] = {0, 0, 0, 0, 0}; // Calib min
// const int initialServoPositions[5] = {120, 120, 120, 120, 120}; // Calib loose, it helps to constrain more the fingers to better feel larger shapes
// const int initialServoPositions[5] = {180, 180, 180, 180, 180}; // Calib max

// const int initialServoPositions[5] = {60*S_m, 180*S_m, 150*S_m, 180*S_m, 60*S_m}; // Cube S
// const int initialServoPositions[5] = {60*M_m, 180*M_m, 150*M_m, 180*M_m, 60*M_m}; // Cube M
// const int initialServoPositions[5] = {60, 180, 150, 180, 60}; // Cube L

// const int initialServoPositions[5] = {60*S_m, 100*S_m, 180*S_m, 100*S_m, 60*S_m}; // Sphere S
// const int initialServoPositions[5] = {60*M_m, 100*M_m, 180*M_m, 100*M_m, 60*M_m}; // Sphere M
// const int initialServoPositions[5] = {60, 100, 180, 100, 60}; // Sphere L

// const int initialServoPositions[5] = {180, 0, 0, 180, 120}; // Random shape




void setup() {
  // put your setup code here, to run once:
  Wire.begin(); // Initialize the I2C bus
  Wire.setClock(40000);
  Serial.begin(115200); // Initialize serial communication
#define MULTIPLEXER_ADDR 0x70 // Address of the I2C multiplexer
  for (int i = 0; i < 5; i++) {
  as5600[i].begin(4);  //  set direction pin.
  as5600[i].setDirection(AS5600_COUNTERCLOCK_WISE);  // default, just be explicit
  }
    // Attach servomotors to their respective pins and set initial positions
  for (int i = 0; i < 5; i++) {
    servos[i].attach(servoPins[i]);
    servos[i].write(90); // Set initial position
  }
  delay(500);
  for (int i = 0; i < 5; i++) {
    servos[i].write(180-initialServoPositions[i]); // Set initial position
  }
}


int rawFingers[5];
void loop() {
  // put your main code here, to run repeatedly:

for (int i = 0; i < 5; i++) {

  selectChannel(i); // Select channel for the i-th AS5600 sensor
  rawFingers[4-i] = as5600[4-i].getCumulativePosition(); // inverse finger position
  }
  //if during the calibration sequence, make sure to update max and mins

// This function transforms raw encoder values mapped from 0 to 180
  static int calibrated[5] = {90,90,90,90,90};
  for (int i = 0; i < 5; i++){
    if (rawFingers[i] > maxFingers[i])
      maxFingers[i] = rawFingers[i];

    if (rawFingers[i] < minFingers[i])
      minFingers[i] = rawFingers[i];
  }

  for (int i = 0; i < 5; i++){
    if (minFingers[i] != maxFingers[i]){
      calibrated[i] = map( rawFingers[i], minFingers[i], maxFingers[i], 0, 180 );
      #if CLAMP_ANALOG_MAP
        if (calibrated[i] < 0)
          calibrated[i] = 0;
        if (calibrated[i] > 180)
          calibrated[i] = 180;
      #endif
    }
    else {
      calibrated[i] = 180 / 2;
    }
     Serial.print(calibrated[i]);
     Serial.print(" ");
  }
   Serial.println();
  return calibrated;
}

// Read every channel from I2C multiplexer to gather encoder data
void selectChannel(int channel) {
  Wire.beginTransmission(MULTIPLEXER_ADDR);
  Wire.write(1 << channel); // Select the desired channel
  Wire.endTransmission();
}

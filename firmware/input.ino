#include "AS5600.h"
#include "Wire.h"
// Requires RunningMedian library by Rob Tillaart
#if ENABLE_MEDIAN_FILTER
  #include <RunningMedian.h>
  RunningMedian rmSamples[5] = {
      RunningMedian(MEDIAN_SAMPLES),
      RunningMedian(MEDIAN_SAMPLES),
      RunningMedian(MEDIAN_SAMPLES),
      RunningMedian(MEDIAN_SAMPLES),
      RunningMedian(MEDIAN_SAMPLES)
  };
#endif


#define MULTIPLEXER_ADDR 0x70 // Address of the I2C multiplexer
AS5600 as5600[5];
int maxFingers[5] = {-10000, -10000, -10000, -10000, -10000};
int minFingers[5] = {10000, 10000, 10000, 10000, 10000};

void setupInputs(){

  for (int i = 0; i < 5; i++) {
  as5600[i].begin(4);  //  set direction pin.
  as5600[i].setDirection(AS5600_COUNTERCLOCK_WISE);  // default, just be explicit.
  }
  pinMode(PIN_JOY_BTN, INPUT_PULLUP);
  pinMode(PIN_A_BTN, INPUT_PULLUP);
  pinMode(PIN_B_BTN, INPUT_PULLUP);

  pinMode(PIN_MENU_BTN, INPUT_PULLUP);
  
  #if !TRIGGER_GESTURE
  pinMode(PIN_TRIG_BTN, INPUT_PULLUP);
  #endif

  #if !GRAB_GESTURE
  pinMode(PIN_GRAB_BTN, INPUT_PULLUP);
  #endif

  #if !PINCH_GESTURE
  pinMode(PIN_PNCH_BTN, INPUT_PULLUP);
  #endif

  #if USING_CALIB_PIN
  pinMode(PIN_CALIB, INPUT_PULLUP);
  #endif
}

int* getFingerPositions(bool calibrating, bool reset){
  int rawFingers[5];
for (int i = 0; i < 5; i++) {

    selectChannel(i); // Select channel for the i-th AS5600 sensor
    rawFingers[i] = as5600[i].getCumulativePosition();

}

  //reset max and mins as needed
  if (reset){
    for (int i = 0; i <5; i++){
      maxFingers[i] = -ANALOG_MAX;
      minFingers[i] = ANALOG_MAX;
    }
  }
  
  //if during the calibration sequence, make sure to update max and mins
  if (calibrating){
    for (int i = 0; i < 5; i++){
      if (rawFingers[i] > maxFingers[i])
        #if CLAMP_FLEXION
          maxFingers[i] = ( rawFingers[i] <= CLAMP_MAX )? rawFingers[i] : CLAMP_MAX;
        #else
          maxFingers[i] = rawFingers[i];
        #endif
      if (rawFingers[i] < minFingers[i])
        #if CLAMP_FLEXION
          minFingers[i] = ( rawFingers[i] >= CLAMP_MIN )? rawFingers[i] : CLAMP_MIN;
        #else
          minFingers[i] = rawFingers[i];
        #endif
    }
  }  
  static int calibrated[5] = {511,511,511,511,511};
  
  for (int i = 0; i < 5; i++){
    if (minFingers[i] != maxFingers[i]){
      calibrated[i] = map( rawFingers[i], minFingers[i], maxFingers[i], 0, 1023 );
      #if CLAMP_ANALOG_MAP
        if (calibrated[i] < 0)
          calibrated[i] = 0;
        if (calibrated[i] > 1023)
          calibrated[i] = 1023;
      #endif
    }
    else {
      calibrated[i] = 1023 / 2;
    }
    // Serial.print(rawFingers[i]);
    // Serial.print(" ");
  }
  // Serial.println();
  return calibrated;
}

int analogReadDeadzone(byte pin){
  int raw = analogRead(pin);
  if (abs(ANALOG_MAX/2 - raw) < JOYSTICK_DEADZONE * ANALOG_MAX / 100)
    return ANALOG_MAX/2;
  else
    return raw;
}

int getJoyX(){
  #if JOYSTICK_BLANK
  return ANALOG_MAX/2;
  #elif JOY_FLIP_X
  return ANALOG_MAX - analogReadDeadzone(PIN_JOY_X);
  #else
  return analogReadDeadzone(PIN_JOY_X);
  #endif
}

int getJoyY(){
  #if JOYSTICK_BLANK
  return ANALOG_MAX/2;
  #elif JOY_FLIP_Y
  return ANALOG_MAX - analogReadDeadzone(PIN_JOY_Y);
  #else
  return analogReadDeadzone(PIN_JOY_Y);
  #endif
}

void selectChannel(int channel) {
  Wire.beginTransmission(MULTIPLEXER_ADDR);
  Wire.write(1 << channel); // Select the desired channel
  Wire.endTransmission();
}

bool getButton(byte pin){
  return digitalRead(pin) != HIGH;
}

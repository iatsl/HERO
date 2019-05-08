//Works for New Right or Left Gloves: Auto mode with Lock button, uses shock and double-tap detection
#include <CurieIMU.h>
#include <Servo.h>

//******************* Variables ****************
// Motors
Servo myservoflex;                //create servo object to control flexion
Servo myservoextend;              //create servo object to control extension
const int extendMotor = 150;      //extended motor value
const int retractMotor = 50;      //retracted motor value
const int extend = 0;             //fingers extended
const int flex = 1;               //fingers flexed
const int relax = 2;              //fingers relaxed
int fingerposition = relax;       //finger position tracker

// Input Timeout
unsigned long currenttime = 0;    //current time
const int interval = 1000;        //interval between accepting inputs (ms)
unsigned long timeoutend = 0;     //time when inputs are accepted again
bool timeout = false;             //flag for input timeout

// Lock Button
bool button = false;              //button position
bool lock = false;                //lock
volatile bool motionflag = false; //motion flag
const int BUTTON_PIN = 3;         //input pin for button

//******************** Setup *******************
void setup() {
  // Set up servos
  myservoflex.attach(A1);         //attaches the servo
  myservoextend.attach(A3);       //attaches the servo
  movemotors(relax);              //relax the glove
  
  // Set up I/O pins
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  pinMode(BUTTON_PIN, INPUT_PULLUP);  //lock button
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  
  // Set up IMU
  CurieIMU.begin();
  // Set Accelerometer range (2g, 4g, 8g, 16g)
  CurieIMU.setAccelerometerRange(4);
  // Set Double-Tap detection threshold
  // 2G: 31.25 to 7968.75 mg, in steps of 62.5 mg 
  // 4G: 62.50 to 31937.50 mg, in steps of 125.0 mg 
  CurieIMU.setDetectionThreshold(CURIE_IMU_DOUBLE_TAP, 62.5);
  // Set Double-Tap durations (acceptance, duration, time between)
  // 50, 100, 150, 200, 250, 275, 500, 700 ms
  CurieIMU.setDetectionDuration(CURIE_IMU_DOUBLE_TAP, 700);
  CurieIMU.setDetectionDuration(CURIE_IMU_TAP_SHOCK, 50); // (50,75 ms)
  CurieIMU.setDetectionDuration(CURIE_IMU_TAP_QUIET, 20); // (20,30 ms)
  // Set Shock detection threshold and duration
  // 2G: 3.91 to 1995.46 mg, in steps of 7.81 mg 
  // 4G: 7.81 to 3993.46 mg, in steps of 15.63 mg 
  CurieIMU.setDetectionThreshold(CURIE_IMU_SHOCK, 1750);
  CurieIMU.setDetectionDuration(CURIE_IMU_SHOCK, 75);     // (50, 75 ms)
  // Attach interrupts
  CurieIMU.attachInterrupt(motioninput);
  CurieIMU.interrupts(CURIE_IMU_DOUBLE_TAP);        //Double-Tap detection
  CurieIMU.interrupts(CURIE_IMU_SHOCK);             //Shock detection

  settimeout(interval);               //wait before accepting inputs
  button = digitalRead(BUTTON_PIN);   //initialize lock button to starting position
}

//***************************** Main Loop ***********************************
void loop() {

  // Relax glove when lock mode is turned off
  if (digitalRead(BUTTON_PIN) != button){
    button = !button;               //toggle button position
    if (lock == true){              //extend on change from lock to unlock
    settimeout(interval);
    movemotors(extend);
    fingerposition = extend;
    }
    lock = !lock;                   //toggle lock
  }
  
  // Read current time and check for timeout
  currenttime = millis();
  if (currenttime >= timeoutend) timeout = false;

  // Motion Detection
  if ((lock == false) && (timeout == false) && (motionflag == true)){
      settimeout(interval);                                             //set an input timeout
      motionflag = false;                                               //clear motion flag
      
      if ((fingerposition == flex) || (fingerposition == relax)){       //fingers flexed or relaxed, move to extension      
        movemotors(extend);
        fingerposition = extend;
      }
      else if (fingerposition == extend){                               //fingers extended, move to flexion
        movemotors(flex);
        fingerposition = flex;
      }         
  }

}

//*************** Movement function for finger motors *******************
void movemotors (const int fp){
  if (fp == extend){
    myservoflex.write(retractMotor);       //extend fingers
    myservoextend.write(extendMotor);
  }
  else if (fp == flex){
    myservoflex.write(extendMotor);        //flex fingers
    myservoextend.write(retractMotor);
  }
  else if (fp == relax){
    myservoflex.write(retractMotor);       //relax fingers
    myservoextend.write(retractMotor);
  }
}

//************** Input timeout function ******************************
void settimeout(int howlong){
  currenttime = millis();                 //get current time
  timeout = true;                         //set timeout flag
  timeoutend = currenttime+howlong;       //set end of input timeout
}

//************* Motion input interrupt *************************
void motioninput(){
  if ((lock == false) && (timeout == false) && (CurieIMU.getInterruptStatus(CURIE_IMU_DOUBLE_TAP))) motionflag = true;
  if ((lock == false) && (timeout == false) && (CurieIMU.getInterruptStatus(CURIE_IMU_SHOCK)))  motionflag = true;
}

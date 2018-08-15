#include <Adafruit_DotStar.h>
#include <SPI.h>
#include <Servo.h>
#include <i2c_t3.h>
//#include <Wire.h>

#define enablePin 22
#define PIN_IR 15
#define motor1 3
#define motor2 4
#define vBatt A0

//animations
unsigned long nextFrameAt = 0;

struct Frame {
  uint16_t duration;//in degrees or milliseconds
  struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
  } led[5];
  struct Frame *next;
} *animationHead, *currentFrame, *idleHead, *tankHead, *spinHead;

void defineAnimations(void);
void resetStaticAnimation(void);
void resetDynamicAnimation(void);

//serial
#define SERIAL_WAIT 0
#define SERIAL_PACKETSTART 1
byte serialState = 0;
unsigned long packetTime = 0;
unsigned long packetTimeout = 200;
byte packet[10];
byte bytesRead = 0;

unsigned long lastReceived = 0;

//controls
byte dir = 0;
byte flip = 0;
int16_t thumbX = 0;
int16_t thumbY = 0;
uint16_t throt = 0;
uint16_t head = 0;
byte en = 0;

uint16_t meltyAngle = 0;//the commanded bearing angle for meltybrain control
uint16_t meltyThrottle = 0;
//leds
Adafruit_DotStar strip = Adafruit_DotStar(5, DOTSTAR_GBR);

//**********************//
// MELTYBRAIN VARIABLES //
//**********************//
uint16_t angle = 0;//LSB is one degree

#define BEACON_SENSING 0x01//if this is defined, we are angle sensing using only the infrared receiver
#define ACCEL_SENSING 0x02//if this is defined, we are angle sensing using only the accelerometer
#define HYBRID_SENSING 0x03//if this is defined, we are angle sensing using both the beacon and the accelerometer
uint8_t senseMode = ACCEL_SENSING;

//BEACON
boolean beacon = false;//this variable keeps track of the status of the beacon internally. If this variable and the digital read don't match, it's a rising or falling edge

unsigned long beaconEdgeTime[2];//this is the array of rising edge acquisition times. We keep some history for better extrapolation

bool beaconEnvelopeStarted = false;
unsigned long beaconHoldTime;
#define BEACON_DEBOUNCE_TIME 2000//in microseconds
uint8_t beaconEdgesRecorded = 0;//this keeps track of how many beacon pulses we've seen, up to APPROXIMATION_ORDER. It resets when a revolution takes too long
#define REV_TIMEOUT 2000 //this (in ms) is how long a revolution can take before we say the robot is spinning too slowly to control and reset the algorithm

#define MELTY_PULSE_WIDTH 30//this is the arc length in degrees over which the motor power is pulsed higher or lower while translating

//ACCELEROMETER
void configAccelerometer(void);

//states
uint8_t state = 1;

#define STATE_IDLE 1
#define STATE_TANK 2
#define STATE_SPIN 3

//motors
Servo ESC1;
Servo ESC2;

void pollSerial(void);
void receivePacket(void);

void shiftToLEDs(void);
void runStaticAnimation(void);
void runDynamicAnimation(void);

void runMeltyBrain(void);

uint16_t getBatteryVoltage() { //returns voltage in millivolts
  return analogRead(vBatt)*5;
}


void setMotorSpeed(int motor, int spd) {
  spd = constrain(spd, -100, 100);//make sure our speed value is valid. This lets us be lazier elsewhere
  //apply a deadband
  if(spd < 5 && spd > -5) spd = 0;

  if(motor == motor1) spd *= -1;
  
  int pulseTime = map(spd, -100, 100, 1000, 2000);
  
  if(motor == motor1) {
    ESC1.writeMicroseconds(pulseTime);
  } else if(motor == motor2) {
    ESC2.writeMicroseconds(pulseTime);
  }
}

void goIdle() {
  state = STATE_IDLE;

  //digitalWrite(enablePin, HIGH);
  setMotorSpeed(motor1, 0);
  setMotorSpeed(motor2, 0);
  
  animationHead = idleHead;
  resetStaticAnimation();
}

void goTank() {
  state = STATE_TANK;

  digitalWrite(enablePin, LOW);
  
  animationHead = tankHead;
  resetStaticAnimation();
}

void goSpin() {
  state = STATE_SPIN;

  digitalWrite(enablePin, LOW);

  beaconEdgesRecorded = 0;

  animationHead = spinHead;
  currentFrame = animationHead;
  nextFrameAt = currentFrame->duration + angle;
  shiftToLEDs();
}

void feedWatchdog() {
  noInterrupts();
  WDOG_REFRESH = 0xA602;
  WDOG_REFRESH = 0xB480;
  interrupts();
}

//this runs if the robot code hangs! cut off the motors
void watchdog_isr() {
  digitalWrite(enablePin, LOW);
}

void setup() {
  Serial.begin(57600);
  Serial1.begin(57600);
  SPI.begin();
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, HIGH);

  pinMode(PIN_IR, INPUT);

  ESC1.attach(motor1, 1000, 2000);
  ESC2.attach(motor2, 1000, 2000);

  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 1800000, I2C_OP_MODE_IMM);//1.8MHz clock rate
  //Wire.begin();

  //SETUP WATCHDOG
  //settings taken from: https://bigdanzblog.wordpress.com/2017/10/27/watch-dog-timer-wdt-for-teensy-3-1-and-3-2/
  noInterrupts();
  WDOG_UNLOCK = WDOG_UNLOCK_SEQ1;
  WDOG_UNLOCK = WDOG_UNLOCK_SEQ2;
  delayMicroseconds(1);

  WDOG_TOVALH = 0x006d; //1 second timer
  WDOG_TOVALL = 0xdd00;
  WDOG_PRESC = 0x400;
  WDOG_STCTRLH |= WDOG_STCTRLH_ALLOWUPDATE | WDOG_STCTRLH_WDOGEN | 
                  WDOG_STCTRLH_WAITEN | WDOG_STCTRLH_STOPEN |
                  WDOG_STCTRLH_CLKSRC | WDOG_STCTRLH_IRQRSTEN;
  interrupts();

  NVIC_ENABLE_IRQ(IRQ_WDOG);//enable watchdog interrupt

  configAccelerometer();

  //build the animations
  defineAnimations();

  goIdle();
}

void loop() {

  //Bark bark
  feedWatchdog();

  //check for incoming messages
  pollSerial();

  //make sure comms haven't timed out
  if(micros() - lastReceived > 1000*1000 && state != STATE_IDLE) {
    en = 0x0;
    goIdle();
  }

  switch(state) {
    case STATE_IDLE:
      runStaticAnimation();

      if(en == 0xAA) {
        goTank();
      }
      
      break;
    case STATE_TANK:
      
      setMotorSpeed(motor1, constrain(thumbY+thumbX, -100, 100));
      setMotorSpeed(motor2, constrain(thumbY-thumbX, -100, 100));
    
      runStaticAnimation();
      
      if(throt > 2) {
        goSpin();
      }

      if(en != 0xAA) {
        goIdle();
      }
      break;
    case STATE_SPIN:

      runMeltyBrain();//manage all of the sensors and predict our current heading
     
      runDynamicAnimation();
      
      if(throt < 2) {
        goTank();
      }

      if(en != 0xAA) {
        goIdle();
      }
    default:
      break;
  }
}




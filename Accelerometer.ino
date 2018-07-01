#define ACCEL_BLOCKING//the code waits for the I2C transimission to complete before continuing
//#define ACCEL_NONBLOCKING//the code starts the transmission, then periodically check for completion. Not currently implemented

#define ADDR_ACCEL 0x19

#define WHO_A,_I 0x0F
#define CTRL_REG1 0x20
#define CTRL_REG2 0x21
#define CTRL_REG3 0x22
#define CTRL_REG4 0x23
#define CTRL_REG5 0x24
#define HP_FILTER_RESET 0x25
#define REFERENCE 0x26
#define STATUS_REG 0x27
#define OUT_X_L 0x28
#define OUT_X_H 0x29
#define OUT_Y_L 0x2A
#define OUT_Y_H 0x2B
#define OUT_Z_L 0x2C
#define OUT_Z_H 0x2D
#define INT1_CFG 0x30
#define INT1_SRC 0x31
#define INT1_THS 0x32
#define INT1_DURATION 0x33
#define INT2_CFG 0x34
#define INT2_SRC 0x35
#define INT2_THS 0x36
#define INT2_DURATION 0x37

//write a single byte to an I2C device
uint8_t writeI2CReg8Blocking(uint8_t addr, uint8_t subaddr, uint8_t data) {
  Wire.beginTransmission(addr);
  Wire.write(subaddr);
  Wire.write(data);
  return Wire.endTransmission();
}

//read N bytes from an I2C device
uint8_t readI2CRegNBlocking(uint8_t addr, uint8_t subaddr, uint8_t buflen, uint8_t *buf) {
  Wire.beginTransmission(addr);
  Wire.write(subaddr);
  Wire.endTransmission();
  Wire.requestFrom(addr, buflen);
  Wire.endTransmission();
  for(int i=0; i<buflen; i++) {
    buf[i] = Wire.readByte();
  }
  return Wire.endTransmission();
}

void recieveEvent(size_t count) {
  
}

void requestEvent(void) {
  
}

void configAccelerometer() {
  writeI2CReg8Blocking(ADDR_ACCEL, CTRL_REG1, 0x36);//1000Hz, normal mode, YZ enabled
  writeI2CReg8Blocking(ADDR_ACCEL, CTRL_REG4, 0xB0);//block data update enabled, 400g full scale
}

//we read from the accelerometer much slower than the accelerometer's data rate to make sure we always get new data
//reading the same data twice could mess with the prediction algorithms
//a better method is to use the interrupt pin on the accelerometer that tells us every time new data is available
unsigned long measurementPeriod = 2500;//in microseconds. Represents 400Hz

double robotSpeed = 0;
//"c" represents the radius term and the conversion from LSB to m/s^2. It is robot-specific and should be carefully calibrated
double c = 1;

void runAccelerometer() {
#ifdef ACCEL_BLOCKING
  //this uses blocking I2C, which makes it relatively slow. But given that we run our I2C ar 1.8MHz we will likely be okay
  if(micros() - lastAccelMeasTime > measurementPeriod) {
    lastAccelMeasTime = micros();
    
    uint8_t accelBuf[6];
    if(readI2CRegNBlocking(ADDR_ACCEL, OUT_X_L, 6, accelBuf) > 0) {
      senseMode = BEACON_SENSING;//if communication with the accelerometer ever fails, we revert to beacon-only mode
      Serial.println("accelerometer failed");
    }
  
    uint16_t xAccel = (((uint16_t) accelBuf[0]) << 8) | (uint16_t) accelBuf[1];//represents acceleration tangential to the ring, not useful to us
    uint16_t yAccel = (((uint16_t) accelBuf[2]) << 8) | (uint16_t) accelBuf[3];//represents acceleration axial to the ring, which shows which way the bot is flipped
    uint16_t zAccel = (((uint16_t) accelBuf[4]) << 8) | (uint16_t) accelBuf[5];//represents acceleration radial to the ring, which is a measure of rotation speed
    
    //"c" is a constant that must be calibrated for your bot
    //it combines the radius term, as well as the conversion from accel measurements to m/s^2
    //Also, sqrt can be slow! use a lookup table instead if speed is an issue
    robotSpeed = sqrt((double) (zAccel)/c);//if there are issues with execution speed, replace this line with a lookup table and use only integer math
    angleAtLastMeasurement = angle;
  }
#endif
}


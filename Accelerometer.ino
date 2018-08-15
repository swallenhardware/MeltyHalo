//H3LIS331DL defines
#define ADDR_ACCEL 0x18

#define WHO_AM_I 0x0F
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
  Wire.write(subaddr | 0x80);//the current accelerometer requires that the msb be set high to do a multi-byte transfer
  Wire.endTransmission();
  Wire.requestFrom(addr, buflen);
  while(Wire.available()) *(buf++) = Wire.readByte();
  return Wire.getError();
}

void recieveEvent(size_t count) {
  
}

void requestEvent(void) {
  
}

void configAccelerometer() {
  writeI2CReg8Blocking(ADDR_ACCEL, CTRL_REG1, 0x2E);//1000Hz, normal mode, YZ enabled
  writeI2CReg8Blocking(ADDR_ACCEL, CTRL_REG4, 0x30);//block data update enabled, 400g full scale
}

//we read from the accelerometer much slower than the accelerometer's data rate to make sure we always get new data
//reading the same data twice could mess with the prediction algorithms
//a better method is to use the interrupt pin on the accelerometer that tells us every time new data is available
unsigned long measurementPeriod = 10000;//in microseconds. Represents 100Hz

uint16_t robotPeriod[2];//measured in microseconds per degree, with some memory for discrete integration

//this is the times we measured the accelerometer at. We keep some history for extrapolation
unsigned long accelMeasTime[2];

//this angle (degrees) is calculated only using the accelerometer. We keep it separate to keep our discrete integration algorithms operating smoothly
//the beacon sets our heading to 0, which would mess up the discrete integration if allowed to affect this variable directly
//instead we utilize a trim variable. In Accel control mode, the user controls trim with the encoder wheel. in hybrid mode, the beacon controls trim
uint16_t accelAngle = 0;

//in degrees, this angle is added to the accel angle as adjusted by the beacon or the driver
uint16_t accelTrim = 0;

uint16_t angleAtLastMeasurement;

void runAccelerometer() {
  //this uses blocking I2C, which makes it relatively slow. But given that we run our I2C ar 1.8MHz we will likely be okay
  if(micros() - accelMeasTime[0] > measurementPeriod) {
    //shift all of the old values down
    for(int i=1; i>0; i--) {
      accelMeasTime[i] = accelMeasTime[i-1];
    }
    //put in the new value
    accelMeasTime[0] = micros();
    
    uint8_t accelBuf[6];
    if(readI2CRegNBlocking(ADDR_ACCEL, OUT_X_L, 6, accelBuf) > 0) {
      senseMode = BEACON_SENSING;//if communication with the accelerometer ever fails, we revert to beacon-only mode
      Serial.println("accelerometer failed");
    }
  
    //int16_t xAccel = (((int16_t) accelBuf[1]) << 8) | (int16_t) accelBuf[0];//represents acceleration tangential to the ring, not useful to us
    int16_t yAccel = (((int16_t) accelBuf[3]) << 8) | (int16_t) accelBuf[2];//represents acceleration axial to the ring, which shows which way the bot is flipped
    int16_t zAccel = (((int16_t) accelBuf[5]) << 8) | (int16_t) accelBuf[4];//represents acceleration radial to the ring, which is a measure of rotation speed

    //shift all of the old values down
    for(int i=1; i>0; i--) {
      robotPeriod[i] = robotPeriod[i-1];
    }
    
    //put in the new value
    //this equation has been carefully calibrated for this bot. See here for explanation:
    //https://www.swallenhardware.io/battlebots/2018/8/12/halo-pt-9-accelerometer-calibration
    robotPeriod[0] = (uint32_t) (855 / sqrt((double) (zAccel-126)/607)) - 122L;

    //give up if the bot is moving too slowly
    if(zAccel < 400) return;

    //find the new angle
    //TRIANGULAR INTEGRATION
    uint32_t deltaT = accelMeasTime[0] - accelMeasTime[1];
    angleAtLastMeasurement = (angleAtLastMeasurement + (deltaT/robotPeriod[0] + deltaT/robotPeriod[1])/2) % 360;

    accelAngle = angleAtLastMeasurement;
    
  } else {//if it isn't time to check the accelerometer, predict our current heading
    //predict the current velocity by extrapolating old data
    uint32_t newTime = micros();
    uint32_t periodPredicted = robotPeriod[1] + (newTime - accelMeasTime[1]) * (robotPeriod[0] - robotPeriod[1]) / (accelMeasTime[0] - accelMeasTime[1]);

    //predict the current robot heading by triangular integration up to the extrapolated point
    uint32_t deltaT = newTime - accelMeasTime[0];
    accelAngle = (angleAtLastMeasurement + (deltaT/periodPredicted + deltaT/robotPeriod[0])/2) % 360;
  }
}


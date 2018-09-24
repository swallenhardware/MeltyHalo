void runMeltyBrain() {
  unsigned long currTime = micros();//record the start of this iteration

  //ACCELEROMETER SENSING
  if(senseMode != BEACON_SENSING) {
    runAccelerometer();
  }
  
  //BEACON SENSING
  if(senseMode != ACCEL_SENSING) {
    //read the sensor
    bool beaconReading = !digitalRead(PIN_IR);
  
    //debounce the reading
    if(!beaconEnvelopeStarted && beaconReading) {//when we detect a new beacon envelope starting
      beaconEnvelopeStarted = true;
      beaconHoldTime = micros();
    } else if(beaconEnvelopeStarted && !beaconReading) {//if we get a 0 reading after we start measuring an envelope
      beaconEnvelopeStarted = false;
    }
    
    if(beaconEnvelopeStarted && micros()-beaconHoldTime > BEACON_DEBOUNCE_TIME && !beacon) {//on the rising edge of a debounced IR detections
      beacon = true;

      //if the accelerometer is active, adjust the trim so that the heading becomes 0
      if(senseMode == HYBRID_SENSING) {
        //here we use the accelerometer to help debounce the beacon
        //we compare how long it's been since the last acceleration to how long the accel says it should take (period * 360) * (% of revolution required)
        if(currTime - beaconEdgeTime[0] > robotPeriod[0]*270) {
          accelTrim = 360 - accelAngle - 40;//the 40 accounts for the fact that the beacon pics up 40 degrees before forward
        }
      } else {//else we are controlling the angle directly, so set it directly
        angle = 0;
      }
      
      if(currTime - beaconEdgeTime[0] > REV_TIMEOUT) {//if we are rotating fast enough to start the control algorithm
        
        for(int i=1; i>0; i--) {//we shift back the previous edge times
          beaconEdgeTime[i] = beaconEdgeTime[i-1];
        }
        
        if(beaconEdgesRecorded < 2) {//if we haven't recorded a long enough history to run the algorithm
          beaconEdgesRecorded++;
        }
        
      } else {//if we are rotating too slow, restart the algorithm
        beaconEdgesRecorded = 0;
      }
      beaconEdgeTime[0] = currTime;
  
    } else if(!beaconReading && beacon) {//on the falling edge of the IR receiver
      beacon = false;
    }

    //if we have enough historical data, extrapolate from it to estimate where we are now
    //this isn't used if the accelerometer is active
    if(beaconEdgesRecorded == 2 && senseMode == BEACON_SENSING) {
  
      //the linear algorithm
      angle = (uint16_t) (((uint32_t) (currTime - beaconEdgeTime[1]) * 360) / (uint32_t) (beaconEdgeTime[0] - beaconEdgeTime[1]) - 360);
    }
  }

  //if the accelerometer is active, apply the trim
  if(senseMode != BEACON_SENSING) {
    angle = (accelAngle + accelTrim) % 360;
  }

  //MOTOR COMMAND
  //first check if the melty throttle is high enough for translation
  if(meltyThrottle > 10) {
    //calculate the distance between the current heading and the commanded direction
    int16_t diff = 180 - abs(abs(meltyAngle - angle) - 180);

    //now check if we are pointed towards the commanded direction or opposite or neither
    if(abs(diff) < 90) {
      //we are pointing towards the commanded heading, forward pulse
      setMotorSpeed(motor1, (flip*2-1)*(throt-meltyThrottle));
      setMotorSpeed(motor2, (1-flip*2)*(throt+meltyThrottle));
    } else {
      //we are pointing opposite the commanded heading, reverse pulse
      setMotorSpeed(motor1, (flip*2-1)*(throt+meltyThrottle));
      setMotorSpeed(motor2, (1-flip*2)*(throt-meltyThrottle));
    }
  } else {
    //if we aren't translating, just run the motors at the throttle speed
    setMotorSpeed(motor1, (flip*2-1)*throt);
    setMotorSpeed(motor2, (1-flip*2)*(throt));
  }
}


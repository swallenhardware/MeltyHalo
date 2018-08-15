void pollSerial() {
  while(Serial1.available()) {
    if(serialState == SERIAL_WAIT) {
      if(Serial1.read() == 0x7E) {
        serialState = SERIAL_PACKETSTART;
        bytesRead = 0;
        continue;
      }
    }
    packet[bytesRead] = Serial1.read();
    bytesRead++;

    if(bytesRead >= 10) {
      receivePacket();
      bytesRead = 0;
      serialState = SERIAL_WAIT;
    }
  }
}

void receivePacket() {
  lastReceived = micros();
  
  //status byte
  byte stat = packet[0];//nothing is currently done here. Can be used to transmit switch states, etc.

  //thumbstick X
  thumbX = map((((uint16_t) packet[1]) << 8) | ((uint16_t) packet[2]), 0, 1024, -100, 100);
  //thumbstick Y
  thumbY = map((((uint16_t) packet[3]) << 8) | ((uint16_t) packet[4]), 0, 1024, -100, 100);
  //throttle
  throt = map((((uint16_t) packet[5]) << 8) | ((uint16_t) packet[6]), 0, 1024, 0, 100);
  //heading
  head = ((uint16_t) packet[7]) << 8 | ((uint16_t) packet[8]);
  //enable
  en = packet[9];

  if(state == STATE_SPIN) {
    //calculate the commanded direction and speed
    meltyThrottle = sqrt(thumbX*thumbX + thumbY*thumbY);
    meltyAngle = degrees(atan2(thumbX, thumbY));
  }

  //now we build the return packet
  packet[0] = 0x7E;//start of packet byte

  //standard return packet
  uint16_t batteryVoltage = getBatteryVoltage();
  packet[1] = (byte) ((batteryVoltage & 0xFF00) >> 8);
  packet[2] = (byte) (batteryVoltage & 0x00FF);
  packet[3] = 0x00;

  Serial1.write(packet, 4);

  /*
  //this code is used in calibration testing
  packet[1] = (byte) (((beaconEdgeTime[0] - beaconEdgeTime[1]) & 0xFF000000) >> 24);
  packet[2] = (byte) (((beaconEdgeTime[0] - beaconEdgeTime[1]) & 0x00FF0000) >> 16);
  packet[3] = (byte) (((beaconEdgeTime[0] - beaconEdgeTime[1]) & 0x0000FF00) >> 8);
  packet[4] = (byte) ((beaconEdgeTime[0] - beaconEdgeTime[1]) & 0x000000FF);
  packet[5] = (byte) ((robotPeriod & 0xFF00) >> 8);
  packet[6] = (byte) (robotPeriod & 0x00FF);

  Serial1.write(packet, 7);
  */
}

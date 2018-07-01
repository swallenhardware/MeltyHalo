void shiftToLEDs() {
  for(int i=0; i<5; i++) {
    strip.setPixelColor(i, frames[frame][i]);
  }

  strip.show();
}

void runDynamicAnimation() {//angle is from 0-360
  uint16_t newFrame = (angle * numFrames) / 360;

  if(newFrame != frame) {
    frame = newFrame;
    shiftToLEDs();
  }
}

void runStaticAnimation() {
  if(micros() - lastChange > frameLength) {
    lastChange = micros();
    frame++;
    if(frame >= numFrames) frame = 0;

    shiftToLEDs();
  }
}

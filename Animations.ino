//adds a fresh frame to the end of a list
struct Frame * createEmptyFrame() {
  struct Frame * f = (struct Frame *) malloc(sizeof(struct Frame));
  for(int i=0; i<5; i++) {
    f->led[i].red = 0x00;
    f->led[i].green = 0x00;
    f->led[i].blue = 0x00;
  }
  f->next = NULL;
  return f;
}

//adds a blank struct to the end of the current tail, and returns the new tail
struct Frame * addEmptyFrame(struct Frame *tail) {
  tail->next = createEmptyFrame();
  return tail->next;
}
//builds a frame of all the same color, with the leds selected by the first 5 bits of a uint8_T
struct Frame * addMonoFrame(struct Frame * tail, uint8_t leds, uint8_t red, uint8_t green, uint8_t blue, uint16_t duration) {
  struct Frame * tempFrame = addEmptyFrame(tail);
  tempFrame->duration = duration;
  for(int i=0; i<5; i++) {
    if((leds >> i) & 0x01) { 
      tempFrame->led[i].red = red;
      tempFrame->led[i].green = green;
      tempFrame->led[i].blue = blue;
    }
  }

  return tempFrame;
}

//adds a single character to the display
//this is the font table we use to add text to the display
//returns the new tail
struct Frame * addChar(struct Frame *tail, char c, uint8_t red, uint8_t green, uint8_t blue, uint16_t duration) {
  struct Frame * tempFrame = addEmptyFrame(tail);
  tempFrame->duration = duration/6;
  switch(c) {
  case 'A':
    tempFrame = addMonoFrame(tempFrame, 0x07, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x0A, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x12, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x0A, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x07, red, green, blue, duration/6);
    break;
  case 'B':
  case 'C':
  case 'D':
  case 'E':
    tempFrame = addMonoFrame(tempFrame, 0x1F, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x15, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x15, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x15, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x11, red, green, blue, duration/6);
    break;
  case 'F':
  case 'G':
  case 'H':
    tempFrame = addMonoFrame(tempFrame, 0x1F, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x04, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x04, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x04, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x1F, red, green, blue, duration/6);
    break;
  case 'I':
  case 'J':
  case 'K':
  case 'L':
    tempFrame = addMonoFrame(tempFrame, 0x1F, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x01, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x01, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x01, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x01, red, green, blue, duration/6);
    break;
  case 'M':
  case 'N':
  case 'O':
    tempFrame = addMonoFrame(tempFrame, 0x0E, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x11, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x11, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x11, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x0E, red, green, blue, duration/6);
    break;
  case 'P':
  case 'Q':
  case 'R':
  case 'S':
  case 'T':
  case 'U':
  case 'V':
  case 'W':
  case 'X':
  case 'Y':
  case 'Z':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
  case '0':
  case '-':
    tempFrame = addMonoFrame(tempFrame, 0x04, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x04, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x04, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x04, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x04, red, green, blue, duration/6);
    break;
  case '>':
    tempFrame = addMonoFrame(tempFrame, 0x00, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x11, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x0A, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x04, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x00, red, green, blue, duration/6);
    break;
  case '<':
    tempFrame = addMonoFrame(tempFrame, 0x00, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x04, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x0A, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x11, red, green, blue, duration/6);
    tempFrame = addMonoFrame(tempFrame, 0x00, red, green, blue, duration/6);
    break;
  case ' ':
  default:
    //if the character isn't in the font table, basically do nothing
    free(tempFrame);
    return tail;
  }

  return tempFrame;
}

//here we predefine all the animations we will use
//since we are using linked lists, dynamically allocating the animations will be slow and can cause fragmentation
//we use linked lists here to simplify animation building
void defineAnimations() {
  struct Frame *tempRef;
  
  //IDLE ANIMATION
  //flashing yellow
  idleHead = createEmptyFrame();
  idleHead->duration = 100;
  tempRef = addEmptyFrame(idleHead);
  tempRef->duration = 100;
  for(int i=0; i<5; i++) { 
    tempRef->led[i].red = 0x05; 
    tempRef->led[i].green = 0x05;
  }

  //TANK ANIMATION
  //chasing red 
  tankHead = createEmptyFrame();
  tankHead->duration = 100;
  tempRef = tankHead;
  for(int i=0; i<5; i++) {
    tempRef = addEmptyFrame(tempRef);
    tempRef->duration=100;
    tempRef->led[i].red = 0x0A;
  }

  //SPIN ANIMATION
  //giant red diamond
  spinHead = createEmptyFrame();
  spinHead->duration = 18;
  spinHead->led[2].red = 0x0A;
  tempRef = spinHead;
  for(int i=0; i<7; i++) tempRef = addChar(tempRef, '<', 0x05, 0x05, 0, 18);
  tempRef = addChar(tempRef, 'H', 0, 0x0A, 0, 18);
  tempRef = addChar(tempRef, 'A', 0, 0x0A, 0, 18);
  tempRef = addChar(tempRef, 'L', 0, 0x0A, 0, 18);
  tempRef = addChar(tempRef, 'O', 0, 0x0A, 0, 18);
  for(int i=0; i<7; i++) tempRef = addChar(tempRef, '>', 0x05, 0x05, 0, 18);
  tempRef = addEmptyFrame(tempRef);
  tempRef->duration = 18;
  tempRef->led[2].blue = 0x0A;
}

void shiftToLEDs() {
  if(flip) {//if the robot is inverted, map the LEDs the opposite way
    for(int i=0; i<5; i++) {
      strip.setPixelColor(4-i, currentFrame->led[i].green, currentFrame->led[i].red, currentFrame->led[i].blue);
    }
  } else {
    for(int i=0; i<5; i++) {
      strip.setPixelColor(i, currentFrame->led[i].green, currentFrame->led[i].red, currentFrame->led[i].blue);
    }
  }

  strip.show();
}

void runDynamicAnimation() {
  //run up the linked list to find the correfct frame. If it's different from the current frame, update
  struct Frame *tempFrame = animationHead;
  int count = 0;

  //traverse up the linked list until we find the correct frame for the current heading
  //if we exceed the list we just break and do nothing
  while(true) {
    count += tempFrame->duration;
    if(count > (flip ? angle : 360-angle)) {
      if(tempFrame != currentFrame) {
        nextFrameAt = currentFrame->duration + tempFrame->duration;
        currentFrame = tempFrame;
        shiftToLEDs();
      }
      break;
    } else if(tempFrame->next != NULL) {
      tempFrame = tempFrame->next;
    } else {
      break;
    }
  }
}

void runStaticAnimation() {
  if(micros() > nextFrameAt) {//this glitches temporarily at micros() rollover, but its just an animation, sue me
    if(currentFrame->next == NULL) {//if we are at the end of the list, reset
      currentFrame = animationHead;
    } else {
      currentFrame = currentFrame->next;
    }
    nextFrameAt += currentFrame->duration*1000;

    shiftToLEDs();
  }
}

//run this function whenever you change to a different animation
void resetStaticAnimation() {
  currentFrame = animationHead;
  nextFrameAt = currentFrame->duration*1000 + micros();
  shiftToLEDs();
}


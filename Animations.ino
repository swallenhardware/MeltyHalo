//adds a fresh frame to the ned of a list
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

struct Frame * addEmptyFrame(struct Frame *tail) {
  tail->next = createEmptyFrame();
  return tail->next;
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
  spinHead->duration = 45;
  spinHead->led[2].red = 0x0A;
  tempRef = addEmptyFrame(spinHead);
  tempRef->duration = 90;
  tempRef->led[1].red = 0x0A;
  tempRef->led[3].red = 0x0A;
  tempRef = addEmptyFrame(tempRef);
  tempRef->duration = 90;
  tempRef->led[0].red = 0x0A;
  tempRef->led[4].red = 0x0A;
  tempRef = addEmptyFrame(tempRef);
  tempRef->duration = 90;
  tempRef->led[1].red = 0x0A;
  tempRef->led[3].red = 0x0A;
  tempRef = addEmptyFrame(tempRef);
  tempRef->duration = 45;
  tempRef->led[2].red = 0x0A;
}

void shiftToLEDs() {
  for(int i=0; i<5; i++) {
    strip.setPixelColor(i, currentFrame->led[i].green, currentFrame->led[i].red, currentFrame->led[i].blue);
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
    if(count > angle) {
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


bool buttonPressed(int btn) {
  int count = 0;
  for(int i=0;i<100;i++) {
    byte value = digitalRead(btn);
    if (value == LOW) count++;
    else count--;
    delay(1);
  } 
  return count > 0;
}

bool buttonReleased(int btn) {
  int count = 0;
  for(int i=0;i<25;i++) {
    byte value = digitalRead(btn);
    if (value == HIGH) count++;
    else count--;
    delay(1);
  } 
  return count > 0;
}


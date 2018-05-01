// An arry that stores bit pattern for 7-segment values
int array[10]={0x01,0x30,0x12,0x06,0x4c,0x24,0x20,0x0f,0x00,0x04};

void setup() {
  Serial.begin(9600);
  // connect D port of arduino to 7-segment
  DDRD = 0xff;
}

void loop() {
  // This code line will read from analogpin A0 and normalise it by 100
  // And write on PORTD with required bit pattern
  PORTD = array[(analogRead(A0)/100)];
}

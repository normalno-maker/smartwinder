#include <TM1637Display.h>
int displayclk = 3;
int displaydio = 2;
int ms1 = 8;
int ms2 = 9;
int ms3 = 10;
int button1=4;
int button2=5;
int button3=6;
int button4=7;
int dir=12;
int stp=11; //DO NOT CHANGE EVER, HARDWARE SPECIFIC
int i = 11; //starting index in shutter speeds array
double shutters[] = {32.0, 16.0, 8.0, 4.0, 2.0, 1.0, 1.0 / 2.0, 1.0 / 4.0, 1.0 / 8.0, 1.0 / 16.0, 1.0 / 32.0, 1.0 / 64.0, 1.0 / 128.0};
int j=2; //starting index in panorama angle aray
double angles[]={120.0,180.0,360.0};
const float steps=400; //(half)steps per 360 degrees
const float alfa=20.0; //horizontal view angle in (half)steps
const float a=2000.0; //steps per second per second
unsigned long int t0; //starting time
TM1637Display display = TM1637Display(displayclk, displaydio);
void setup() {
display.setBrightness(7);
pinMode(button1,INPUT_PULLUP); //final speed
pinMode(button2,INPUT_PULLUP);
pinMode(button3,INPUT_PULLUP);
pinMode(button4,INPUT_PULLUP);
pinMode(ms1,OUTPUT);
pinMode(ms2,OUTPUT);
pinMode(ms3,OUTPUT);
pinMode(dir,OUTPUT);
pinMode(stp,OUTPUT);
digitalWrite(ms1,HIGH);
digitalWrite(ms2,LOW); 
digitalWrite(ms3,LOW);
digitalWrite(dir,LOW);
digitalWrite(stp,LOW);
}

void shoot()
{
  float t=shutters[i]; //shutter speed in seconds
  float v=alfa/t; //final speed
  int n = (int) floor(0.5*v*v/a);
  t0=micros();
  int k=1; //step index during acceleration
  while(k<=n) {
    if(micros()-t0>=((unsigned long) 1000000.0*sqrt(2.0*k/a))) {
      PORTB^=B00001000;
      delayMicroseconds(1);
      PORTB^=B00001000;
      k++;
    }
  }
  t0=micros();
  n=(int) angles[j]*steps/360.0;
  k=0; //step index during constant rotation
  unsigned long pause=(unsigned long) 1000000.0/v;
  while(k<=n) {
    if(micros()-t0>=pause) {
      PORTB^=B00001000;
      delayMicroseconds(1);
      PORTB^=B00001000;
      k++;
      t0+=pause;
    }
  }
  
}
void loop() {
while(1){
  if(shutters[i]==floor(shutters[i])) display.showNumberDecEx((int) shutters[i],0b00000000,true);
  else display.showNumberDecEx((int) (1.0/shutters[i]),0b00000000,false);
  if (digitalRead(button1)==LOW) {i=(i+12)%13;delay(200);}
  if (digitalRead(button2)==LOW) {i=(i+1)%13;delay(200);}
  if (digitalRead(button3)==LOW) shoot();
  if (digitalRead(button4)==LOW) {delay(200);break;}
}
while(1){
  display.showNumberDecEx((int) angles[j]);
  if (digitalRead(button1)==LOW) {j=(j+2)%3;delay(200);}
  if (digitalRead(button2)==LOW) {j=(j+1)%3;delay(200);}
  if (digitalRead(button3)==LOW) shoot();
  if (digitalRead(button4)==LOW) {delay(200);break;}
}
}

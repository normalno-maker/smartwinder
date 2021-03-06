#include <TM1637Display.h>
byte enable=13;
byte displayclk = 3;
byte displaydio = 2;
byte ms1 = 8;
byte ms2 = 9;
byte ms3 = 10;
byte button1=4;
byte button2=5;
byte button3=6;
byte button4=7;
byte dir=12;
byte stp=11; //DO NOT CHANGE EVER, HARDWARE SPECIFIC
unsigned int i = 11; //starting index in shutter speeds array
float shutters[] = {32.0, 16.0, 8.0, 4.0, 2.0, 1.0, 1.0 / 2.0, 1.0 / 4.0, 1.0 / 8.0, 1.0 / 16.0, 1.0 / 32.0, 1.0 / 64.0, 1.0 / 128.0,1.0/256.0};
unsigned int j=4; //starting index in panorama angle aray
unsigned int shsiz=sizeof(shutters)/sizeof(shutters[0]); 
float angles[]={60,90,120.0,180.0,360.0};
unsigned int ansiz=sizeof(angles)/sizeof(angles[0]);
float steps; //(micro)steps per 360 degrees
const float fov=4.0; //horizontal view angle in degrees
float a; //acceleration (micro)steps per second per second
float ms; //microstep divider
float alfa; //field of view in (micro)steps
unsigned long int t0; //starting time
float th1=1.0/64.0; //longest shutter for using quartersteps
float th2=1.0/32.0;  //longest shutter for using eigthsteps
float th3=1.0/16.0;      //longest shuter for using sixteenthsteps
float maxaccel=4000.0; //maximal acceleration in steps, correct values for microstepping are calculated from here
TM1637Display display = TM1637Display(displayclk, displaydio);
void setup() {
pinMode(enable,OUTPUT);
digitalWrite(enable,HIGH);
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
void microstepping()
{
  if(shutters[i]>=th3) { 
    digitalWrite(ms1,HIGH);
    digitalWrite(ms2,HIGH); 
    digitalWrite(ms3,HIGH);
    ms=16.0;
    }
  else if(shutters[i]>=th2) { 
    digitalWrite(ms1,HIGH);
    digitalWrite(ms2,HIGH); 
    digitalWrite(ms3,LOW);
    ms=8.0;
    }
  else if(shutters[i]>=th1) {
    digitalWrite(ms1,LOW);
    digitalWrite(ms2,HIGH); 
    digitalWrite(ms3,LOW);
    ms=4.0;
    }
  else {
    digitalWrite(ms1,HIGH);
    digitalWrite(ms2,LOW); 
    digitalWrite(ms3,LOW);
    ms=2.0;
    }
  steps=200.0*ms; //correcting steps per rotation
  a=maxaccel*ms;
  alfa=fov*ms;
}
void microstepping_debug(int m)
{
  if(m==16) { //default slow speed
    digitalWrite(ms1,HIGH);
    digitalWrite(ms2,HIGH); 
    digitalWrite(ms3,HIGH);
    ms=16.0;
    }
  else if(m==8) { 
    digitalWrite(ms1,HIGH);
    digitalWrite(ms2,HIGH); 
    digitalWrite(ms3,LOW);
    ms=8.0;
    }
  else if(m==4) {
    digitalWrite(ms1,LOW);
    digitalWrite(ms2,HIGH); 
    digitalWrite(ms3,LOW);
    ms=4.0;
    }
  else {
    digitalWrite(ms1,HIGH);
    digitalWrite(ms2,LOW); 
    digitalWrite(ms3,LOW);
    ms=2.0;
    }
  steps=200.0*ms; //correcting steps per rotation
  a=maxaccel*ms;
  alfa=fov*ms;
}

void shoot()
{
  digitalWrite(enable,LOW); //enable motor
  float t=shutters[i]; //shutter speed in seconds
  float v=alfa/t; //final speed
  //ramp up:
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
    //constant speed:
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
  //slow down:
  n = (int) floor(0.5*v*v/a);
  t0=micros();
  k=1; //step index during deceleration
  while(k<=n) {
    if(micros()-t0>=((unsigned long) 1000000.0*((v-sqrt(v*v-2.0*k*a))/a))) {
      PORTB^=B00001000;
      delayMicroseconds(1);
      PORTB^=B00001000;
      k++;
    }
  }
  digitalWrite(enable,HIGH); //disable motor
}
void debugloop(){
  i=3;
  j=0;
  while(1){
    if(shutters[i]==floor(shutters[i])) display.showNumberDecEx((int) shutters[i],0b00000000,true);
    else display.showNumberDecEx((int) (1.0/shutters[i]),0b00000000,false);//visualizing shutterspeed in seconds and fractions differently
    delay(1000);
    display.showNumberDecEx(2,0b00000000,true);
    microstepping_debug(2);
    shoot();
    delay(1000);
    display.showNumberDecEx(4,0b00000000,true);
    microstepping_debug(4);
    shoot();
    delay(1000);
    display.showNumberDecEx(8,0b00000000,true);
    microstepping_debug(8);
    shoot();
    delay(1000);
    display.showNumberDecEx(16,0b00000000,true);
    microstepping_debug(16);
    shoot();
    delay(1000);
  }
}
void loop() {
//debugloop(); //uncomment to try same shutter speed with different microstepping
while(1){
  if(shutters[i]==floor(shutters[i])) display.showNumberDecEx((int) shutters[i],0b00000000,true);
  else display.showNumberDecEx((int) (1.0/shutters[i]),0b00000000,false);//visualizing shutterspeed in seconds and fractions differently
  if (digitalRead(button2)==LOW) {i=(i+shsiz-1)%shsiz;delay(200);}
  if (digitalRead(button1)==LOW) {i=(i+1)%shsiz;delay(200);}
  if (digitalRead(button4)==LOW) {microstepping();shoot();}
  if (digitalRead(button3)==LOW) {delay(200);break;}
}
while(1){
  display.showNumberDecEx((int) angles[j]);
  if (digitalRead(button2)==LOW) {j=(j+ansiz-1)%ansiz;delay(200);}
  if (digitalRead(button1)==LOW) {j=(j+1)%ansiz;delay(200);}
  if (digitalRead(button4)==LOW) {microstepping();shoot();}
  if (digitalRead(button3)==LOW) {delay(200);break;}
}
}

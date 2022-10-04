#include<stdlib.h>
#define leds 4
#define buttons 4
#define L_ON 10
#define T_ON 11
#define T1 2
#define T2 3
#define T3 4
#define T4 5
#define L1 T1+leds
#define L2 T2+leds
#define L3 T3+leds
#define L4 T4+leds
int counter;
int roundabout;
void setup() {
  Serial.begin(9600);
  pinInitializing();
  counter = 0;
  roundabout = 1;
}

void loop() {
  test();
  //pattern();
}
void test(){
  Serial.println(counter);
  counter=counter+1;
  counter = counter % (leds+1);
  delay(200);
  digitalWrite(counter+leds+2, HIGH);
  delay(100);
  digitalWrite(counter+leds+2, LOW);
}

void pinInitializing(){
  int i = L1;
  for(;i<leds+1;i++){
    pinMode(i,OUTPUT);
  }
  i = T1;
  for(;i<buttons;i++){
    pinMode(i,INPUT);
  }
}
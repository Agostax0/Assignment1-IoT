#include<stdlib.h>
int score;
int Factor;
int* pattern;
#define L1 2
#define L2 3
#define L3 4
#define L4 5

#define Ls 6

#define Pot 7

#define T1 8
#define T2 9
#define T3 10
#define T4 11
//int counter;
//int roundabout;
void setup() {
  Serial.begin(9600);
  pinMode(L1,OUTPUT);
  pinMode(L2,OUTPUT);
  pinMode(L3,OUTPUT);
  pinMode(L4,OUTPUT);
  pinMode(Ls,OUTPUT);
  score = 0;
  Factor = 1;
  pattern = (int *)malloc(sizeof(int) * Factor); 
  //counter = 0;
  //roundabout = 1;
}

void loop() {
  //test();
  pattern();
}
void pattern(){
  int value = random(4);
  //Serial.println(value);
  digitalWrite(value+2, HIGH);
  delay(200);
  digitalWrite(value+2, LOW);
}
/*void test(){
  counter=counter+roundabout;
  counter = counter % 5;
  delay(200);
  digitalWrite(counter + 2, HIGH);
  delay(50);
  digitalWrite(counter + 2, LOW);
  if(counter == 0 || counter == 4){
    roundabout = roundabout * (-1);
  }
}*/
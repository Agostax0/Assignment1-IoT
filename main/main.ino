#include<stdlib.h>
#include<avr/sleep.h>


#define T1 2
#define T2 T1+1
#define T3 T2+1
#define T4 T3+1

#define L1 6
#define L2 L1+1
#define L3 L2+1
#define L4 L3+1

#define L_ON 10
#define POT A0

int factor;
bool generatedPattern;
bool gameStart;
int score;
int penalty;
int *pattern;
int brightness;
int fadeAmount;
void initializingVariables(){
  //factor = ((int)(analogRead(POT)/256))+1;
  //Serial.println(factor);
  factor = 4;
  pattern = (int*) malloc(sizeof(int)*factor);
  generatedPattern = false;
  gameStart = false;
  score = 0;
  penalty = 0;
  brightness = 0;
  fadeAmount = 15;
}
void setup()
{
  Serial.begin(9600);
  int i;
  for(i=L1;i<L1+4;i++){
    pinMode(i,OUTPUT);
  }
  for(i=T1;i<T1+4;i++){
    pinMode(i,INPUT);
  }
  pinMode(L_ON,OUTPUT);

  initializingVariables();
  Serial.println("Welcome to the Catch the Led Pattern Game. Press Key T1 to Start");
  factor = 4;
}


void loop()
{ 
  if(!gameStart){
    do{
      waitingGameStart();
    }while(digitalRead(T1)==LOW);
    gameStart = true;
    digitalWrite(L_ON,LOW);     
    Serial.println("Go!"); 
  }
  

  if(generatedPattern == false && gameStart == true){
    generatePattern();
    Serial.println("displaying pattern");
    displayPattern();
  }
}

void generatePattern(){
  int i;
  Serial.println("values:");
  for(i=0;i<factor*4;i++){
    int value = ((int)random(4));
    Serial.print(value);
    Serial.print(" ");    
    pattern[i]=value;
  }
  generatedPattern = true;
  Serial.println();
}

void displayPattern(){
  int i=0;
  for(;i<factor*4;i++){
    digitalWrite(pattern[i]+L1,HIGH);
    delay(500);
    digitalWrite(pattern[i]+L1,LOW);
    delay(100);
  }
}

void waitingGameStart(){
  brightness = brightness + fadeAmount;
  if(brightness == 0 || brightness == 255){
    fadeAmount = fadeAmount * (-1);        
  }
  analogWrite(L_ON,brightness);
  delay(50);
}


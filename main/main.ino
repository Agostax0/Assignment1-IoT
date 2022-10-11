#include<stdlib.h>
#include<avr/sleep.h>
#include<Bounce2.h>

#define n_buttons 4
#define n_leds 4

#define T1 3
#define T2 T1+1
#define T3 T2+1
#define T4 T3+1

#define L1 7
#define L2 L1+1
#define L3 L2+1
#define L4 L3+1

#define L_ON 11
#define POT A0

int Time1;
int Time2;
int Time3;

int factor;
bool gameStart;
bool generatedPattern;
bool guessingPattern;
int score = 0;
int penalty;
int pattern[4];
int buttonPressed[4];
int brightness;
int fadeAmount;
int patternCounter;

Bounce buttons[4];

unsigned long time0;

void initializingVariables(){
  factor = (analogRead(POT)/256)+1;
  
  Time1 = (1/factor)*(10*1000);
  Time2 = (1/factor)*(10*700);
  Time3 = (1/factor)*(10*500); 
   
  patternCounter=0;
  gameStart = false;
  generatedPattern = false;
  guessingPattern = false;

  penalty = 0;
  brightness = 0;
  fadeAmount = 15;

  Serial.println("Welcome to the Catch the Led Pattern Game. Press Key T1 to Start");
}
void setup()
{
  Serial.begin(9600);
  int i;
  for(i=L1;i<L1+n_leds;i++){
    pinMode(i,OUTPUT);
  }
  for(i=T1;i<T1+n_buttons;i++){
    pinMode(i,INPUT);
  }

  for(i = 0; i < 4; i++){
    buttons[i]=Bounce();
    buttons[i].attach(T1+i,INPUT);
    buttons[i].interval(25);    
  }

  pinMode(L_ON,OUTPUT);
  initializingVariables();
  
}

void loop()
{ 
  if(!gameStart){
    time0 = millis();
    do{
      buttons[1].update();
      waitingGameStart();
    }while(buttons[1].read()==LOW && millis()-time0<(10*1000));
    if(millis()-time0<(10*1000)){
      gameStart = true;
      digitalWrite(L_ON,LOW);     
      Serial.println("Go!");    
    }
    else{
      wakeUp();        
    }
  }
  

  if(!generatedPattern && gameStart){
    generatePattern();
    generatedPattern = true;
    Serial.println("displaying pattern");
    displayPattern();
    //this reset the timer for guessing the pattern
    time0 = millis();
  }


  if(generatedPattern && !guessingPattern){
    //delay(Time2);
    //Serial.println("Time to guess!");
    guessPattern();

  }  
}

void generatePattern(){
  int i,j;
  Serial.println("values:");
  for(i=0;i<4;i++){
    for(j = 0; j < 4; j++){
      buttons[j].update();
      if(buttons[j].rose()){
        assignPenalty();        
      }      
    }
    int value = ((int)random(n_leds));
    Serial.print(value); 
    Serial.print("="); 
    pattern[i]=value;
    Serial.print(pattern[i]); 
    Serial.print(" ");    
  }
  Serial.println();
}

void displayPattern(){
  int i=0;
  for(;i<4;i++){
    //L1 being the offset from pin 0
    digitalWrite(pattern[i]+L1,HIGH);
    delay(Time1);
    digitalWrite(pattern[i]+L1,LOW);
    //delay for visibility of same consecutive led
    delay(80);
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
void guessPattern(){
  attachInterrupt(0,pinPolling,RISING);
  //patternCounter = (factor*n_leds);
  //buttonPressed = pattern;
  if(patternCounter == (4) /*|| timeNow-time0 > Time3 */ ){
    detachInterrupt(0);
    //Serial.println("pattern inputted was: ");
    int i;
    for(i = 0; i < 4;i++){
      Serial.print(buttonPressed[i]);
      Serial.print(" ");
    } 
    Serial.println();
    scoring();
  }  
}

void pinPolling(){
  int i;
  for(i = T1; i < T1+n_buttons ; i++){
    if(digitalRead(i)==HIGH){
      //There shouldn't be the need for this since the Diods act as a block for multiple button pressing
      //making it so that no more than 1 interrupt can be called simultaneously
      noInterrupts();
      buttonPressed[patternCounter]=(i-T1);
      Serial.print("you pressed ");
      Serial.println(buttonPressed[patternCounter]);      
      patternCounter++;
      Serial.print((factor*n_leds)-patternCounter);
      Serial.println(" remaining");
      interrupts();
      detachInterrupt(0);
      delay(100);
      attachInterrupt(0,pinPolling,RISING);
    }
  }
  delay(50);
}

void scoring(){
  bool success = true;
  int i;
  for(i = 0; i < 4; i++){
    Serial.print(pattern[i]);
    Serial.print("\t");
    Serial.print(buttonPressed[i]);
    Serial.print("\t");
    Serial.print(pattern[i]==buttonPressed[i]);
    Serial.println();
    if(pattern[i]!=buttonPressed[i]){
      success = false;       
    }    
  }
  if(success){
    score++;
    Serial.print("New point! Score: ");
    Serial.println(score);
    initializingVariables();
  }
  else{
    assignPenalty();
  }
  
}

void wakeUp(){

  assignVariables();
}

void assignPenalty(){
  penalty++;
  if(penalty >= 3){
    Serial.print("Game Over. Final Score: ");
    Serial.println(score);
    delay(10*1000);
    initializingVariables();
  }
  else{
    digitalWrite(L_ON,HIGH);
    Serial.println("Penalty!");
    Serial.print("Current penalties: ");
    Serial.println(penalty);
    delay(1000);
    digitalWrite(L_ON,LOW);
  }
}
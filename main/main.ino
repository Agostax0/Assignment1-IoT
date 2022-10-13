#include<stdlib.h>
#include<avr/sleep.h>
#include<math.h>
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
enum state {
  START,
  GENERATING,
  DISPLAYING,
  POLLING,
  CHECKING,
  SCORING,
};
int Time1;
int Time2;
int Time3;

int factor;
int game_state;
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
  
  Time1 = (1000);
  
  Time2 = (1/factor)*(10*700);
  
  Time3 = (1/sqrt(factor))*(10*500) * 4; 
   
  patternCounter=0;
  game_state = START;

  penalty = 0;
  brightness = 0;
  fadeAmount = 15;

  int i;
  for(i=0;i<n_buttons;i++){
    buttonPressed[i]=-1;
  }

  Serial.println("Welcome to the Catch the Led Pattern Game. Press Key T1 to Start");
}
void setup()
{
  Serial.begin(9600);
  int i;
  for(i=L1;i<L1+n_leds;i++){
    pinMode(i,OUTPUT);
  }

  for(i = 0; i < n_buttons; i++){
    buttons[i]=Bounce();
    buttons[i].attach(T1+i,INPUT);
    buttons[i].interval(25);    
  }

  pinMode(L_ON,OUTPUT);

  
  Serial.println("setup");
  initializingVariables();
  
}

void loop()
{ 
  if(game_state==START){
    time0 = millis();
    do{
      buttons[0].update();
      waitingGameStart();
    }while(buttons[0].read()==LOW && millis()-time0<(10*1000));
    if(millis()-time0<(10*1000)){
      game_state = GENERATING;
      digitalWrite(L_ON,LOW);     
      Serial.println("Go!");    
    }
    else{
      goToSleep();        
    }
  }
  

  if(game_state == GENERATING){
    generatePattern();
    Serial.println("displaying pattern");

    
  }

  if(game_state == DISPLAYING){
    displayPattern();
    //this reset the timer for guessing the pattern
    time0 = millis();
  }


  if(game_state == POLLING){
    //delay(Time2);
    //Serial.println("Time to guess!");
    guessPattern();

  }  

  if(game_state == CHECKING){

    
  }

  if(game_state == SCORING){
    
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
    pattern[i]=value;
    Serial.print(pattern[i]); 
    Serial.print(" ");    
  }
  Serial.println();
}

void displayPattern(){
  int i,j;
  for(i=0;i<4;i++){
    //L1 being the offset from pin 0
    for(j = 0; j < 4; j++){
      buttons[j].update();
      if(buttons[j].rose()){
        assignPenalty();        
      }      
    }
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
  if(patternCounter == (4) /*|| timeNow-time0 > Time3 */ ){
    
    Serial.println("pattern inputted was: ");
    int i;
    for(i = 0; i < 4;i++){
      Serial.print(buttonPressed[i]);
      Serial.print(" ");
    } 
    Serial.println();
    scoring();
  }
  else{
    Serial.print("polling");
    pinPolling();  
  }
}

void pinPolling(){
  int i;
  for(i = T1; i < T1+n_buttons ; i++){
    buttons[i].update();
    if(buttons[i].rose()){
      buttonPressed[patternCounter]=(i-T1);
      Serial.print("you pressed ");
      Serial.println(buttonPressed[patternCounter]);      
      patternCounter++;
      Serial.print((factor*n_leds)-patternCounter);
      Serial.println(" remaining");
    }
  }
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
    if(pattern[i]!=buttonPressed[i] && buttonPressed[i] != -1){
      success = false;       
    }    
  }
  if(success){
    score++;
    Serial.print("New point! Score: ");
    Serial.println(score);
    Serial.println("success");
    initializingVariables();
  }
  else{
    assignPenalty();
  }
  
}
void wakeUpNow(){}
void goToSleep(){
  Serial.println("Going to Sleep...");
  Serial.flush();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();

  attachInterrupt(1,wakeUpNow,RISING);

  sleep_mode();

  sleep_disable();

  detachInterrupt(1);

  Serial.println("Woke up..");
  Serial.flush();
}

void assignPenalty(){
  penalty++;
  if(penalty >= 3){
    Serial.print("Game Over. Final Score: ");
    Serial.println(score);
    delay(10*1000);
    Serial.println("3 penalties");
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

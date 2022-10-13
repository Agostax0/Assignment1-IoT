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
};
int Time1;
int Time2;
int Time3;

int factor;
int game_state;
int score = 0;
bool penalty_flag;
int penalty;
int pattern[4];
int buttonPressed[4];
int brightness;
int fadeAmount;
int patternCounter;

int counter = 0;

Bounce buttons[4];

unsigned long time0;


void setup()
{
  Serial.begin(9600);
  randomSeed(analogRead(5));
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
      game_state++;
      digitalWrite(L_ON,LOW);     
      Serial.println("Go!"); 
      Serial.println("Proceding to GENERATING PATTERN");
      Serial.flush();  
      
    }
    else{
      goToSleep();        
    }
  }
  

  if(game_state == GENERATING){
    if(counter < 4){
      counter++;
      i_generatePattern();
    }
    else{
      Serial.println("Proceding to DISPLAYING PATTERN");
      Serial.flush(); 
      counter = 0;
      game_state++;
      //millis for Time1 
      time0 = millis();
    }
  }

  if(game_state == DISPLAYING){
    if(counter < 4  || ((millis()-time0)<Time1)){
      counter++;
      i_displayPattern();
    }
    else{
      for(int i = 0; i < 4 ; i++){
        digitalWrite(L1+i, LOW);
      }
      Serial.println("Proceding to POLLING");
      Serial.flush(); 
      counter = 0;
      //game_state++;
      //Proceding to Guessing
      //time0 = millis();
      delay(20000);
      game_state = START;
    }
    
  }


  /*if(game_state == POLLING){
    if(patternCounter != 4){
      counter++;
      counter = counter % 4;
      i_guessPattern();
    }
    else{
      patternCounter = 0;
      Serial.println("Proceding to SCORING");
      Serial.flush(); 
      counter = 0;
      game_state++;
    }
  }  

  if(game_state == CHECKING){
    if(counter < 4){
      counter++;
      i_scoring();
    }
    else{
      if(penalty_flag){
        
      }
      Serial.println("Game Finished");
      Serial.println("Proceding to GENERATING PATTERN");
      Serial.flush(); 
      counter = 0;
      game_state = START;
      //initializingVariables();      
    }

  }*/
}
void initializingVariables(){
  factor = (analogRead(POT)/256)+1;
  
  Time1 = (10000);
  
  Time2 = (1/factor)*(10*700);
  
  Time3 = (1/sqrt(factor))*(10*500) * 4; 
   
  patternCounter=0;
  game_state = START;

  penalty = 0;
  penalty_flag = false;
  
  brightness = 0;
  fadeAmount = 15;

  Serial.println("Welcome to the Catch the Led Pattern Game. Press Key T1 to Start");
}

void waitingGameStart(){
  brightness = brightness + fadeAmount;
  if(brightness == 0 || brightness == 255){
    fadeAmount = fadeAmount * (-1);        
  }
  analogWrite(L_ON,brightness);
  delay(50);
}

void i_generatePattern(){
  for(int j = 0; j < 4; j++){
      buttons[j].update();
      if(buttons[j].rose()){
        assignPenalty();        
      }      
  }
  int on = ((int)random(2));
  pattern[counter]=on;
  if(counter == 3){
    if(pattern[0] == 0 && pattern[1] == 0 && pattern[2] == 0){
      pattern[3] = 1;
    }
  }
  Serial.print(pattern[counter]); 
  Serial.print(" ");  
}

void i_displayPattern(){
  for(int j = 0; j < 4; j++){
      buttons[j].update();
      if(buttons[j].rose()){
        assignPenalty();        
      }      
  }
  (pattern[counter]) ? digitalWrite(counter+L1,HIGH) : delay(1);  
  //delay for visibility of same consecutive led
}

void i_guessPattern(){
  buttons[counter].update();
  if(buttons[counter].rose()){
    buttonPressed[patternCounter] = counter;
    patternCounter++;
  }
}

void i_scoring(){
  if(pattern[counter] != buttonPressed[counter] && !penalty_flag){
    penalty_flag = true;
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
    //initializingVariables();
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

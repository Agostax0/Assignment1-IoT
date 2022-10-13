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

unsigned long Time1;
/*Time for leds to stay on*/
unsigned long Time2;
/*Time to recreate the configuration*/
unsigned long Time3;

int factor = 1;
int game_state;
int score = 0;
int penalty;
int pattern[4];
int pattern_length;
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
  initializeVariables();
        
}

void loop()
{ 
  if(game_state == START){
    buttons[0].update();
    if(buttons[0].read()==LOW && millis()-time0<(10*1000)){
      waitingGameStart();     
    }else{
      if(millis()-time0<(10*1000)){
        game_state++;
        Serial.println("Proceding to GENERATING PATTERN");
        digitalWrite(L_ON,LOW); 
        time0 = millis();
        counter = 0;
        Serial.flush();  
      }else{
        digitalWrite(L_ON,LOW);
        goToSleep();
        time0 = millis();        
      }
    }
  }
  

  if(game_state == GENERATING){
    if(millis()-time0 < Time1){
      digitalWrite(counter,LOW);
      counter = (counter+1) % 4;
    }else{
      if(counter < 4){
        counter++;
        i_generatePattern();
      }else{
        //TEST
        for(int i = 0; i < n_leds; i++){
          Serial.print(pattern[i]);
          Serial.print(" ");
        }                
        Serial.println();

        Serial.println("Proceding to DISPLAYING PATTERN");
        Serial.flush(); 
        counter = 0;
        game_state++;
        //millis for Time2 
        time0 = millis();
      }      
    }
    
  }

  if(game_state == DISPLAYING){
    if(counter < 4  || ((millis()-time0)<Time2)){
      i_displayPattern();
      counter++;
    }else{
      for(int i = 0; i < 4 ; i++){
        digitalWrite(L1+i, LOW);
      }
      Serial.println("Proceding to POLLING");
      Serial.flush(); 
      counter = 0;
      game_state++;
      //Proceding to Guessing
      time0 = millis();
    }
    
  }


  if(game_state == POLLING){
    
    if(patternCounter != pattern_length && (millis()-time0<Time3)){
      i_polling();
      counter = (counter+1) % n_buttons;
    }else{
      patternCounter = 0;
      Serial.println("Proceding to SCORING");
      Serial.flush(); 
      counter = 0;
      game_state++;
    }
  }  

  if(game_state == CHECKING){
    bool res_flag = true;
    if(counter < 4 && res_flag){
      res_flag = i_scoring();
      counter++;
    }else{
      if(res_flag){
        //Score
        score++;
        Serial.print("New point! Score: ");
        Serial.println(score);
        Serial.flush();
        Serial.println("Increasing Difficulty");
        Serial.flush();
        factor++;
        initializeVariables();
      }else{
        //Penalty
        penalty++;
        if(penalty >= 3){
          Serial.print("Game Over. Final Score: ");
          Serial.println(score);
          delay(10*1000);
          initializeVariables();
        }else{
          digitalWrite(L_ON,HIGH);
          Serial.println("Penalty");          
          Serial.flush();
          delay(1000);
          digitalWrite(L_ON,LOW);
        }
      }
    }

  }
}
void initializeVariables(){
  int L = (analogRead(POT)/256)+1;
  
  int difficulty = factor * L;
  
  Time1 = (int)random(2000);
  
  Time2 = (1/factor)*(10*700);
  
  Time3 = (1/sqrt(factor)) * (10*500) * 4; 
   
  patternCounter = 0;
  pattern_length = 0;
  game_state = START;

  penalty = 0;
  
  brightness = 0;
  fadeAmount = 15;

  Serial.println("Welcome to the Catch the Led Pattern Game. Press Key T1 to Start");

  for(int i = 0; i < n_buttons ; i++){
    buttonPressed[i] = 0;
  }

  time0 = millis();
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
  Serial.flush();
  for(int j = 0; j < 4; j++){
      buttons[j].update();
      if(buttons[j].rose()){
        penalty++;       
      }      
  }
  int on = ((int)random(2));
  pattern_length = (on) ? pattern_length + 1 : pattern_length;
  pattern[counter]=on;
  if(counter == 4 && pattern[0]==0 && pattern[1]==0 && pattern[2]==0 && pattern[3]==0){
    int anti_all_zeros_scenario = (int)random(4);
    pattern[anti_all_zeros_scenario] = 1;
  }
}

void i_displayPattern(){
  for(int j = 0; j < 4; j++){
      buttons[j].update();
      if(buttons[j].rose()){
        penalty++;        
      }      
  }
  (pattern[counter]) ? digitalWrite(counter+L1,HIGH) : delay(1);  
}

void i_polling(){
  //This stop inputting more than the needed inputs.
  if(patternCounter != n_buttons){
    buttons[counter].update();
    if(buttons[counter].rose()){
      buttonPressed[counter] = 1;
      patternCounter++;
    }
  }
}

bool i_scoring(){
  if(pattern[counter]!=buttonPressed[counter]){
    return false;    
  }  
  return true;
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
  
  delay(100);

  Serial.println("Welcome to the Catch the Led Pattern Game. Press Key T1 to Start");
}
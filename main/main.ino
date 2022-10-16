#include <avr/sleep.h>
#include <EnableInterrupt.h>
#include <math.h>
#include <stdlib.h>
#include <Bounce2.h>

#define n_buttons 4
#define n_leds 4

#define T1 3
#define T2 T1 + 1
#define T3 T2 + 1
#define T4 T3 + 1

#define L1 7
#define L2 L1 + 1
#define L3 L2 + 1
#define L4 L3 + 1

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
int L;
int game_state;
int score = 0;
int penalty;
//TODO implement weighted led and button pattern style
/*where each led is weighted like 1 2 4 8
  and only the right button press allows it*/
int pattern[4];
int pattern_length;
int buttonPressed[4];
int brightness;
int fadeAmount;
int patternCounter;

int counter = 0;

Bounce buttons[4];

unsigned long time0;

void setup() {
  Serial.begin(9600);
  int seed = analogRead(5);
  randomSeed(seed);
  int i;
  for (i = L1; i < L1 + n_leds; i++) {
    pinMode(i, OUTPUT);
  }

  for (i = 0; i < n_buttons; i++) {
    buttons[i] = Bounce();
    buttons[i].attach(T1 + i, INPUT);
    buttons[i].interval(25);
  }

  pinMode(L_ON, OUTPUT);

  
  initializeVariables();
}

void loop() {
  if(millis()==0){
    delay(150);

    Serial.println("Welcome to the Catch the Led Pattern Game. Press Key T1 to Start");
  }
  if (game_state == START) {
    buttons[0].update();
    if (buttons[0].read() == LOW && millis() - time0 < (10 * 1000)) {
      waitingGameStart();
    } else {
      if (millis() - time0 < (10 * 1000)) {
        game_state = GENERATING;
        //Serial.println("Proceding to GENERATING PATTERN");
        digitalWrite(L_ON, LOW);
        time0 = millis();
        counter = 0;
        Serial.flush();
      } else {
        digitalWrite(L_ON, LOW);
        goToSleep();
        time0 = millis();
      }
    }
  }


  if (game_state == GENERATING) {
    if (millis() - time0 < Time1) {
      digitalWrite(counter, LOW);
      counter = (counter + 1) % 4;
    } else {
      if (counter < 4) {

        i_generatePattern();
        counter++;

      } else {
        pattern_length = 0;
        for (int i = 0; i < n_leds; i++) {
          if (pattern[i]) {
            pattern_length++;
          }
        }
        if (pattern[0] == 0 && pattern[1] == 0 && pattern[2] == 0 && pattern[3] == 0) {
          //Serial.println("Anti all zeros scenario");
          int anti_all_zeros_scenario = (int)random(4);
          pattern[anti_all_zeros_scenario] = 1;
          pattern_length = 1;
        }
        //Serial.println("Proceding to DISPLAYING PATTERN");
        //Serial.flush();
        counter = 0;
        game_state = DISPLAYING;
        //printState();
        //millis for Time2
        time0 = millis();
      }
    }
  }

  if (game_state == DISPLAYING) {
    if (((millis() - time0) < Time2)) {
      displayPattern();
    } else {
      for (int i = 0; i < 4; i++) {
        digitalWrite(L1 + i, LOW);
      }
      //Serial.println("Proceding to POLLING");
      //Serial.flush();
      counter = 0;
      game_state = POLLING;
      patternCounter = 0;
      //Proceding to Guessing
      time0 = millis();
    }
  }


  if (game_state == POLLING) {

    if (patternCounter != pattern_length && (millis() - time0 < Time3)) {
      i_polling();
      counter = (counter + 1) % n_buttons;
    } else {
      if (millis() - time0 > Time3) {
        Serial.println("Time's up");
      }
      patternCounter = 0;
      //Serial.println("Proceding to SCORING");
      Serial.flush();
      counter = 0;
      game_state = CHECKING;

      //printState();
    }
  }

  if (game_state == CHECKING) {
    //no need to check if the pattern is correct if the player racked up 3 or more penalties
    if (penalty < 3 && scoring()) {
      //Score
      score++;
      Serial.print("New point! Score: ");
      Serial.println(score);
      Serial.flush();
      Serial.println("Increasing Difficulty");
      Serial.flush();
      factor++;
      nextRound();
      Serial.println("Next Round Starting...");
      Serial.flush();
    } else {
      //Penalty
      penalty++;
      if (penalty >= 3) {
        Serial.print("Game Over. Final Score: ");
        Serial.flush();
        Serial.println(score);
        Serial.flush();
        delay(10 * 1000);
        initializeVariables();
        Serial.println("Welcome to the Catch the Led Pattern Game. Press Key T1 to Start");
      } else {
        digitalWrite(L_ON, HIGH);
        Serial.println("Penalty");
        Serial.flush();
        Serial.print("Current Penalties: ");
        Serial.flush();
        Serial.println(penalty);
        Serial.flush();
        delay(1000);
        digitalWrite(L_ON, LOW);
        nextRound();
        Serial.println("Next Round Starting...");
        Serial.flush();
      }
    }
    //printState();
  }
}

void waitingGameStart() {
  brightness = brightness + fadeAmount;
  if (brightness <= 0 || brightness >= 255) {
    fadeAmount = fadeAmount * (-1);
  }
  analogWrite(L_ON, brightness);
  delay(50);
}

void i_generatePattern() {
  checkForPenalty();
  int on = ((int)random(2));
  pattern[counter] = on;
}

void displayPattern() {
  checkForPenalty();
  for (int i = 0; i < n_leds; i++) {
    if (pattern[i]) {
      digitalWrite(i + L1, HIGH);
    }
  }
}

void i_polling() {
  //This stop inputting more than the needed inputs.
  if (patternCounter != n_buttons) {
    buttons[counter].update();
    if (buttons[counter].rose()) {
      //Serial.println("You pressed " + (String)counter);
      buttonPressed[counter] = 1;
      patternCounter++;
    }
  }
}

bool scoring() {
  bool result = true;
  for (int i = 0; i < n_buttons; i++) {
    //if the led was on it checks if the corresponding button was pressed
    if (pattern[i] && pattern[i] != buttonPressed[i]) {
      Serial.println("Wrong sequence");
      return false;
    }
  }
  Serial.println("No errors in the sequence");
  return true;
}
void wakeUpNow() {}
void goToSleep() {
  Serial.println("Going to Sleep...");
  Serial.flush();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();

  enableInterrupt(T1, wakeUpNow, CHANGE);
  enableInterrupt(T2, wakeUpNow, CHANGE);
  enableInterrupt(T3, wakeUpNow, CHANGE);
  enableInterrupt(T4, wakeUpNow, CHANGE);

  sleep_mode();

  sleep_disable();

  disableInterrupt(T1);
  disableInterrupt(T2);
  disableInterrupt(T3);
  disableInterrupt(T4);

  Serial.println("Woke up..");
  Serial.flush();

  delay(100);

  Serial.println("Welcome to the Catch the Led Pattern Game. Press Key T1 to Start");
}
void checkForPenalty() {
  for (int j = 0; j < 4; j++) {
    buttons[j].update();
    if (buttons[j].rose()) {
      penalty++;
    }
  }
}
void initializeVariables() {
  penalty = 0;
  score = 0;
  factor = 1;

  brightness = 0;
  fadeAmount = 15;
  nextRound();
  game_state = START;

  
}

void nextRound() {
  L = ((int)(analogRead(POT) / 256)) + 1;
  switch (L) {
    case 1:
      fadeAmount = 4;
      break;
    case 2:
      fadeAmount = 8;
      break;
    case 3:
      fadeAmount = 12;
      break;
    default:
      fadeAmount = 15;
      break;
  }
  randomSeed(random(5000));

  Time1 = (int)random(500, 2000);
  //1.0 is needed to box it to double so that the division doesn't approx to 0

  //As the game goes on Time2 and Time3 can reach 0
  Time2 = 1.0 / sqrt((L * factor)) * (2500);
  Time3 = (1 / sqrt(L * factor)) * (2500 * 4);
  //This prevents any value below doable times
  Time2 = (Time2 > 400) ? Time2 : 400;
  Time3 = (Time3 > 600 * 4) ? Time3 : 600 * 4;

  patternCounter = 0;
  pattern_length = 0;
  game_state = GENERATING;

  for (int i = 0; i < n_buttons; i++) {
    buttonPressed[i] = -1;
    pattern[i] = 0;
  }

  time0 = millis();
}
void printState() {
  Serial.println("----------------------------------------");
  Serial.print("Current Pot: ");
  Serial.println(L);
  Serial.print("Current Factor: ");
  Serial.println(factor);
  Serial.print("Current game_state: ");
  switch (game_state) {
    case START:
      Serial.println("START");
      break;
    case GENERATING:
      Serial.println("GENERATING");
      break;
    case DISPLAYING:
      Serial.println("DISPLAYING");
      break;
    case POLLING:
      Serial.println("POLLING");
      break;
    case CHECKING:
      Serial.println("SCORING");
      break;
    default:
      Serial.println("ERROR");
      break;
  }

  Serial.print("Current score: ");
  Serial.println(score);
  Serial.print("Current Penalties: ");
  Serial.println(penalty);
  Serial.print("Current Pattern Length: ");
  Serial.println(pattern_length);
  Serial.println("Current pattern");
  for (int i = 0; i < n_leds; i++) {
    Serial.print(pattern[i]);
    Serial.print(" ");
  }
  Serial.println();
  Serial.println("Current buttonPressed: ");
  for (int i = 0; i < n_buttons; i++) {
    Serial.print(buttonPressed[i]);
    Serial.print(" ");
  }
  Serial.println();

  Serial.print("Current PatternCounter: ");
  Serial.println(patternCounter);
  Serial.print("Current counter: ");
  Serial.println(counter);

  Serial.print("Current Time1: ");
  Serial.println(Time1);
  Serial.print("Current Time2: ");
  Serial.println(Time2);
  Serial.print("Current Time3: ");
  Serial.println(Time3);

  /*Serial.println("Waiting T1 input to continue post-debug");
  do{
    buttons[0].update();        
  }while(!buttons[0].rose());
  time0 = millis();*/
  Serial.println("----------------------------------------");
}
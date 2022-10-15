#include <avr/sleep.h>
#include <math.h>
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


void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(5));
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


  Serial.println("setup");
  initializeVariables();
}

void loop() {
  if (game_state == START) {
    buttons[0].update();
    if (buttons[0].read() == LOW && millis() - time0 < (10 * 1000)) {
      waitingGameStart();
    } else {
      if (millis() - time0 < (10 * 1000)) {
        game_state=GENERATING;
        Serial.println("Proceding to GENERATING PATTERN");
        digitalWrite(L_ON, LOW);
        time0 = millis();
        counter = 0;
        printState();
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
        Serial.println("Proceding to DISPLAYING PATTERN");
        Serial.flush();
        counter = 0;
        game_state=DISPLAYING;
        printState();
        //millis for Time2
        time0 = millis();
      }
    }
  }

  if (game_state == DISPLAYING) {
    if (counter < 4 || ((millis() - time0) < Time2)) {
      i_displayPattern();
      counter = (counter == 4) ? counter : counter + 1;
    } else {
      for (int i = 0; i < 4; i++) {
        digitalWrite(L1 + i, LOW);
      }
      Serial.println("Proceding to POLLING");
      Serial.flush();
      counter = 0;
      game_state=POLLING;
      printState();
      //Proceding to Guessing
      time0 = millis();
    }
  }


  if (game_state == POLLING) {

    if (patternCounter != pattern_length && (millis() - time0 < Time3)) {
      i_polling();
      counter = (counter + 1) % n_buttons;
    } else {
      patternCounter = 0;
      Serial.println("Proceding to SCORING");
      Serial.flush();
      counter = 0;
      game_state=CHECKING;

      //printState();
    }
  }

  if (game_state == CHECKING) {
    if (scoring()) {
      //Score
      score++;
      Serial.print("New point! Score: ");
      Serial.println(score);
      Serial.flush();
      Serial.println("Increasing Difficulty");
      Serial.flush();
      factor++;
      initializeVariables();
    } else {
      //Penalty
      penalty++;
      if (penalty >= 3) {
        Serial.print("Game Over. Final Score: ");
        Serial.println(score);
        delay(10 * 1000);
        initializeVariables();
      } else {
        digitalWrite(L_ON, HIGH);
        Serial.println("Penalty");
        Serial.print("Current Penalties: ");
        Serial.println(penalty);
        Serial.flush();
        delay(1000);
        digitalWrite(L_ON, LOW);
        initializeVariables();
      }
    }
  }
}
void initializeVariables() {
  int L = (analogRead(POT) / 256) + 1;

  int difficulty = factor * L;

  Time1 = (int)random(2000);

  Time2 = (1 / difficulty) * (10 * 700);

  Time3 = (1 / sqrt(difficulty)) * (10 * 500) * 4;

  patternCounter = 0;
  pattern_length = 0;
  game_state = START;

  penalty = 0;

  brightness = 0;
  fadeAmount = 15;

  Serial.println("Welcome to the Catch the Led Pattern Game. Press Key T1 to Start");

  for (int i = 0; i < n_buttons; i++) {
    buttonPressed[i] = -1;
  }

  time0 = millis();
}

void waitingGameStart() {
  brightness = brightness + fadeAmount;
  if (brightness == 0 || brightness == 255) {
    fadeAmount = fadeAmount * (-1);
  }
  analogWrite(L_ON, brightness);
  delay(50);
}

void i_generatePattern() {
  Serial.flush();
  for (int j = 0; j < 4; j++) {
    buttons[j].update();
    if (buttons[j].rose()) {
      penalty++;
    }
  }
  int on = ((int)random(2));
  pattern_length = (on) ? pattern_length + 1 : pattern_length;
  pattern[counter] = on;
  if (pattern[0] == 0 && pattern[1] == 0 && pattern[2] == 0 && pattern[3] == 0) {
    Serial.println("Anti all zeros scenario");
    int anti_all_zeros_scenario = (int)random(4);
    pattern[anti_all_zeros_scenario] = 1;
  }
}

void i_displayPattern() {
  for (int j = 0; j < 4; j++) {
    buttons[j].update();
    if (buttons[j].rose()) {
      penalty++;
    }
  }
  if (counter < 4) {
    (pattern[counter]) ? digitalWrite(counter + L1, HIGH) : delay(1);
  }
}

void i_polling() {
  //This stop inputting more than the needed inputs.
  if (patternCounter != n_buttons) {
    //printState();
    buttons[counter].update();
    if (buttons[counter].rose()) {
      buttonPressed[counter] = 1;
      patternCounter++;
    }
  }
}

bool scoring() {
  printState();
  bool result = true;
  for (int i = 0; i < n_buttons; i++) {
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

  attachInterrupt(1, wakeUpNow, RISING);

  sleep_mode();

  sleep_disable();

  detachInterrupt(1);

  Serial.println("Woke up..");
  Serial.flush();

  delay(100);

  Serial.println("Welcome to the Catch the Led Pattern Game. Press Key T1 to Start");
}

void printState() {
  Serial.print("Current Factor: ");
  Serial.println(factor);
  Serial.print("Current game_state: ");
  Serial.println(game_state);
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

  Serial.println("Waiting T1 input to continue post-debug");
  do{
    buttons[0].update();        
  }while(!buttons[0].rose());
  time0 = millis();
  Serial.println("----------------------------------------");
}
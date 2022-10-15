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
int L;
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

  initializeVariables();
}

void loop() {
  if (game_state == START) {
    buttons[0].update();
    if (buttons[0].read() == LOW && millis() - time0 < (10 * 1000)) {
      waitingGameStart();
    } else {
      if (millis() - time0 < (10 * 1000)) {
        game_state = GENERATING;
        digitalWrite(L_ON, LOW);
        time0 = millis();
        counter = 0;
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
        counter = 0;
        game_state = DISPLAYING;
        //millis for Time2
        time0 = millis();
      }
    }
  }

  if (game_state == DISPLAYING) {
    if (counter < 4 && ((millis() - time0) < Time2)) {
      i_displayPattern();
      counter = (counter + 1) % 4;
    } else {
      for (int i = 0; i < 4; i++) {
        digitalWrite(L1 + i, LOW);
      }
      counter = 0;
      game_state = POLLING;
      time0 = millis();
    }
  }


  if (game_state == POLLING) {

    if (patternCounter != pattern_length && (millis() - time0 < Time3)) {
      i_polling();
      counter = (counter + 1) % n_buttons;
    } else {
      patternCounter = 0;
      counter = 0;
      game_state = CHECKING;
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
      initializeVariables();
    } else {
      //Penalty
      penalty++;
      if (penalty >= 3) {
        Serial.print("Game Over. Final Score: ");
        Serial.println(score);
        delay(10 * 1000);
        penalty = 0;
        score = 0;
        factor = 1;
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
  L = ((int)(analogRead(POT) / 256)) + 1;

  Time1 = (int)random(500, 2000);
  //1.0 is needed to box it to double so that the division doesn't approx to 0

  //As the game goes on Time2 and Time3 can reach 0
  Time2 = ((1.0 / (L * factor)) * (5000));
  Time3 = (1 / sqrt(L * factor)) * (10 * 250) * 4;
  //This prevents any value below 250ms
  Time2 = (Time2 > 250) ? Time2 : 250;
  Time3 = (Time3 > 250) ? Time3 : 250;

  patternCounter = 0;
  pattern_length = 0;
  game_state = START;


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
  if (brightness <= 0 || brightness >= 255) {
    fadeAmount = fadeAmount * (-1);
  }
  analogWrite(L_ON, brightness);
  delay(50);
}

void i_generatePattern() {
  checkForPenalty();
  int on = ((int)random(2));
  pattern_length = (on) ? pattern_length + 1 : pattern_length;
  pattern[counter] = on;
  if (pattern[0] == 0 && pattern[1] == 0 && pattern[2] == 0 && pattern[3] == 0 && counter == n_leds - 1) {
    int anti_all_zeros_scenario = (int)random(4);
    pattern[anti_all_zeros_scenario] = 1;
  }
}

void i_displayPattern() {
  checkForPenalty();
  if (counter < 4) {
    (pattern[counter]) ? digitalWrite(counter + L1, HIGH) : delay(1);
  }
}

void i_polling() {
  //This stop inputting more than the needed inputs.
  if (patternCounter != n_buttons) {
    buttons[counter].update();
    if (buttons[counter].rose()) {
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
      return false;
    }
  }
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
void checkForPenalty() {
  for (int j = 0; j < 4; j++) {
    buttons[j].update();
    if (buttons[j].rose()) {
      penalty++;
    }
  }
}
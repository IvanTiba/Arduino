#include <LiquidCrystal.h>

// Photo Sensors PINS

const int CAR0_SENSOR_PIN = A0;
const int CAR1_SENSOR_PIN = A1;

// Semaphore Leds PINS

const int SEMAPHORE_LED0_PIN = 11;
const int SEMAPHORE_LED1_PIN = 12;
const int SEMAPHORE_LED2_PIN = 13;

// End Race Led PINS

const int END_RACE_LED_PIN = 8;

// Start button PINS

const int START_BUTTON_PIN = 10;

// Race statues

const int STATUS_NO_RACE = 0;
const int STATUS_STARTING_RACE = 1;
const int STATUS_RACE = 2;
const int STATUS_STOP_RACE = 3;

// Cars Name

const String CAR0_ID = "Car0";
const String CAR1_ID = "Car1";

// Laps variables

const int MIN_LAPS = 5;
const int MAX_LAPS = 30;
const int INC_LAPS = 5;

// Results screens

const int RESULT_SCREEN_1 = 1;
const int RESULT_SCREEN_2 = 2;
const int RESULT_SCREEN_3 = 3;

// LCD variable

LiquidCrystal lcd(7,6,2,3,4,5);

// Semaphore values

unsigned long startMillis = 0;

// Cars variables

int car0SensorValue = 0;
int car1SensorValue = 0;

int maxCar0SensorValue = 0;
int maxCar1SensorValue = 0;

unsigned long lastTimeCar0 = 0;
unsigned long lastTimeCar1 = 0;

unsigned long totalTimeCar0 = 0;
unsigned long totalTimeCar1 = 0;

int car0Laps = 0;
int car1Laps = 0;

// Start button state

int startButtonState = 0;

// Number of Laps

int lapsNumber = MIN_LAPS;

// Race Status

int raceStatus = STATUS_NO_RACE;

// Flag end race ok TRUE/FALSE

bool flagEndRace = false;

// Results timer and screen

int screenResult = RESULT_SCREEN_1;
unsigned long showResultsTimer = 0;

// ==============
// Setup function
// ==============
void setup() {
  // Serial setup
  Serial.begin(9600);

  // PINS setup
  pinMode(SEMAPHORE_LED0_PIN,OUTPUT);
  pinMode(SEMAPHORE_LED1_PIN,OUTPUT);
  pinMode(SEMAPHORE_LED2_PIN,OUTPUT);

  pinMode(END_RACE_LED_PIN,OUTPUT);

  pinMode(START_BUTTON_PIN,INPUT);

  // LCD
  lcd.begin(16,2);
  lcd.print("F1 GP Chrono");

  // Race status
  raceStatus = STATUS_NO_RACE;

  // No results
  flagEndRace = false;
}

// ==============
// Loop function
// ==============
void loop() {

  if (raceStatus == STATUS_NO_RACE) {
    noRace();
  }
  
  if (raceStatus == STATUS_STARTING_RACE) {
    startingRace();
  }
  
  if (raceStatus == STATUS_RACE) {
    race();
  }

  if (raceStatus == STATUS_STOP_RACE) {
    stopRace();
  }
}

void noRace() {

  if (raceStatus == STATUS_NO_RACE) {
    
    // 1. Read Start button status

    startButtonState = digitalRead(START_BUTTON_PIN);

    // 2. Start count down!

    if (startButtonState == HIGH) {     
      // Change status
      raceStatus = STATUS_STARTING_RACE;

      // Reset starting millis
      startMillis = millis();

      // Turn off end race led
      setEndRace(false);

      // End race flag to false
      flagEndRace = false;

      // Update LCD
      updateLcdStartingRace();
    } else {
      // Turn off semaphore leds
      setSemaphore(false,false,false);
    }

    updateLcdNoRace();
  }
  
}

void startingRace() {
  
  if (raceStatus == STATUS_STARTING_RACE) {

    // 1. Read Start button status

    startButtonState = digitalRead(START_BUTTON_PIN);

    // 2. If press button, change the number of laps and restart count down

    if (millis()-startMillis > 500) {
      if (startButtonState == HIGH) {
        // Inc Laps     
        lapsNumber = lapsNumber + INC_LAPS;
  
        if (lapsNumber > MAX_LAPS) {
          lapsNumber = MIN_LAPS;
        }
  
        // Reset starting millis
        startMillis = millis();

        // Update LCD
        updateLcdStartingRace();
      }
    }
    
    if (millis()-startMillis > 2000) {
      setSemaphore(true,false,false);
    }

    if (millis()-startMillis > 3000) {
      setSemaphore(true,true,false);
    }

    if (millis()-startMillis > 4000) {
      setSemaphore(true,true,true);
    }

    // START RACE!!!

    if (millis()-startMillis > 5000) {
      setSemaphore(false,false,false);

      // Change status
      raceStatus = STATUS_RACE;
  
      // Reset max photo sensors values
      maxCar0SensorValue = 0;
      maxCar1SensorValue = 0;      

      // Reset last times
      lastTimeCar0 = millis();
      lastTimeCar1 = millis();

      // Reset total times
      totalTimeCar0 = 0;
      totalTimeCar1 = 0;

      // Reset laps
      car0Laps = 0;
      car1Laps = 0;

      // Update LCD
      updateLcdRace();
    }
  }
  
}

void setSemaphore(bool s0, bool s1, bool s2) {
  digitalWrite(SEMAPHORE_LED0_PIN, (s0 ? HIGH : LOW) );
  digitalWrite(SEMAPHORE_LED1_PIN, (s1 ? HIGH : LOW) );
  digitalWrite(SEMAPHORE_LED2_PIN, (s2 ? HIGH : LOW) );
}

void setEndRace(bool v) {
  digitalWrite(END_RACE_LED_PIN, (v ? HIGH : LOW) );
}

void race() {

  if (raceStatus == STATUS_RACE) {

    // 1. Read Start button status
  
    startButtonState = digitalRead(START_BUTTON_PIN);

    if (startButtonState == HIGH) {
      raceStatus = STATUS_STOP_RACE;
      flagEndRace = false;
      updateLcdStopRace();
      return;
    }
    
    // 2. Read sensor values
  
    car0SensorValue = analogRead(CAR0_SENSOR_PIN);
    delay(5);
  
    car1SensorValue = analogRead(CAR1_SENSOR_PIN);
    delay(5);
  
    // 3. Adjust Max values
  
    if (car0SensorValue > maxCar0SensorValue) {
      maxCar0SensorValue = car0SensorValue;
    }
  
    if (car1SensorValue > maxCar1SensorValue) {
      maxCar1SensorValue = car1SensorValue;
    }
  
    // 4. Check Laps
  
    checkLapCar(CAR0_ID, car0SensorValue, maxCar0SensorValue, &lastTimeCar0, &car0Laps, &totalTimeCar0);
    checkLapCar(CAR1_ID, car1SensorValue, maxCar1SensorValue, &lastTimeCar1, &car1Laps, &totalTimeCar1);
  
    // 5. Check end race
  
    checkEndRace();
  }
}

void checkLapCar(String carId, int carSensorValue, int maxCarSensorValue, unsigned long *lastTimeCar, int *carLaps, unsigned long *totalTimeCar) {
  if (carSensorValue < (maxCarSensorValue*0.9)) {
    unsigned long currMillis = millis();
    if ((currMillis - *lastTimeCar) > 2000) {
      *carLaps = *carLaps + 1;
      *totalTimeCar = *totalTimeCar + (currMillis - *lastTimeCar);     

      debugRaceValues(carId, *lastTimeCar, *carLaps, carSensorValue, maxCarSensorValue);
      
      *lastTimeCar = currMillis;

      updateLcdRace();
    }
  }
}

void debugRaceValues(String carId, unsigned long lastTimeCar, int carLaps, int carSensorValue, int maxCarSensorValue) {
  Serial.print(carId);
  Serial.print(" - lastTimeCar: ");
  Serial.print(lastTimeCar);
  Serial.print(" carLaps: ");
  Serial.print(carLaps);
  Serial.print(" carSensorValue: ");
  Serial.print(carSensorValue);
  Serial.print(" maxCarSensorValue: ");
  Serial.println(maxCarSensorValue);
}

void checkEndRace() {

  if ((car0Laps >= lapsNumber) || (car1Laps >= lapsNumber)) {
    // Change status
    raceStatus = STATUS_NO_RACE;

    // Turn on end race led
    setEndRace(true);

    // Set end race flag TRUE
    flagEndRace = true;
  }
  
}

void stopRace() {
  if (raceStatus == STATUS_STOP_RACE) {
    
    // 1. Read Start button status
  
    startButtonState = digitalRead(START_BUTTON_PIN);
  
    // 2. Reset status when the button is off!
  
    if (startButtonState == LOW) {     
      raceStatus = STATUS_NO_RACE;
    }
  }
}

void updateLcdNoRace() {
  
  if (raceStatus == STATUS_NO_RACE) {
    
    if ((flagEndRace) && (millis() - showResultsTimer > 2000)) {
    
      lcd.clear();

      if (screenResult == RESULT_SCREEN_1) {
        if ((car0Laps > car1Laps) || (totalTimeCar0 > totalTimeCar1)) {
          lcd.print(CAR0_ID + " win!");
        } else {
          lcd.print(CAR1_ID + " win!");
        } 
      }

      if (screenResult == RESULT_SCREEN_2) {
        lcd.print(CAR0_ID + " laps: " + car0Laps);
        lcd.setCursor(0,1);
        String text = String("Time: ");
        lcd.print(text + (totalTimeCar0/1000));
      }

      if (screenResult == RESULT_SCREEN_3) {
        lcd.print(CAR1_ID + " laps: " + car1Laps);
        lcd.setCursor(0,1);
        String text = String("Time: ");
        lcd.print(text + (totalTimeCar1/1000));
      }

      screenResult = (screenResult == RESULT_SCREEN_3) ? RESULT_SCREEN_1 : (screenResult+1);
      showResultsTimer = millis();
    }
  }
}

void updateLcdStartingRace() {
  if (raceStatus == STATUS_STARTING_RACE) {
    lcd.clear();
    
    lcd.setCursor(0,0);
    lcd.print("Countdown...");
    
    lcd.setCursor(0,1);
    String text = String("Laps: ");
    lcd.print(text + lapsNumber);
  }
}

void updateLcdRace() {
  if (raceStatus == STATUS_RACE) {
    lcd.clear();
    
    lcd.setCursor(0,0);
    lcd.print(CAR0_ID + ": " + car0Laps + "/" + lapsNumber);

    lcd.setCursor(0,1);
    lcd.print(CAR1_ID + ": " + car1Laps + "/" + lapsNumber);
  }
}

void updateLcdStopRace() {
  if (raceStatus == STATUS_STOP_RACE) {
    lcd.clear();
    
    lcd.setCursor(0,0);
    lcd.print("Race stopped!");
  }
}

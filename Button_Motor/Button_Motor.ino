#define LEDR 6
#define LEDB 7
#define BUTTON 9
#define ENABLE 5
#define DIRA 3
#define DIRB 4

#include "LedControl.h"

LedControl lc=LedControl(12,10,11,1);

int buttonState = HIGH;
int motorState = LOW;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 250;
unsigned long motorOnTime = 0;
unsigned long motorDuration = 10000;
int secondsLeft = 0;

const uint64_t IMAGES[] = {
  0x3844444444444438,
  0x3810101010101810,
  0x7c08102040404438,
  0x3844404030404438,
  0x40407c4448506040,
  0x384440403c04047c,
  0x384444443c044438,
  0x080808102040407c,
  0x3844444438444438,
  0x3844407844444438,
  0x4444442810284444
};
const int IMAGES_LEN = sizeof(IMAGES)/8;

void setup() {
  pinMode(LEDR, OUTPUT);
  pinMode(LEDB, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);

  pinMode(ENABLE,OUTPUT);
  pinMode(DIRA,OUTPUT);
  pinMode(DIRB,OUTPUT);

  digitalWrite(LEDR, motorState);
  digitalWrite(LEDB, LOW);

  lastDebounceTime = millis();

  Serial.begin(9600);
  Serial.println("Setup complete");

  lc.shutdown(0,false);
  lc.setIntensity(0,5);
  lc.clearDisplay(0);
}

void displayImage(uint64_t image) {
  for (int i = 0; i < 8; i++) {
    byte row = (image >> i * 8) & 0xFF;
    for (int j = 0; j < 8; j++) {
      lc.setLed(0, i, j, bitRead(row, j));
    }
  }
}

void motorSlowStop() {
  displayImage(IMAGES[0]);
  
  Serial.println("Stopping motor");

  digitalWrite(LEDB, HIGH);
  analogWrite(ENABLE,180);
  delay(1000);
  analogWrite(ENABLE,128);
  delay(1000);
  analogWrite(ENABLE,50);
  delay(1000);
  digitalWrite(ENABLE, LOW);

  digitalWrite(LEDB, LOW);

  lc.clearDisplay(0);
}

void loop() {
  int reading = digitalRead(BUTTON);

  if(motorState == HIGH) {
    Serial.println(motorDuration - (millis() - motorOnTime));
  }
 
  if(motorState == HIGH && (millis() - motorOnTime) > motorDuration) {
    motorSlowStop();
    
    digitalWrite(LEDR, LOW);
    motorState = LOW;
  }

  if(motorState == HIGH && (motorDuration - (millis() - motorOnTime)) < 9000) {
    secondsLeft = (motorDuration - (millis() - motorOnTime)) / 1000;

    if((motorDuration - (millis() - motorOnTime)) % 1000 > 0 && (motorDuration - (millis() - motorOnTime)) >= 0) {
      secondsLeft = secondsLeft + 1;
    }

    displayImage(IMAGES[secondsLeft]);
  }

  if(reading != buttonState && (millis() - lastDebounceTime) > debounceDelay) {
    buttonState = reading;
    lastDebounceTime = millis();

    if(buttonState == HIGH) {
      if(motorState == HIGH) {
        displayImage(IMAGES[10]);
        Serial.println("Stopping motor early");

        motorState = LOW;
        digitalWrite(LEDR, motorState);

        digitalWrite(DIRA, LOW);
        digitalWrite(ENABLE, LOW);
        delay(1000);
        lc.clearDisplay(0);
      } else {
        Serial.println("Starting motor");

        motorState = HIGH;
        digitalWrite(LEDR, motorState);

        digitalWrite(ENABLE, HIGH);
        digitalWrite(DIRA, HIGH);
        digitalWrite(DIRB, LOW);

        motorOnTime = millis();
      }
    }
  }

}

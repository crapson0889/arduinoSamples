#define LEDR 6
#define LEDB 7
#define BUTTON 2
#define ENABLE 5
#define DIRA 3
#define DIRB 4
#define MTRPIN1 11
#define MTRPIN2 10
#define MTRPIN3 9
#define MTRPIN4 8

#include "LedControl.h"

LedControl lc=LedControl(5,3,4,1);

int pole1[] = {0,0,0,0, 0,1,1,1, 0};
int pole2[] = {0,0,0,1, 1,1,0,0, 0};
int pole3[] = {0,1,1,1, 0,0,0,0, 0};
int pole4[] = {1,1,0,0, 0,0,0,1, 0};

int poleStep = 0;
int stopMotor = 8;

int buttonState = HIGH;
int motorState = LOW;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 250;
unsigned long motorOnTime = 0;
unsigned long motorDuration = 10000;
int secondsLeft = 0;

const int rolePerMinute = 15; 

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

  pinMode(MTRPIN1, OUTPUT);
  pinMode(MTRPIN2, OUTPUT);
  pinMode(MTRPIN3, OUTPUT);
  pinMode(MTRPIN4, OUTPUT);

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

  driveStepper(stopMotor);

  digitalWrite(LEDB, HIGH);
  //analogWrite(ENABLE,180);
  delay(1000);
  //analogWrite(ENABLE,128);
  delay(1000);
  //analogWrite(ENABLE,50);
  delay(1000);
  //digitalWrite(ENABLE, LOW);
  digitalWrite(LEDB, LOW);

  lc.clearDisplay(0);
}

void driveStepper(int c) {
  digitalWrite(MTRPIN1, pole1[c]);
  digitalWrite(MTRPIN2, pole2[c]);
  digitalWrite(MTRPIN3, pole3[c]);
  digitalWrite(MTRPIN4, pole4[c]);
}

void loop() {
  int reading = digitalRead(BUTTON);

  if(motorState == HIGH) {
    //Serial.println(motorDuration - (millis() - motorOnTime));

    Serial.println("Stepping " + poleStep);

    driveStepper(poleStep);
    poleStep++;

    if(poleStep > 7) {
      poleStep=0;
    }
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

        driveStepper(stopMotor);

        //digitalWrite(DIRA, LOW);
        //digitalWrite(ENABLE, LOW);
        
        delay(1000);
        lc.clearDisplay(0);
      } else {
        Serial.println("Starting motor");

        motorState = HIGH;
        digitalWrite(LEDR, motorState);

        //digitalWrite(ENABLE, HIGH);
        //digitalWrite(DIRA, HIGH);
        //digitalWrite(DIRB, LOW);
    
        digitalWrite(LEDR, motorState);
  
        motorOnTime = millis();
      }
    }
    
    //delay(1);
  }

}

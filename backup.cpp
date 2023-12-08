#include <Arduino_FreeRTOS.h>
#include <task.h>
#include <LiquidCrystal.h>
#include "Timer.h"

int distance, lastDistance; //for hifi
int distanceThreshold = 2;
long duration;
bool reverse = false;

int trigPin = 5;
int echoPin = 6;
const float SOUND = 0.034;

int TIMER_PERIOD = 10;

const int light_Period = 100; //light period
int light_Elapsed = light_Period;

const int buzzer_Period = 125; //buzzer period
int buzzer_Elapsed = buzzer_Period;

const int reverse_Period = 400; //how often we probe for reversing
int reverse_Elapsed = reverse_Period;

const int rs = 12, en = 11, d4 = 7, d5 = 8, d6 = 9, d7 = 10;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

enum STATE {light_INIT, light_OFF, light_LEFT, light_RIGHT} gState = light_INIT;

void tickLights() {
    switch(gState) {
        case light_INIT:
            gState = light_OFF;
            break;
        case light_OFF:
            gState = light_LEFT;
            break;
        case light_LEFT:
            gState = light_RIGHT;
            break;
        case light_RIGHT:
            gState = light_OFF;
            break;
        default:
            gState = light_INIT;
            break;
    }


    switch(gState) {
        case light_INIT:
            break;
        case light_OFF:
            //Serial.print("OFF\n");
            digitalWrite(2, LOW);
            digitalWrite(3, LOW);
            break;
        case light_LEFT:
            //Serial.print("LEFT\n");
            digitalWrite(2, HIGH);
            digitalWrite(3, LOW);
            break;
        case light_RIGHT:
            //Serial.print("RIGHT\n");
            digitalWrite(2, LOW);
            digitalWrite(3, HIGH);
            break;
    }
}

enum STATE_B {buzzer_INIT, buzzer_OFF, buzzer_ON} bState = buzzer_INIT;

void tickBuzzer(void) {
    switch(bState) {
        case buzzer_INIT:
            bState = buzzer_OFF;
            break;
        case buzzer_OFF:
            // if(distance < 10) {
            //     bState = buzzer_ON;
            // }

            bState = buzzer_ON;
            break;
        case buzzer_ON:
            // if(distance > 10) {
            //   bState = buzzer_OFF;
            // }
            
            bState = buzzer_OFF;
            break;
        default:
            bState = buzzer_INIT;
            break;
    }

    switch(bState) {
        case buzzer_OFF:
            Serial.print(distance);
            digitalWrite(4, LOW);
            
            //delay(500);
            break;
        case buzzer_ON:
            Serial.print("ON\n");
            Serial.print(distance);
            digitalWrite(4, HIGH);
            
            //delay(500);
            break;        
    }
}


bool backingUp() {

    int temp = lastDistance - distance;
    if(temp >= distanceThreshold) {
        return true;
    }

    return false;
}


void setup() {
    TimerSet(TIMER_PERIOD);
    TimerOn();

    lcd.begin(16, 2); //cursor initialization

    lcd.setCursor(0,0);

    Serial.begin(9600);

    pinMode(2, OUTPUT); //led for flashing backup
    pinMode(3, OUTPUT);
    pinMode(trigPin, OUTPUT); //hifi sensor
    pinMode(echoPin, INPUT);
    pinMode(4, OUTPUT); //beeping output

    digitalWrite(4, LOW);
}

int getDistance() {
    int localDistance = distance;
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH);
    // Calculating the distance

    lastDistance = localDistance;
    localDistance = duration * SOUND / 2;

    return localDistance;
}


void loop() {

    if(reverse_Elapsed >= reverse_Period) {
        distance = getDistance();
        reverse = backingUp();
        reverse_Elapsed = 0;
    }

    reverse_Elapsed += TIMER_PERIOD;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Distance: ");
    lcd.print(distance);
    lcd.print(" cm");

    if((light_Elapsed >= light_Period) && reverse) {
        tickLights();
        light_Elapsed = 0;
    } else if(!reverse) {
        digitalWrite(2, LOW);
        digitalWrite(3, LOW);
    }

    light_Elapsed += TIMER_PERIOD;

    if((buzzer_Elapsed >= buzzer_Period) && distance < 10) {
        tickBuzzer();
        buzzer_Elapsed = 0;
    } else if (distance > 10) {
        digitalWrite(4, LOW);
    }

    buzzer_Elapsed += TIMER_PERIOD;

    while(!TimerFlag){}
    TimerFlag = 0;  

    lastDistance = distance;
}


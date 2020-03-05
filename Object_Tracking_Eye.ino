#include "EyeDisplay.h"
#include <Wire.h>
#include <SparkFun_APDS9960.h>

EyeDisplay eye(-1);
SparkFun_APDS9960 apds = SparkFun_APDS9960();

enum State { IDLE, ALERT };
State state = IDLE;

uint8_t proximityData = 0;
constexpr uint8_t nearThreshold = 30;

unsigned long lastAlert;
unsigned long alertDuration = 1000;

void setup() {
    randomSeed(analogRead(0));
    // Serial.begin(9600);

    // Enable gesture sensor and proximity sensor, with no interrupt
    apds.init();
    apds.enableGestureSensor(false);
    apds.setProximityGain(PGAIN_2X);
    apds.enableProximitySensor(false);

    eye.begin();
    setIdle();
}

void loop() {
    apds.readProximity(proximityData);
    apds.clearProximityInt();
    if (proximityData > nearThreshold) {
        setAlert();
    } else if (millis() - lastAlert > alertDuration) {
        setIdle();
    }

    if (state == IDLE)
        runIdle();

    eye.update();
}

unsigned long lastBlink;
unsigned long lastMove;
unsigned long minBlinkInterval = 5000;
unsigned long maxMoveInterval = 1500;
int blinkFreq = 250;
int moveFreq = 70;

void setIdle() {
    state = State::IDLE;

    eye.setIrisSpeed(1.5);
    eye.setTopSpeed(5);
}

void setAlert() {
    state = State::ALERT;
    lastAlert = millis();

    eye.setIrisSpeed(4);
    eye.moveIris(0, 0);
    eye.moveTop(EyeDisplay::top_d0);
    while (eye.isIrisMoving() || eye.isTopMoving()) {
        eye.update();
    }

    if (apds.isGestureAvailable()) {
        switch (apds.readGesture()) {
            case DIR_UP:
                eye.moveIris(0, 20); break;
            case DIR_DOWN:
                eye.moveIris(0, -20); break;
            case DIR_LEFT:
                eye.moveIris(25, 0); break;
            case DIR_RIGHT:
                eye.moveIris(-25, 0); break;
            default:
                eye.moveIris(0, 0);
        }
    }
}

void runIdle() {
    if (millis() - lastBlink > minBlinkInterval && random(blinkFreq) == 0) {
        eye.blink();
        lastBlink = millis();
    }
    if (millis() - lastMove > maxMoveInterval || random(moveFreq) == 0) {
        eye.moveIris(random(-10, 11), random(-8, 9));
        lastMove = millis();
    }
}

void runAlert() {
}

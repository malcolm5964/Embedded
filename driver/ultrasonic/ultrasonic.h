#include <stdio.h>
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "hardware/timer.h"

extern const int timeout;
extern const uint trigPin;
extern const uint echoPin;

void setupUltrasonicPins(uint trigPin, uint echoPin);
uint64_t getPulse(uint trigPin, uint echoPin);
uint64_t getCm(uint trigPin, uint echoPin);



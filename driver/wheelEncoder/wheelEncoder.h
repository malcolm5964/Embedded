#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"

#include "hardware/gpio.h"


extern const uint WHEEL_EN_LEFT_OUT ;
extern const uint WHEEL_EN_LEFT_VCC;
extern const uint WHEEL_EN_RIGHT_OUT;
extern const uint WHEEL_EN_RIGHT_VCC;

void initWheelEncoderLeft();
void initWheelEncoderRight();
int measureSpeedLeft();
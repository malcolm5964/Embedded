#include <stdio.h>
#include "pico/stdlib.h"

#include "hardware/pwm.h"
#include "hardware/gpio.h"

#include "../magnometer/magnometer.h"

extern const uint PWM_LEFT;
extern const uint N1;
extern const uint N2;

extern const uint PWM_RIGHT;
extern const uint N3;
extern const uint N4;

extern uint slice_num_right;
extern uint slice_num_left;

void direction_forward();
void direction_back();
void direction_right();
void direction_left();
void stop();
void init_right_motor();
void init_left_motor();
void set_left_speed(int level);
void set_right_speed(int level);
void turn_right();





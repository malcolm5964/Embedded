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

void init_right_motor();
void init_left_motor();

void set_direction_forward();
void set_direction_back();
void set_direction_right();
void set_direction_left();
void stop();

void set_left_speed(int level);
void set_right_speed(int level);

void move_forward();
void move_backward();
void turn_left();
void turn_right();





#include "motor.h"
#include "../magnometer/magnometer.h"


const uint PWM_LEFT = 10;
const uint N1 = 11;
const uint N2 = 12;

const uint PWM_RIGHT = 15;
const uint N3 = 14;
const uint N4 = 13;

uint slice_num_right = 0;
uint slice_num_left = 0;

void direction_forward()
{   
    gpio_put(N1, 0);
    gpio_put(N2, 1);
    gpio_put(N3, 1);
    gpio_put(N4, 0);
}

void direction_back()
{
    gpio_put(N1, 1);
    gpio_put(N2, 0);    
    gpio_put(N3, 0);
    gpio_put(N4, 1);
}

void direction_right()
{
    gpio_put(N1, 0);
    gpio_put(N2, 1);    
    gpio_put(N3, 0);
    gpio_put(N4, 0);
}

void direction_left()
{
    gpio_put(N1, 0);
    gpio_put(N2, 0);    
    gpio_put(N3, 1);
    gpio_put(N4, 0);
}

void stop()
{
    gpio_put(N1, 0);
    gpio_put(N2, 0);    
    gpio_put(N3, 0);
    gpio_put(N4, 0);
}


void init_right_motor(){
    printf("Init All\n");
    gpio_set_function(PWM_RIGHT, GPIO_FUNC_PWM);
    slice_num_right = pwm_gpio_to_slice_num(PWM_RIGHT);
    pwm_set_clkdiv(slice_num_right, 100);
    pwm_set_wrap(slice_num_right, 62500);
    pwm_set_enabled(slice_num_right, true);
    gpio_init(N3);
    gpio_init(N4);
    gpio_set_dir(N3, GPIO_OUT);
    gpio_set_dir(N4, GPIO_OUT);
}

void init_left_motor(){
    gpio_set_function(PWM_LEFT, GPIO_FUNC_PWM);
    slice_num_left = pwm_gpio_to_slice_num(PWM_LEFT);
    pwm_set_clkdiv(slice_num_left, 100);
    pwm_set_wrap(slice_num_left, 62500);
    pwm_set_enabled(slice_num_left, true);
    gpio_init(N1);
    gpio_init(N2);
    gpio_set_dir(N1, GPIO_OUT);
    gpio_set_dir(N2, GPIO_OUT);
}

void set_left_speed(int level){
    pwm_set_chan_level(slice_num_left, PWM_CHAN_A, 62500 / level);
}

void set_right_speed(int level){
    pwm_set_chan_level(slice_num_right, PWM_CHAN_B, 62500 / level);
}

void turn_right(){
    direction_right();
    set_left_speed(3);
    vTaskDelay(950);
}



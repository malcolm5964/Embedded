#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"

#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define mbaTASK_MESSAGE_BUFFER_SIZE       ( 60 )

const uint PWM_LEFT = 10;
const uint N1 = 11;
const uint N2 = 12;

const uint PWM_RIGHT = 15;
const uint N3 = 14;
const uint N4 = 13;

const uint WHEEL_EN_LEFT_OUT = 26;
const uint WHEEL_EN_LEFT_VCC = 7;

const uint WHEEL_EN_RIGHT_OUT = 27;
const uint WHEEL_EN_RIGHT_VCC = 6;

uint slice_num_right;
uint slice_num_left;


void leftForward() {
    gpio_set_pulls(N1, true, false);
    gpio_set_pulls(N2, true, false);
}

void leftBack() {
    gpio_set_pulls(N1, false, true);
    gpio_set_pulls(N2, false, true);
}

void Forward() {
    gpio_set_pulls(N3, false, true);
    gpio_set_pulls(N4, true, false);
    gpio_set_pulls(N1, false, true);
    gpio_set_pulls(N2, true, false);
}

void Back() {
    gpio_set_pulls(N3, true, false);
    gpio_set_pulls(N4, false, true);
    gpio_set_pulls(N1, true, false);
    gpio_set_pulls(N2, false, true);
}



void moving_task(__unused void *params) {

    //Init Right Motor
    gpio_set_function(PWM_RIGHT, GPIO_FUNC_PWM);
    slice_num_right = pwm_gpio_to_slice_num(PWM_RIGHT);
    pwm_set_clkdiv(slice_num_right, 100);
    pwm_set_wrap(slice_num_right, 62500);
    pwm_set_enabled(slice_num_right, true);
    gpio_set_dir(N3, GPIO_OUT);
    gpio_set_dir(N4, GPIO_OUT);

    //Left Motor
    gpio_set_function(PWM_LEFT, GPIO_FUNC_PWM);
    slice_num_left = pwm_gpio_to_slice_num(PWM_LEFT);
    pwm_set_clkdiv(slice_num_left, 100);
    pwm_set_wrap(slice_num_left, 62500);
    pwm_set_enabled(slice_num_left, true);  
    gpio_set_dir(N1, GPIO_OUT);
    gpio_set_dir(N2, GPIO_OUT);

    
    while (true) {
    vTaskDelay(10);
    Forward();
    pwm_set_chan_level(slice_num_left, PWM_CHAN_A, 62500/5);
    pwm_set_chan_level(slice_num_right, PWM_CHAN_B, 62500/5);

    //vTaskDelay(2000);
    //pwm_set_chan_level(slice_num_left, PWM_CHAN_A, 0);
    //pwm_set_chan_level(slice_num_right, PWM_CHAN_B, 0);
//
    //vTaskDelay(2000);
    //Back();
    //pwm_set_chan_level(slice_num_left, PWM_CHAN_A, 62500/2);
    //pwm_set_chan_level(slice_num_right, PWM_CHAN_B, 62500/2);

    vTaskDelay(2000);
    }

}


void measureSpeed_task(__unused void *params) {

    absolute_time_t previousTime = get_absolute_time();;
    uint8_t edgeCounter = 0;
    uint8_t detect_change;

    //Init Left IR Sensor
    //adc_gpio_init(WHEEL_EN_LEFT_OUT);
    gpio_init(WHEEL_EN_LEFT_OUT);
    gpio_set_dir(WHEEL_EN_LEFT_OUT, GPIO_IN);
    gpio_init(WHEEL_EN_LEFT_VCC);
    gpio_set_dir(WHEEL_EN_LEFT_VCC, GPIO_OUT);
    gpio_put(WHEEL_EN_LEFT_VCC, 1);//Set to HIGH for VCC

    //Init Right IR Sensor
    gpio_init(WHEEL_EN_RIGHT_OUT);
    gpio_set_dir(WHEEL_EN_RIGHT_OUT, GPIO_IN);
    gpio_init(WHEEL_EN_RIGHT_VCC);
    gpio_set_dir(WHEEL_EN_RIGHT_VCC, GPIO_OUT);
    gpio_put(WHEEL_EN_RIGHT_VCC, 1);//Set to HIGH for VCC

    while(true){
        
        if(gpio_get(WHEEL_EN_LEFT_OUT) == 1) {
            detect_change = 1;
        }
        if((gpio_get(WHEEL_EN_LEFT_OUT) == 0) && (detect_change == 1)) {
            detect_change = 0;
            edgeCounter++;
            if(edgeCounter >= 20) {
                edgeCounter = 0;
                absolute_time_t current_time = get_absolute_time();
                uint64_t revolutionTime = absolute_time_diff_us(previousTime, current_time);
                uint64_t speed = 11000000 / revolutionTime;
                printf("Wheel Speed: %lldcm/s\n", speed);
                previousTime = current_time;
            }
        }
    }
}


void vLaunch(void) {
    TaskHandle_t moveCar;
    xTaskCreate(moving_task, "MoveCar", configMINIMAL_STACK_SIZE, NULL, 3, &moveCar);

    TaskHandle_t measureSpeed;
    xTaskCreate(measureSpeed_task, "MeasureSpeed", configMINIMAL_STACK_SIZE, NULL, 3, &measureSpeed);

    vTaskStartScheduler();
}

int main(void)
{
    stdio_init_all();
    stdio_usb_init();
    adc_init();

    vLaunch();

    return 0;
}
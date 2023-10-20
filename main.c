#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"

#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define mbaTASK_MESSAGE_BUFFER_SIZE       ( 60 )

const uint BTN_PIN_PWM_LEFT = 10;
const uint BTN_PIN_N1 = 11;
const uint BTN_PIN_N2 = 12;


const uint BTN_PIN_PWM_RIGHT = 15;
const uint BTN_PIN_N3 = 14;
const uint BTN_PIN_N4 = 13;

const uint BTN_PIN_IR = 9;
const uint BTN_PIN_IR = 8;

uint slice_num_right;
uint slice_num_left;

void leftForward() {
    gpio_set_pulls(BTN_PIN_N1, true, false);
    gpio_set_pulls(BTN_PIN_N2, true, false);
}

void leftBack() {
    gpio_set_pulls(BTN_PIN_N1, false, true);
    gpio_set_pulls(BTN_PIN_N2, false, true);
}

void Forward() {
    gpio_set_pulls(BTN_PIN_N3, false, true);
    gpio_set_pulls(BTN_PIN_N4, true, false);
    gpio_set_pulls(BTN_PIN_N1, false, true);
    gpio_set_pulls(BTN_PIN_N2, true, false);
}

void Back() {
    gpio_set_pulls(BTN_PIN_N3, true, false);
    gpio_set_pulls(BTN_PIN_N4, false, true);
    gpio_set_pulls(BTN_PIN_N1, true, false);
    gpio_set_pulls(BTN_PIN_N2, false, true);
}



void moving_task(__unused void *params) {

    
    while (true) {
    vTaskDelay(10);
    Forward();
    pwm_set_chan_level(slice_num_left, PWM_CHAN_A, 62500/2);
    pwm_set_chan_level(slice_num_right, PWM_CHAN_B, 62500/2);
    printf("ADC value: %d\n", slice_num_right);

    vTaskDelay(2000);
    pwm_set_chan_level(slice_num_left, PWM_CHAN_A, 0);
    pwm_set_chan_level(slice_num_right, PWM_CHAN_B, 0);

    vTaskDelay(2000);
    Back();
    pwm_set_chan_level(slice_num_left, PWM_CHAN_A, 62500/2);
    pwm_set_chan_level(slice_num_right, PWM_CHAN_B, 62500/2);

    vTaskDelay(2000);
    }

}


void speed_callback(uint gpio, uint32_t events) {
    printf("Event: %i\n", events);
}

void measureSpeed_task(__unused void *params) {

    gpio_set_irq_enabled_with_callback(2, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &speed_callback);

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

    //Init Right Motor
    gpio_set_function(BTN_PIN_PWM_RIGHT, GPIO_FUNC_PWM);
    slice_num_right = pwm_gpio_to_slice_num(BTN_PIN_PWM_RIGHT);
    pwm_set_clkdiv(slice_num_right, 100);
    pwm_set_wrap(slice_num_right, 62500);
    pwm_set_enabled(slice_num_right, true);
    gpio_set_dir(BTN_PIN_N3, GPIO_OUT);
    gpio_set_dir(BTN_PIN_N4, GPIO_OUT);

    //Left Motor
    gpio_set_function(BTN_PIN_PWM_LEFT, GPIO_FUNC_PWM);
    slice_num_left = pwm_gpio_to_slice_num(BTN_PIN_PWM_LEFT);
    pwm_set_clkdiv(slice_num_left, 100);
    pwm_set_wrap(slice_num_left, 62500);
    pwm_set_enabled(slice_num_left, true);  
    gpio_set_dir(BTN_PIN_N1, GPIO_OUT);
    gpio_set_dir(BTN_PIN_N2, GPIO_OUT);
    pwm_set_chan_level(slice_num_right, PWM_CHAN_B, 62500/2);

    //Init IR Sensor


    vLaunch();

    return 0;
}
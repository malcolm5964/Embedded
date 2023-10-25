#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"

#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define mbaTASK_MESSAGE_BUFFER_SIZE (60)

const uint BTN_PIN_PWM_LEFT = 10;
const uint BTN_PIN_N1 = 11;
const uint BTN_PIN_N2 = 12;

const uint BTN_PIN_PWM_RIGHT = 15;
const uint BTN_PIN_N3 = 14;
const uint BTN_PIN_N4 = 13;

const uint BTN_PIN_IR = 26;
// const uint BTN_PIN_IR = 8;

uint slice_num_right;
uint slice_num_left;


void leftForward()
{
    gpio_set_pulls(BTN_PIN_N1, true, false);
    gpio_set_pulls(BTN_PIN_N2, true, false);
}

void leftBack()
{
    gpio_set_pulls(BTN_PIN_N1, false, true);
    gpio_set_pulls(BTN_PIN_N2, false, true);
}

void Forward()
{
    gpio_set_pulls(BTN_PIN_N3, false, true);
    gpio_set_pulls(BTN_PIN_N4, true, false);
    gpio_set_pulls(BTN_PIN_N1, false, true);
    gpio_set_pulls(BTN_PIN_N2, true, false);
}

void Back()
{
    gpio_set_pulls(BTN_PIN_N3, true, false);
    gpio_set_pulls(BTN_PIN_N4, false, true);
    gpio_set_pulls(BTN_PIN_N1, true, false);
    gpio_set_pulls(BTN_PIN_N2, false, true);
}

void moving_task(__unused void *params)
{

    while (true)
    {
        vTaskDelay(10);
        Forward();
        pwm_set_chan_level(slice_num_left, PWM_CHAN_A, 62500 / 2);
        pwm_set_chan_level(slice_num_right, PWM_CHAN_B, 62500 / 2);
        printf("ADC value: %d\n", slice_num_right);

        vTaskDelay(2000);
        pwm_set_chan_level(slice_num_left, PWM_CHAN_A, 0);
        pwm_set_chan_level(slice_num_right, PWM_CHAN_B, 0);

        vTaskDelay(2000);
        Back();
        pwm_set_chan_level(slice_num_left, PWM_CHAN_A, 62500 / 2);
        pwm_set_chan_level(slice_num_right, PWM_CHAN_B, 62500 / 2);

        vTaskDelay(2000);
    }
}

void speed_callback(uint gpio, uint32_t events)
{
    printf("Event: %i\n", events);
}

void measureSpeed_task(__unused void *params)
{

    gpio_set_irq_enabled_with_callback(2, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &speed_callback);
}

void ir_sensor()
{
    while (true)
    {
        vTaskDelay(100);

        adc_select_input(0);
        uint32_t left_result = adc_read();
        adc_select_input(1);
        uint32_t right_result = adc_read();

        bool leftThreshold = (left_result >= 600);
        bool rightThreshold = (right_result >= 600);

        if (leftThreshold == 1 && rightThreshold == 1)
        {
            printf("Forward\n");
        }
        else if (leftThreshold == 0 && rightThreshold == 1)
        {
            printf("Turn left\n");
        }
        else if (leftThreshold == 1 && rightThreshold == 0)
        {
            printf("Turn right");
        }
    }
}

void vLaunch(void)
{
    // TaskHandle_t moveCar;
    // xTaskCreate(moving_task, "MoveCar", configMINIMAL_STACK_SIZE, NULL, 3, &moveCar);

    // TaskHandle_t measureSpeed;
    // xTaskCreate(measureSpeed_task, "MeasureSpeed", configMINIMAL_STACK_SIZE, NULL, 3, &measureSpeed);

    TaskHandle_t irSensor;
    xTaskCreate(ir_sensor, "IRSensor", configMINIMAL_STACK_SIZE, NULL, 3, &irSensor);

    vTaskStartScheduler();
}

int main(void)
{
    stdio_init_all();
    stdio_usb_init();

    // Init Right Motor
    gpio_set_function(BTN_PIN_PWM_RIGHT, GPIO_FUNC_PWM);
    slice_num_right = pwm_gpio_to_slice_num(BTN_PIN_PWM_RIGHT);
    pwm_set_clkdiv(slice_num_right, 100);
    pwm_set_wrap(slice_num_right, 62500);
    pwm_set_enabled(slice_num_right, true);
    gpio_set_dir(BTN_PIN_N3, GPIO_OUT);
    gpio_set_dir(BTN_PIN_N4, GPIO_OUT);

    // Left Motor
    gpio_set_function(BTN_PIN_PWM_LEFT, GPIO_FUNC_PWM);
    slice_num_left = pwm_gpio_to_slice_num(BTN_PIN_PWM_LEFT);
    pwm_set_clkdiv(slice_num_left, 100);
    pwm_set_wrap(slice_num_left, 62500);
    pwm_set_enabled(slice_num_left, true);
    gpio_set_dir(BTN_PIN_N1, GPIO_OUT);
    gpio_set_dir(BTN_PIN_N2, GPIO_OUT);
    pwm_set_chan_level(slice_num_right, PWM_CHAN_B, 62500 / 2);

    // Init IR Sensor
    adc_init();
    adc_set_temp_sensor_enabled(true);
    gpio_set_dir(BTN_PIN_IR, GPIO_IN);
    gpio_set_function(BTN_PIN_IR, GPIO_FUNC_SIO);
    gpio_disable_pulls(BTN_PIN_IR);
    gpio_set_input_enabled(BTN_PIN_IR, false);

    vLaunch();

    return 0;
}
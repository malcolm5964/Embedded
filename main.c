#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"
#include <math.h>

#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#include "driver/motor/motor.h"
#include "driver/magnometer/magnometer.h"


#define mbaTASK_MESSAGE_BUFFER_SIZE (60)
static MessageBufferHandle_t rawADCvalue1;
static MessageBufferHandle_t rawADCvalue2;

//IR Sensor
const uint BTN_PIN_IR = 26;
const uint BTN_PIN_IR2 = 27;
const uint LEFT_IR_SENSOR_VCC = 22;

//Wheel IR Sensor
const uint WHEEL_EN_LEFT_OUT = 16;
const uint WHEEL_EN_LEFT_VCC = 7;
const uint WHEEL_EN_RIGHT_OUT = 0;
const uint WHEEL_EN_RIGHT_VCC = 1;

void moving_task(__unused void *params)
{
    vTaskDelay(1000);
    init_right_motor();
    init_left_motor();
    magnometer_init();

    while (true)
    {   
        turn_right();
        stop();

        vTaskDelay(10000);
    }
    
}

void measureSpeed_task(__unused void *params)
{
    absolute_time_t previousTime = get_absolute_time();
    uint8_t edgeCounter = 0;
    uint8_t detect_change;

    // Init Left IR Sensor
    // adc_gpio_init(WHEEL_EN_LEFT_OUT);
    gpio_init(WHEEL_EN_LEFT_OUT);
    gpio_set_dir(WHEEL_EN_LEFT_OUT, GPIO_IN);
    gpio_init(WHEEL_EN_LEFT_VCC);
    gpio_set_dir(WHEEL_EN_LEFT_VCC, GPIO_OUT);
    gpio_put(WHEEL_EN_LEFT_VCC, 1); // Set to HIGH for VCC

    // Init Right IR Sensor
    gpio_init(WHEEL_EN_RIGHT_OUT);
    gpio_set_dir(WHEEL_EN_RIGHT_OUT, GPIO_IN);
    gpio_init(WHEEL_EN_RIGHT_VCC);
    gpio_set_dir(WHEEL_EN_RIGHT_VCC, GPIO_OUT);
    gpio_put(WHEEL_EN_RIGHT_VCC, 1); // Set to HIGH for VCC

    while (true)
    {

        if (gpio_get(WHEEL_EN_LEFT_OUT) == 1)
        {
            detect_change = 1;
        }
        if ((gpio_get(WHEEL_EN_LEFT_OUT) == 0) && (detect_change == 1))
        {
            detect_change = 0;
            edgeCounter++;
            if (edgeCounter >= 20)
            {
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

void ir_sensor()
{
    gpio_init(LEFT_IR_SENSOR_VCC);
    gpio_set_dir(LEFT_IR_SENSOR_VCC, GPIO_OUT);
    gpio_put(LEFT_IR_SENSOR_VCC, 1);
    while (true)
    {
        vTaskDelay(100);

        adc_select_input(0);
        uint32_t left_result = adc_read();
        adc_select_input(1);
        uint32_t right_result = adc_read();

        xMessageBufferSend(
            rawADCvalue1,         /* The message buffer to write to. */
            (void *)&left_result, /* The source of the data to send. */
            sizeof(left_result),  /* The length of the data to send. */
            0);
        xMessageBufferSend(
            rawADCvalue2,          /* The message buffer to write to. */
            (void *)&right_result, /* The source of the data to send. */
            sizeof(right_result),  /* The length of the data to send. */
            0);
    }
}

void vLaunch(void)
{
    TaskHandle_t moveCar;
    xTaskCreate(moving_task, "MoveCar", configMINIMAL_STACK_SIZE, NULL, 3, &moveCar);

    TaskHandle_t measureSpeed;
    xTaskCreate(measureSpeed_task, "MeasureSpeed", configMINIMAL_STACK_SIZE, NULL, 3, &measureSpeed);

    TaskHandle_t irSensor;
    xTaskCreate(ir_sensor, "IRSensor", configMINIMAL_STACK_SIZE, NULL, 3, &irSensor);

    rawADCvalue1 = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    rawADCvalue2 = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);

    vTaskStartScheduler();
}

int main(void)
{
    stdio_init_all();
    stdio_usb_init();

    // Init IR Sensor
    adc_init();
    adc_set_temp_sensor_enabled(true);

    gpio_set_dir(BTN_PIN_IR, GPIO_IN);
    gpio_set_function(BTN_PIN_IR, GPIO_FUNC_SIO);
    gpio_disable_pulls(BTN_PIN_IR);
    gpio_set_input_enabled(BTN_PIN_IR, false);

    gpio_set_dir(BTN_PIN_IR2, GPIO_IN);
    gpio_set_function(BTN_PIN_IR2, GPIO_FUNC_SIO);
    gpio_disable_pulls(BTN_PIN_IR2);
    gpio_set_input_enabled(BTN_PIN_IR2, false);

    vLaunch();

    return 0;
}
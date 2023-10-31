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
#include "driver/wheelEncoder/wheelEncoder.h"


#define mbaTASK_MESSAGE_BUFFER_SIZE (60)
static MessageBufferHandle_t rawADCvalue1;
static MessageBufferHandle_t rawADCvalue2;

//IR Sensor
const uint BTN_PIN_IR = 26;
const uint BTN_PIN_IR2 = 27;
const uint LEFT_IR_SENSOR_VCC = 22;

void moving_task(__unused void *params)
{
    vTaskDelay(1000);
    init_right_motor();
    init_left_motor();

    while (true)
    {   //Test Forward
        move_forward();
        vTaskDelay(5000);
        //Test Right
        turn_right();
        //Test Backwards
        move_backward();
        vTaskDelay(5000);
        stop();
    }
    
}

void measureSpeed_task(__unused void *params)
{
    vTaskDelay(4000);
    initWheelEncoderLeft();
    initWheelEncoderRight();
    measureSpeedLeft();
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
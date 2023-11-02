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
#include "hardware/timer.h"

#include "driver/motor/motor.h"
#include "driver/magnometer/magnometer.h"
#include "driver/wheelEncoder/wheelEncoder.h"
#include "driver/irline/irline.h"
#include "driver/ultrasonic/ultrasonic.h"

#define mbaTASK_MESSAGE_BUFFER_SIZE (60)
static MessageBufferHandle_t rawADCvalue_left;
static MessageBufferHandle_t rawADCvalue_right;
// static MessageBufferHandle_t sendDataUltrasonicSensorCMB;


void moving_task(__unused void *params)
{
    vTaskDelay(1000);
    init_right_motor();
    init_left_motor();

    while (true)
    { // Test Forward
        move_forward();
        vTaskDelay(5000);
        // Test Right
        turn_right();
        // Test Backwards
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
    init_ir();
    while (true)
    {
        vTaskDelay(100);

        bool left_color_detected = left_ir();
        bool right_color_detected = right_ir();

        if (left_color_detected && right_color_detected)
        {
            printf("BLACK\n");
        }
    }
}


void getCM_task(__unused void *params) {

    setupUltrasonicPins(trigPin, echoPin);
    while(true){
        printf("\n %lld cm", getCm(trigPin, echoPin));
        sleep_ms(5000); 
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

    TaskHandle_t getCM;
    xTaskCreate(getCM_task, "getCM", configMINIMAL_STACK_SIZE, NULL, 4, &getCM);

    rawADCvalue_left = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    rawADCvalue_right = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    // sendDataUltrasonicSensorCMB = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);

    vTaskStartScheduler();
}

int main(void)
{
    stdio_init_all();
    stdio_usb_init();

    vLaunch();

    return 0;
}
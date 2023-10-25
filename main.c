#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"

#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define mbaTASK_MESSAGE_BUFFER_SIZE (60)

static MessageBufferHandle_t rawADCvalue1;
static MessageBufferHandle_t rawADCvalue2;

const uint PWM_LEFT = 10;
const uint N1 = 11;
const uint N2 = 12;

const uint PWM_RIGHT = 15;
const uint N3 = 14;
const uint N4 = 13;

const uint BTN_PIN_IR = 26;
const uint BTN_PIN_IR2 = 27;
const uint LEFT_IR_SENSOR_VCC = 22;

const uint WHEEL_EN_LEFT_OUT = 16;
const uint WHEEL_EN_LEFT_VCC = 7;
const uint WHEEL_EN_RIGHT_OUT = 0;
const uint WHEEL_EN_RIGHT_VCC = 1;

uint slice_num_right;
uint slice_num_left;

void Forward()
{   
    gpio_put(N1, 1);
    gpio_put(N2, 0);
    gpio_put(N3, 1);
    gpio_put(N4, 0);
}

void Back()
{
    gpio_put(N1, 0);
    gpio_put(N2, 1);    
    gpio_put(N3, 0);
    gpio_put(N4, 1);

}

void moving_task(__unused void *params)
{
    uint32_t leftIrADC;
    uint32_t rightIrADC;
    size_t xReceivedBytes, xReceivedBytes2;

    // Init Right Motor
    gpio_set_function(PWM_RIGHT, GPIO_FUNC_PWM);
    slice_num_right = pwm_gpio_to_slice_num(PWM_RIGHT);
    pwm_set_clkdiv(slice_num_right, 100);
    pwm_set_wrap(slice_num_right, 62500);
    pwm_set_enabled(slice_num_right, true);
    gpio_set_dir(N3, GPIO_OUT);
    gpio_set_dir(N4, GPIO_OUT);

    // Left Motor
    gpio_set_function(PWM_LEFT, GPIO_FUNC_PWM);
    slice_num_left = pwm_gpio_to_slice_num(PWM_LEFT);
    pwm_set_clkdiv(slice_num_left, 100);
    pwm_set_wrap(slice_num_left, 62500);
    pwm_set_enabled(slice_num_left, true);
    gpio_set_dir(N1, GPIO_OUT);
    gpio_set_dir(N2, GPIO_OUT);

    while (true)
    {
        vTaskDelay(100);
        Forward();
        pwm_set_chan_level(slice_num_left, PWM_CHAN_A, 62500 / 5);
        pwm_set_chan_level(slice_num_right, PWM_CHAN_B, 62500 / 5);

        xReceivedBytes = xMessageBufferReceive(
            rawADCvalue1,       /* The message buffer to receive from. */
            (void *)&leftIrADC, /* Location to store received data. */
            sizeof(leftIrADC),  /* Maximum number of bytes to receive. */
            portMAX_DELAY);

        xReceivedBytes2 = xMessageBufferReceive(
            rawADCvalue2,        /* The message buffer to receive from. */
            (void *)&rightIrADC, /* Location to store received data. */
            sizeof(rightIrADC),  /* Maximum number of bytes to receive. */
            portMAX_DELAY);

        if (xReceivedBytes > 0 && xReceivedBytes2 > 0)
        {
            // printf("Left ADC Result: %d\n", leftIrADC);
            // printf("Right ADC Result: %d\n", rightIrADC);
            bool leftThreshold = (leftIrADC >= 600);
            bool rightThreshold = (rightIrADC >= 600);

            if (leftThreshold == 1 && rightThreshold == 1)
            {
                printf("Forward");
            }
            else if (leftThreshold == 0 && rightThreshold == 1)
            {
                printf("Turn left");
            }
            else if (leftThreshold == 1 && rightThreshold == 0)
            {
                printf("Turn right");
            }
        }

        // vTaskDelay(2000);
        // pwm_set_chan_level(slice_num_left, PWM_CHAN_A, 0);
        // pwm_set_chan_level(slice_num_right, PWM_CHAN_B, 0);
        //
        // vTaskDelay(2000);
        // Back();
        // pwm_set_chan_level(slice_num_left, PWM_CHAN_A, 62500/2);
        // pwm_set_chan_level(slice_num_right, PWM_CHAN_B, 62500/2);

        // vTaskDelay(100);
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
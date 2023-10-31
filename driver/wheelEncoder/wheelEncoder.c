#include "wheelEncoder.h"



//Wheel IR Sensor
const uint WHEEL_EN_LEFT_OUT = 2;
const uint WHEEL_EN_LEFT_VCC = 3;
const uint WHEEL_EN_LEFT_GND = 4;

const uint WHEEL_EN_RIGHT_OUT = 0;
const uint WHEEL_EN_RIGHT_VCC = 1;

void initWheelEncoderLeft(){
    printf("Wheel encoder left init\n");
    // Init Left IR Sensor out
    gpio_init(WHEEL_EN_LEFT_OUT);
    gpio_set_dir(WHEEL_EN_LEFT_OUT, GPIO_IN);
    // Set to HIGH for VCC
    gpio_init(WHEEL_EN_LEFT_VCC);
    gpio_set_dir(WHEEL_EN_LEFT_VCC, GPIO_OUT);
    gpio_put(WHEEL_EN_LEFT_VCC, 1);
    //Set low for GND
    gpio_init(WHEEL_EN_LEFT_GND);
    gpio_set_dir(WHEEL_EN_LEFT_GND, GPIO_OUT);
    gpio_put(WHEEL_EN_LEFT_GND, 0);
}

void initWheelEncoderRight(){
    printf("Wheel encoder left init\n");
    // Init Right IR Sensor
    gpio_init(WHEEL_EN_RIGHT_OUT);
    gpio_set_dir(WHEEL_EN_RIGHT_OUT, GPIO_IN);
    gpio_init(WHEEL_EN_RIGHT_VCC);
    gpio_set_dir(WHEEL_EN_RIGHT_VCC, GPIO_OUT);
    gpio_put(WHEEL_EN_RIGHT_VCC, 1); // Set to HIGH for VCC
}

int measureSpeedLeft(){
    printf("Wheel encoder measure speed\n");
    absolute_time_t previousTime = get_absolute_time();
    uint8_t edgeCounter = 0;
    uint8_t detect_change;

    while (true)
    {
        //printf("test: %i\n", gpio_get(WHEEL_EN_RIGHT_OUT));

        if (gpio_get(WHEEL_EN_RIGHT_OUT) == 1)
        {
            detect_change = 1;
        }
        if ((gpio_get(WHEEL_EN_RIGHT_OUT) == 0) && (detect_change == 1))
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
#include "irline.h"
#include "../magnometer/magnometer.h"

const uint LEFT_IR_PIN_ADC = 26;
const uint RIGHT_IR_PIN_ADC = 27;
const uint LEFT_IR_SENSOR_VCC = 22;
const uint COLOUR_THRESHOLD = 2000;

volatile uint32_t left_ir_raw;
volatile uint32_t right_ir_raw;
volatile bool colour_detected;

void init_ir()
{
    printf("Scanning for BLACKS\n");

    adc_init();
    adc_set_temp_sensor_enabled(true);

    gpio_init(LEFT_IR_SENSOR_VCC);
    gpio_set_dir(LEFT_IR_SENSOR_VCC, GPIO_OUT);
    gpio_put(LEFT_IR_SENSOR_VCC, 1);

    gpio_set_dir(LEFT_IR_PIN_ADC, GPIO_IN);
    gpio_set_function(LEFT_IR_PIN_ADC, GPIO_FUNC_SIO);
    gpio_disable_pulls(LEFT_IR_PIN_ADC);
    gpio_set_input_enabled(LEFT_IR_PIN_ADC, false);

    gpio_set_dir(RIGHT_IR_PIN_ADC, GPIO_IN);
    gpio_set_function(RIGHT_IR_PIN_ADC, GPIO_FUNC_SIO);
    gpio_disable_pulls(RIGHT_IR_PIN_ADC);
    gpio_set_input_enabled(RIGHT_IR_PIN_ADC, false);
}

void get_ir_value()
{
    while (true)
    {
        adc_select_input(0);
        left_ir_raw = adc_read();

        adc_select_input(1);
        right_ir_raw = adc_read();

        printf("%d\n", left_ir_raw);
        printf("%d\n", right_ir_raw);
    }
}

bool left_ir()
{
    adc_select_input(0);
    left_ir_raw = adc_read();

    return left_ir_raw > COLOUR_THRESHOLD;
}

bool right_ir()
{
    adc_select_input(1); 
    right_ir_raw = adc_read();

    return right_ir_raw > COLOUR_THRESHOLD;
}
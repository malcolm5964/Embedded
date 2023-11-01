#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"

extern volatile uint32_t left_ir_raw;
extern volatile uint32_t right_ir_raw;

void init_ir();
void get_ir_value();
bool left_ir();
bool right_ir();
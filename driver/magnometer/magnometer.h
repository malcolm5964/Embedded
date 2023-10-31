#include <stdio.h>
#include "pico/stdlib.h"
#include <math.h>
#include "hardware/i2c.h"
#include "hardware/gpio.h"

//Magnetometer
extern const uint mag_VIN;

//i2c Pins
extern const uint sda_pin;
extern const uint scl_pin;

//Ports
extern i2c_inst_t *i2c;

//Buffer to store raw reads
extern uint8_t magData[6];
extern uint8_t accData[6];

//Magnometer Register and Values
extern const uint8_t MAG_ADDRESS;

extern const int CRA_REG_M_ADDR;
extern const int CRB_REG_M_ADDR;
extern const int MR_REG_M_ADDR;

extern int CRA_REG_M_value;
extern int CRB_REG_M_value;
extern int MR_REG_M_value;

extern const uint8_t MAG_OUT_X_H_M;

//Accelerator Register and Values
extern const uint8_t ACC_ADDRESS;

extern const uint8_t CTRL_REG1_ADDR;
extern const uint8_t CTRL_REG2_ADDR;
extern const uint8_t CTRL_REG3_ADDR;
extern const uint8_t CTRL_REG4_ADDR;
extern const uint8_t CTRL_REG5_ADDR;
extern const uint8_t CTRL_REG6_ADDR;

extern int CTRL_REG1_A_value; //50Hz Low Power disable, all axes enabled
extern int CTRL_REG2_A_value; //
extern int CTRL_REG3_A_value; //no interrupt or watermark enabled
extern int CTRL_REG4_A_value; // 2G full scale, high resolution, no SPI mode selected
extern int CTRL_REG5_A_value; // nothing enabled
extern int CTRL_REG6_A_value;

extern const uint8_t ACC_X_REG; // Address for retrieveing


int reg_write(i2c_inst_t *i2c, 
                const uint addr, 
                const uint8_t reg, 
                uint8_t buf,
                const uint8_t nbytes);

int reg_read(i2c_inst_t *i2c,
                const uint addr,
                const uint8_t reg,
                uint8_t *buf,
                const uint8_t nbytes);

void magnometer_init();

float magnometer_read();

void accelerator_read();


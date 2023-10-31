#include "magnometer.h"

//Magnetometer
const uint mag_VIN = 16;

//i2c Pins
const uint sda_pin = 18;
const uint scl_pin = 19;

//Ports
i2c_inst_t *i2c = i2c1;

uint8_t magData[6];
uint8_t accData[6];

//Magnometer Register and Values
const uint8_t MAG_ADDRESS = 0x1E;

const int CRA_REG_M_ADDR = 0x00;
const int CRB_REG_M_ADDR = 0x01;
const int MR_REG_M_ADDR = 0x02;

int CRA_REG_M_value = 0x10;
int CRB_REG_M_value = 0x20;
int MR_REG_M_value = 0x00;

const uint8_t MAG_OUT_X_H_M = 0x03;

//Accelerator Register and Values
const uint8_t ACC_ADDRESS = 0x19;

const uint8_t CTRL_REG1_ADDR = 0x20;
const uint8_t CTRL_REG2_ADDR = 0x21;
const uint8_t CTRL_REG3_ADDR = 0x22;
const uint8_t CTRL_REG4_ADDR = 0x23;
const uint8_t CTRL_REG5_ADDR = 0x24;
const uint8_t CTRL_REG6_ADDR = 0x25;

int CTRL_REG1_A_value = 0x57; //50Hz Low Power disable, all axes enabled
int CTRL_REG2_A_value = 0x00; //
int CTRL_REG3_A_value = 0x00; //no interrupt or watermark enabled
int CTRL_REG4_A_value = 0x00; // 2G full scale, high resolution, no SPI mode selected
int CTRL_REG5_A_value = 0x00; // nothing enabled
int CTRL_REG6_A_value = 0x00;

const uint8_t ACC_X_REG = 0x28; // Address for retrieveing

// Write 1 byte to the specified register
int reg_write(  i2c_inst_t *i2c, 
                const uint addr, 
                const uint8_t reg, 
                uint8_t buf,
                const uint8_t nbytes) {

    int num_bytes_read = 0;
    uint8_t msg[nbytes + 1];

    // Check to make sure caller is sending 1 or more bytes
    if (nbytes < 1) {
        return 0;
    }

    // Append register address to front of data packet
    msg[0] = reg;
    for (int i = 0; i < nbytes; i++) {
        msg[i + 1] = buf;
    }

    // Write data to register(s) over I2C
    i2c_write_blocking(i2c, addr, msg, (nbytes + 1), false);

    return num_bytes_read;
}

// Read byte(s) from specified register. If nbytes > 1, read from consecutive
// registers.
int reg_read(  i2c_inst_t *i2c,
                const uint addr,
                const uint8_t reg,
                uint8_t *buf,
                const uint8_t nbytes) {

    int num_bytes_read = 0;

    // Check to make sure caller is asking for 1 or more bytes
    if (nbytes < 1) {
        return 0;
    }

    // Read data from register(s) over I2C
    i2c_write_blocking(i2c, addr, &reg, 1, true);
    num_bytes_read = i2c_read_blocking(i2c, addr, buf, nbytes, false);

    return num_bytes_read;
}

void magnometer_init(){
    printf("magno init\n");
    //Set VIN to 3.3v
    gpio_init(mag_VIN);
    gpio_set_dir(mag_VIN, GPIO_OUT);
    gpio_put(mag_VIN, 1); 

    //Initialize I2C port at 400 kHz
    i2c_init(i2c, 400000);

    //Set SDA and SCL pin to I2C
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);

    gpio_pull_up(sda_pin);                  // Enable pull-up on SDA
    gpio_pull_up(scl_pin);                  // Enable pull-up on SCL
}

//Read Magnometer Data
float magnometer_read(){
    printf("magno read\n");

    //Write to acceleratometer register
    //reg_write(i2c, ACC_ADDRESS, CTRL_REG1_ADDR, CTRL_REG1_A_value, 1);
    //reg_write(i2c, ACC_ADDRESS, CTRL_REG2_ADDR, CTRL_REG2_A_value, 1);
    //reg_write(i2c, ACC_ADDRESS, CTRL_REG3_ADDR, CTRL_REG3_A_value, 1);
    //reg_write(i2c, ACC_ADDRESS, CTRL_REG4_ADDR, CTRL_REG4_A_value, 1);
    //reg_write(i2c, ACC_ADDRESS, CTRL_REG5_ADDR, CTRL_REG5_A_value, 1);
    //reg_write(i2c, ACC_ADDRESS, CTRL_REG6_ADDR, CTRL_REG6_A_value, 1);

    //Enable continuous conversion
    uint8_t config[] = {MR_REG_M_ADDR, MR_REG_M_value};
    i2c_write_blocking(i2c1, MAG_ADDRESS, config, sizeof(config), true);
    //reg_write(i2c, MAG_ADDRESS, MR_REG_M_ADDR, MR_REG_M_value, 1);

    //Set data rate
    config[0] = CRA_REG_M_ADDR;
    config[1] = CRA_REG_M_value;
    i2c_write_blocking(i2c1, MAG_ADDRESS, config, sizeof(config), true);
    //reg_write(i2c, MAG_ADDRESS, CRA_REG_M_ADDR, CRA_REG_M_value, 1);

    //Set gain on magnetometer
    config[0] = CRB_REG_M_ADDR;
    config[1] = CRB_REG_M_value;
    i2c_write_blocking(i2c1, MAG_ADDRESS, config, sizeof(config), true);

    reg_read(i2c, MAG_ADDRESS, MAG_OUT_X_H_M, magData, 6);

    uint8_t xhm = magData[0];
    uint8_t xlm = magData[1];
    uint8_t zhm = magData[2];
    uint8_t zlm = magData[3];
    uint8_t yhm = magData[4];
    uint8_t ylm = magData[5];

    int raw_xm = (int16_t)((xhm << 8) | xlm);
    int raw_ym = (int16_t)((yhm << 8) | ylm);
    int raw_zm = (int16_t)((zhm << 8) | zlm);

    printf("X: %i, Y: %i, Z: %i\n", raw_xm, raw_ym, raw_zm);

    // Calculate the heading angle
    float heading_radians = atan2(raw_ym / 1370.0, raw_xm / 1370.0);

    float heading_degrees = heading_radians * 180.0 / M_PI;

    if(heading_degrees < 0) {
        heading_degrees += 360.0;
    }

    printf("Angle: %.3f degrees\n", heading_degrees);

    sleep_ms(1000);
    
    return 1;
}


void accelerator_read(){
    

    //uint8_t xla = accData[0];
    //uint8_t xha = accData[1];
    //uint8_t yla = accData[2];
    //uint8_t yha = accData[3];
    //uint8_t zla = accData[4];
    //uint8_t zha = accData[5];

    //int16_t raw_xa = (int16_t)((xla | (xha << 8)) >> 4);
    //int16_t raw_ya = (int16_t)((yla | (yha << 8)) >> 4);
    //int16_t raw_za = (int16_t)((zla | (zha << 8)) >> 4);
}
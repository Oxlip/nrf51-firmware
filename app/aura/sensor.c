#include <stdio.h>
#include <string.h>
#include <nrf_gpio.h>
#include <app_timer.h>
#include <app_gpiote.h>
#include <app_button.h>

#include "platform.h"
#include <boards.h>
#include "twi_master.h"
#include "sensor.h"

#define CURRENT_SENSOR_ADDRESS  0x2
#define UPPER_BIT               24
#define UPPER_BIT_IDX           (UPPER_BIT - 1)

enum outlet_reg_t {
    REG_COMMAND = 0,
    REG_STATUS= 0x1A,
    REG_CONFIG = 0x25,
    REG_VA_RMS = 0x2B,
    REG_VB_RMS = 0x2C,
    REG_VA_PEAK = 0x3A,
    REG_VB_PEAK = 0x3B,
    REG_IA_RMS = 0x3E,
    REG_IB_RMS = 0x3F,
    REG_IA = 0x44,
    REG_IB = 0x45,
    REG_IA_PEAK = 0x46,
    REG_IB_PEAK = 0x47,
    REG_WATT_A = 0x4B,
    REG_WATT_B = 0x4C,
    REG_LINE_FREQ = 0x6D,
};


/** Convert given signed binary float representation in S.N format to C float pointing number.
 */
static double cs_signed_to_float(uint32_t val, uint32_t pointIndex)
{
    int i, sign, index = 2;
    double res = 0;
    int tmpres = 0;

    sign = (1 << UPPER_BIT_IDX) & val;
    val &= ~(1 << UPPER_BIT_IDX);

    for (i = UPPER_BIT_IDX - 1; i >= pointIndex; i--) {
        tmpres |= ((1 << i) & val) >> pointIndex;
    }
    res = (double)tmpres;

    for (; i >= 0; i--, index *= 2) {
        if ((1 << i) & val) {
            res += 1. / index;
        }
    }
    if (sign) {
        res *= -1;
    }
    return res;
}

/** Read current sensor.
 */
static uint8_t
sensor_78M6610_read(uint8_t slave_address, uint8_t reg, uint8_t *data)
{
    bool succeed = false;

    slave_address = (slave_address << 1);

    /* set the register address */
    succeed = twi_master_transfer(slave_address, &reg, 1, true);

    succeed &= twi_master_transfer(slave_address | TWI_READ_BIT, data, 3, true);

    return succeed;
}

#if 0
/** Write current sensor.
 */
static uint8_t
sensor_78M6610_write(uint8_t slave_address, uint8_t reg, uint8_t *data)
{
    uint8_t buf[4];
    int i;

    buf[0] = reg;
    for (i = 0; i < 3; i++) {
        buf[i + 1] = data[i];
    }
    return twi_master_transfer(slave_address << 1, buf, 4, true);
}
#endif

bool
cs_calibrate()
{
#define CALIBRATE_SETTING 0b01111111
    uint8_t result;
    uint8_t buf[] = {REG_COMMAND, 0xCA, CALIBRATE_SETTING, 0};
    uint8_t read_buf[3] = {0};

    printf("Calibrating current sensor");

    result = twi_master_transfer(CURRENT_SENSOR_ADDRESS << 1, buf, sizeof(buf), true);
    if (!result) {
        printf("failed to calibrate.\n");
        return -1;
    }
    do {
        sensor_78M6610_read(CURRENT_SENSOR_ADDRESS, REG_COMMAND, read_buf);
        if (read_buf[0]!= 0) {
            printf("still calibrating %x %x %x\n", read_buf[0], read_buf[1], read_buf[2]);
        }
    }while(read_buf[0]!= 0);
    result = sensor_78M6610_read(CURRENT_SENSOR_ADDRESS, REG_CONFIG, read_buf);
    if (!result) {
        printf("failed to read config.\n");
    }
    printf("Config %x %x %x\n", read_buf[0], read_buf[1], read_buf[2]);
    return 0;
}


static inline double cs_get(int reg, int fbits)
{
    uint8_t buf[3];
    uint32_t temp;
    double res;

    if (!sensor_78M6610_read(CURRENT_SENSOR_ADDRESS, reg, buf)) {
        printf("Could not read current sensor.\n");
        return -1;
    }
    //temp = (buf[2] << 16) | (buf[1] << 8) | buf[0];
    temp = (buf[0] << 16) | (buf[1] << 8) | buf[0];
    res = cs_signed_to_float(temp, fbits);
    return res;
}

uint32_t cs_get_status()
{
    uint32_t status = 0;
    if (!sensor_78M6610_read(CURRENT_SENSOR_ADDRESS, REG_STATUS, (uint8_t*)&status)) {
        printf("Could not get current sensor status.\n");
        return -1;
    }
    return status;
}

/** Get RMS current.
 */
double cs_get_rms_current(int outlet_nuber)
{
    return cs_get(REG_IA_RMS + outlet_nuber, 23);
}

/** Get RMS voltage.
 */
double cs_get_rms_voltage(int outlet_nuber)
{
    return cs_get(REG_VA_RMS + outlet_nuber, 23);
}

/** Get peak current.
 */
double cs_get_peak_current(int outlet_nuber)
{
    return cs_get(REG_IA_PEAK + outlet_nuber, 23);
}

/** Get peak voltage.
 */
double cs_get_peak_voltage(int outlet_nuber)
{
    return cs_get(REG_VA_PEAK + outlet_nuber, 23);
}

/** Get active watts.
 */
double cs_get_active_watts(int outlet_nuber)
{
    return cs_get(REG_WATT_A + outlet_nuber, 23);
}

/** Get line frequency.
 */
double cs_get_line_frequency()
{
    return cs_get(REG_LINE_FREQ, 16);
}

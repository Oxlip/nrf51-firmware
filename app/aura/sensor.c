#include <stdio.h>
#include <string.h>
#include <nrf_gpio.h>
#include <app_timer.h>
#include <app_gpiote.h>
#include <app_button.h>

#include "platform.h"
#include "board_conf.h"
#include "twi_master.h"
#include "sensor.h"

#define CURRENT_SENSOR_ADDRESS  0x2
#define UPPER_BIT               24
#define UPPER_BIT_IDX           (UPPER_BIT - 1)

enum outlet_reg_t {
    REG_VA_RMS = 0x2B,
    REG_VB_RMS = 0x2C,
    REG_IA_RMS = 0x3E,
    REG_IB_RMS = 0x3F,
    REG_IA = 0x44,
    REG_IB = 0x45,
    REG_WATT_A = 0x4B,
    REG_WATT_B = 0x4C
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

/** Get instant current.
 */
double sensor_get_instant_current(int outlet_nuber)
{
    int reg = REG_IA + outlet_nuber;
    uint32_t buf = 0;
    double res;

    if (!sensor_78M6610_read(CURRENT_SENSOR_ADDRESS, reg, (uint8_t*)&buf)) {
        printf("Could not get the current\n");
        return -1;
    }

    res = cs_signed_to_float(buf, 23);
    printf("Sensor got %f\n", res);
    return res;
}

static inline double cs_get( int reg, int outlet_nuber)
{
    uint32_t buf = 0;
    double res;

    if (!sensor_78M6610_read(CURRENT_SENSOR_ADDRESS, reg, (uint8_t*)&buf)) {
        printf("Could not read current sensor\n");
        return -1;
    }

    res = cs_signed_to_float(buf, 23);
    printf("Sensor read %lx from %x\n", buf, reg);
    return res;
}

/** Get RMS current.
 */
double cs_get_rms_current(int outlet_nuber)
{
    return cs_get(REG_IA_RMS + outlet_nuber, outlet_nuber);
}

/** Get RMS voltage.
 */
double cs_get_rms_voltage(int outlet_nuber)
{
    return cs_get(REG_VA_RMS + outlet_nuber, outlet_nuber);
}


/** Get active watts.
 */
double cs_get_active_watts(int outlet_nuber)
{
    return cs_get(REG_WATT_A + outlet_nuber, outlet_nuber);
}

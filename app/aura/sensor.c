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
    REG_IA = 44,
    REG_IB = 45,
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
    int reg;
    uint32_t buf = 0;
    double res;

    if (outlet_nuber == 0) {
        reg = REG_IA;
    }
    else {
        reg = REG_IB;
    }

    if (!sensor_78M6610_read(CURRENT_SENSOR_ADDRESS, reg, (uint8_t*)&buf)) {
        printf("Could not get the current\n");
        return -1;
    }

    res = cs_signed_to_float(buf, 23);
    printf("Sensor got %f\n", res);
    return res;
}

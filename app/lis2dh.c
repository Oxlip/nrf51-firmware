/* Driver for LIS2DH.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <twi_master.h>
#include <nrf_gpio.h>
#include <app_gpiote.h>

#include "board_conf.h"

#define LIS2DH_INT1_PIN             6
#define LIS2DH_INT2_PIN             5

#define LIS2DH_I2C_ADDR             0x18

#define LIS2DH_REG_STATUS_AUX       0x7
#define LIS2DH_REG_OUT_TEMP_L       0xC
#define LIS2DH_REG_OUT_TEMP_H       0xD
#define LIS2DH_REG_INT_COUNTER      0xE
#define LIS2DH_REG_WHO_AM_I         0xF
#define LIS2DH_REG_TEMP_CFG_REG     0x1F

#define LIS2DH_REG_CTRL_REG1        0x20
#define LIS2DH_REG_CTRL_REG2        0x21
#define LIS2DH_REG_CTRL_REG3        0x22
#define LIS2DH_REG_CTRL_REG4        0x23
#define LIS2DH_REG_CTRL_REG5        0x24
#define LIS2DH_REG_CTRL_REG6        0x25

#define LIS2DH_REG_REFERENCE        0x26
#define LIS2DH_REG_STATUS_REG       0x27

#define LIS2DH_REG_OUT_X_L          0x28
#define LIS2DH_REG_OUT_X_H          0x29
#define LIS2DH_REG_OUT_Y_L          0x2A
#define LIS2DH_REG_OUT_Y_H          0x2B
#define LIS2DH_REG_OUT_Z_L          0x2C
#define LIS2DH_REG_OUT_Z_H          0x2D

#define LIS2DH_REG_FIFO_CTRL_REG    0x2E
#define LIS2DH_REG_FIFO_SRC_REG     0x2F

#define LIS2DH_REG_INT1_CFG         0x30
#define LIS2DH_REG_INT1_SRC         0x31
#define LIS2DH_REG_INT1_THS         0x32
#define LIS2DH_REG_INT1_DURATION    0x33
#define LIS2DH_REG_INT2_CFG         0x34
#define LIS2DH_REG_INT2_SRC         0x35
#define LIS2DH_REG_INT2_THS         0x36
#define LIS2DH_REG_INT2_DURATION    0x37

#define LIS2DH_REG_CLICK_CFG        0x38
#define LIS2DH_REG_CLICK_SRC        0x39
#define LIS2DH_REG_CLICK_THS        0x3A
#define LIS2DH_REG_TIME_LIMIT       0x3B
#define LIS2DH_REG_TIME_LATENCY     0x3C
#define LIS2DH_REG_TIME_WINDOW      0x3D

#define LIS2DH_REG_ACT_THS          0x3E
#define LIS2DH_REG_ACT_DUR          0x3F

//ODR - 1, Low Power Mode - 1 and enable all 3 axis
#define LIS2DH_CTRL_REG1_VALUE      0b00101111
// Enable all interrupts
#define LIS2DH_CTRL_REG3_VALUE      0b11111110
#define LIS2DH_CTRL_REG6_VALUE      0b11111000

/** Read lis2dh.
 */
static uint8_t
lis2dh_read(uint8_t reg, uint8_t *data)
{
    const uint8_t i2c_addr = LIS2DH_I2C_ADDR << 1;
    bool succeed;

    //write sub register address
    succeed = twi_master_transfer(i2c_addr, &reg, sizeof(reg), true);
    //read value
    succeed &= twi_master_transfer(i2c_addr | TWI_READ_BIT, data, 1, true);

    if (!succeed) {
        printf("LIS2DH read failed(%#x).\n", reg);
    }

    return succeed;
}

/** Write lis2dh.
 */
static uint8_t
lis2dh_write(uint8_t reg, uint8_t value)
{
    const uint8_t i2c_addr = LIS2DH_I2C_ADDR << 1;
    uint8_t data[] = {reg, value};
    uint8_t succeed;

    succeed = twi_master_transfer(i2c_addr, data, sizeof(data), true);
    if (!succeed) {
        printf("LIS2DH write failed(%#x).\n", reg);
    }
    return succeed;
}

uint16_t
lis2dh_get_axis(uint8_t axis_reg)
{
    uint8_t high, low;

    lis2dh_read(axis_reg, &low);
    lis2dh_read(axis_reg + 1, &high);

    return (high << 8) | low;
}

void lis2dh_print_values()
{
    uint16_t x, y, z;

    x = lis2dh_get_axis(LIS2DH_REG_OUT_X_L);
    y = lis2dh_get_axis(LIS2DH_REG_OUT_Y_L);
    z = lis2dh_get_axis(LIS2DH_REG_OUT_Z_L);

    printf("X %d Y %d Z %d\n", x, y, z);
}

static app_gpiote_user_id_t  lis2dh_int_gpiote_id;

static void lis2dh_int_handler(uint32_t event_pins_low_to_high, uint32_t event_pins_high_to_low)
{
    uint8_t int_src;
    printf("event_pins_low_to_high %#lx event_pins_high_to_low %#lx \n", event_pins_low_to_high, event_pins_high_to_low);

    lis2dh_print_values();

    lis2dh_read(LIS2DH_REG_INT1_SRC, &int_src);
    lis2dh_read(LIS2DH_REG_INT2_SRC, &int_src);
}

void lis2dh_init()
{
    uint32_t interrupt_mask = (1 << LIS2DH_INT1_PIN) | (1 << LIS2DH_INT2_PIN);

    //setup interrupt handler
    nrf_gpio_cfg_sense_input(LIS2DH_INT1_PIN, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(LIS2DH_INT2_PIN, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_LOW);
    app_gpiote_user_register(&lis2dh_int_gpiote_id, interrupt_mask, interrupt_mask, lis2dh_int_handler);
    app_gpiote_user_enable(lis2dh_int_gpiote_id);

    //write configuration registers
    lis2dh_write(LIS2DH_REG_CTRL_REG1, LIS2DH_CTRL_REG1_VALUE);
    lis2dh_write(LIS2DH_REG_CTRL_REG3, LIS2DH_CTRL_REG3_VALUE);
    lis2dh_write(LIS2DH_REG_CTRL_REG6, LIS2DH_CTRL_REG6_VALUE);

    //clear interrupt
    uint8_t int_src;
    lis2dh_read(LIS2DH_REG_INT1_SRC, &int_src);
    lis2dh_read(LIS2DH_REG_INT2_SRC, &int_src);
}

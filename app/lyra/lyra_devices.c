#include <stdio.h>
#include <string.h>
#include <math.h>
#include <nrf_gpio.h>
#include <app_timer.h>
#include <app_gpiote.h>
#include <app_button.h>

#include "platform.h"
#include "board_conf.h"
#include "twi_master.h"

#define SI7013_I2C_ID                     0x40
#define SI7013_MEASURE_RH_CMD             0xE5
#define SI7013_MEASURE_TEMP_CMD           0xE3
#define SI7013_MEASURE_PREV_TEMP_CMD      0xE0

#define MAX44009_I2C_ID                   0x4A
#define MAX44009_INTR_STATUS_REG          0x0
#define MAX44009_INTR_ENABLE_REG          0x1
#define MAX44009_CONFIGURATION_REG        0x2
#define MAX44009_LUX_HIGH_REG             0x3
#define MAX44009_LUX_LOW_REG              0x4

#define MOTION_DETECTOR_GPIO_BASE         GPIO_C_BASE
#define MOTION_DETECTOR_GPIO_PIN          5
#define MOTION_DETECTOR_GPIO_PIN_MASK     (1 << MOTION_DETECTOR_GPIO_PIN)
#define MOTION_DETECTOR_PORT_NUM          GPIO_C_NUM
#define MOTION_DETECTOR_VECTOR            NVIC_INT_GPIO_PORT_C

static inline uint16_t
swap16(uint16_t word)
{
  return (word << 8) | (word >> 8);
}

/** Read current sensor.
 */
static uint8_t
i2c_read(uint8_t slave_address, uint8_t reg, uint8_t *data)
{
    bool succeed = false;

    slave_address = (slave_address << 1);

    /* set the register address */
    succeed = twi_master_transfer(slave_address, &reg, 1, true);

    succeed &= twi_master_transfer(slave_address | TWI_READ_BIT, data, 2, true);

    return succeed;
}

/*
 * Reads values from Si7013 sensor and fills the temperature and humidity values.
 */
int
read_si7013(float *temperature, int32_t *humidity)
{
  uint16_t temp_code, rh_code;

  i2c_read(SI7013_I2C_ID, SI7013_MEASURE_RH_CMD, (uint8_t *) &rh_code);
  rh_code = swap16(rh_code);
  i2c_read(SI7013_I2C_ID, SI7013_MEASURE_PREV_TEMP_CMD, (uint8_t *) &temp_code);
  temp_code = swap16(temp_code);

  *humidity = ((rh_code * 15625) >> 13) - 6000;
  *temperature = ((temp_code * 21965) >> 13) - 46850;

  return 0;
}

/*
 * Convert Lux to percentage.
 */
float
lux_to_pct(float lux)
{
  uint16_t rh_code;

  i2c_read(SI7013_I2C_ID, SI7013_MEASURE_RH_CMD, (uint8_t *) &rh_code);
  return ((125 * rh_code) / 65536) - 6;
}

/*
 * Read Ambient Light Sensor(MAX44009) value.
 */
float
get_ambient_lux()
{
  uint8_t exponent, mantissa, high, low;

  i2c_read(MAX44009_I2C_ID, MAX44009_LUX_HIGH_REG, (uint8_t *) &high);
  i2c_read(MAX44009_I2C_ID, MAX44009_LUX_LOW_REG, (uint8_t *) &low);

  exponent = (high & 0xF0) >> 4;
  mantissa = (high & 0x0F) << 4;
  mantissa |= (low & 0x0F);

  return mantissa * (1 << exponent) * 0.045f;
}

/* Add Motion Sensor handling */

/**
 * @}
 * @}
 */

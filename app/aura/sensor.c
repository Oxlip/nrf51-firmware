#include <stdio.h>
#include <string.h>
#include <ble_ss.h>
#include "twi_master.h"
#include "sensor.h"

#define TSL2561_ADDRESS 0x39
#define TSL2561_COMMAND(reg) (0x80 | reg)
void i2c_test()
{
  uint8_t i, value;
  uint16_t value16;

  //start the device
  value = 0b11;
  i2c_smb_write(TSL2561_ADDRESS, TSL2561_COMMAND(0), false, &value);

  //read the value back to confirm
  i2c_smb_read(TSL2561_ADDRESS, TSL2561_COMMAND(0), false, &value);
  printf("Power %d\n", value);

  i2c_smb_read(TSL2561_ADDRESS, TSL2561_COMMAND(0xa), false, &value);
  printf("ID %d\n", value);

  for(i=0xc; i <= 0xf; i++) {
    i2c_smb_read(TSL2561_ADDRESS, TSL2561_COMMAND(i), false, &value);
    printf("smb read (offset = %x, val=%d)\n", i, value);
  }

  i2c_smb_read(TSL2561_ADDRESS, TSL2561_COMMAND(0x20 | 0xc), true, (uint8_t*)&value16);
  printf("16bit value %d\n", value16);
}


#define CURRENT_SENSOR_ADDRESS 0x2
#define UPPER_BIT 24
#define UPPER_BIT_IDX (UPPER_BIT - 1)

double sensor_signed_to_float(uint32_t val, uint32_t pointIndex)
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



double get_inst_current(enum outlet_t outlet)
{
  int reg;
  uint32_t buf = 0;
  double res;

  if (outlet == sensorA) {
    reg = REG_IA;
  }
  else {
    reg = REG_IB;
  }

  if (!sensor_78M6610_read(CURRENT_SENSOR_ADDRESS, reg, (uint8_t*)&buf)) {
    printf("Could not get the current\n");
    return -1;
  }
  res = sensor_signed_to_float(buf, 23);
  printf("Sensor got %f\n", res);
  return res;
}


#ifdef DEBUG

void test_conversion(void)
{
  int i, j;
  unsigned int val[] =
    {
      0x200000, 0x600000, 0x700000, 0x780000, 0x740000, 0x640000, 0x640110,
      0xA00000, 0xF00000
    };
  unsigned int indexes[] = { 21, 23 };

  for (i = 0; i < sizeof (indexes) / sizeof (unsigned int); i++) {
    printf("Running tests with index: %d\n", indexes[i]);
    for (j = 0; j < sizeof (val) / sizeof (unsigned int); j++) {
      printf("val = 0x%x -> %f\n", val[j], sensor_signed_to_float(val[j], indexes[i]));
    }
  }
}


#endif /* !DEBUG */


uint8_t
sensor_78M6610_read(uint8_t slave_address, uint8_t reg, uint8_t *data)
{
  bool succeed = false;

  slave_address = (slave_address << 1);

  /* set the register address */
  succeed = twi_master_transfer(slave_address, &reg, 1, true);

  succeed &= twi_master_transfer(slave_address | TWI_READ_BIT,
				 data, 3, true);

  return succeed;
}

uint8_t
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


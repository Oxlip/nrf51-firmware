#include <stdio.h>
#include "twi_master.h"


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


#define CURRENT_SENSOR_ADDRESS 0x10	// FIXME: put good address
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

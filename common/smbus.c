#include <twi_master.h>
#include <nrf_gpio.h>
#include <boards.h>
#include <stdio.h>


bool scan_bus(void)
{
  uint8_t addr = 0, data = 0, data_lenght = 1;
  int found = 0;

  for (addr = 0; addr < 128; addr++) {
    if (twi_master_transfer(addr << 1 | TWI_READ_BIT, &data, data_lenght, false)) {
      found = 1;
      printf("Found device on address %u\n", addr);
    }
  }
  if (!found) {
    printf("Failed to find any devices on the bus!\n");
  }
  return found;

}



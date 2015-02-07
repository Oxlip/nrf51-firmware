#ifndef __SENSOR__
#define __SENSOR__

enum outlet_t {
  sensorA,
  sensorB
};

enum outlet_reg_t {
  REG_IA = 44,
  REG_IB = 45,
};

void i2c_test();
double sensor_signed_to_float(uint32_t val, uint32_t pointIndex);
uint8_t sensor_78M6610_read(uint8_t slave_address, uint8_t reg, uint8_t *data);
uint8_t sensor_78M6610_write(uint8_t slave_address, uint8_t reg, uint8_t *data);
double get_inst_current(enum outlet_t outlet);


#endif /* !__SENSOR__ */

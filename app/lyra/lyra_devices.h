#ifndef __LYRA_DEVICES_H__
#define __LYRA_DEVICES_H__

int
read_si7013(float *temperature, int32_t *humidity);

float
lux_to_pct(float lux);

float
get_ambient_lux();

#endif /* __LYRA_DEVICES_H__ */

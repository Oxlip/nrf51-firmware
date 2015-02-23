#ifndef __SENSOR__
#define __SENSOR__

double sensor_get_instant_current(int outlet_nuber);

double cs_get_rms_current(int outlet_nuber);
double cs_get_rms_voltage(int outlet_nuber);
double cs_get_active_watts(int outlet_nuber);

#endif /* !__SENSOR__ */

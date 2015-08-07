#ifndef __CS_78M6610__
#define __CS_78M6610__s

bool cs_calibrate();
uint32_t cs_get_status();
double cs_get_rms_current(int outlet_nuber);
double cs_get_rms_voltage(int outlet_nuber);
double cs_get_active_watts(int outlet_nuber);
double cs_get_peak_current(int outlet_nuber);
double cs_get_peak_voltage(int outlet_nuber);
double cs_get_line_frequency();

#endif /* !__CS_78M6610__ */

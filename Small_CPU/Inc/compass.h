/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef COMPASS_H
#define COMPASS_H

#include <stdint.h>

void compass_init(uint8_t fast, uint8_t gain);
void accelerator_init(void);
void compass_read(void);
void acceleration_read(void);
int compass_calib(void);
void compass_calc(void);
//void compass_calc_mini_during_calibration(void);
 
float check_compass_calib(void);

void compass_sleep(void);
void accelerator_sleep(void);

#endif /* COMPASS_H */

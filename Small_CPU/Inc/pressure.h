/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef PRESSURE_H
#define PRESSURE_H

#include <stdint.h>

uint8_t init_pressure(void);
uint8_t pressure_update(void);
void pressure_update_alternating(void);

uint8_t is_init_pressure_done(void);

HAL_StatusTypeDef  pressure_sensor_get_pressure_raw(void);
HAL_StatusTypeDef  pressure_sensor_get_temperature_raw(void);
void pressure_calculation(void);

float get_temperature(void);
float get_pressure_mbar(void);
float get_surface_mbar(void);

void init_surface_ring(void);
void update_surface_pressure(uint8_t call_rhythm_seconds);

uint32_t demo_modify_temperature_and_pressure(int32_t divetime_in_seconds, uint8_t subseconds, float ceiling_mbar);

#endif /* PRESSURE_H */


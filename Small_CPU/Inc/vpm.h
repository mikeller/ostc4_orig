/*
 * vpm.h
 *
 * Created: 23.04.2013 11:38:19
 *  Author: produktion04
 */


#ifndef VPM_H
#define VPM_H

#include "data_central.h"
extern long vpm_time_calc_begin;
enum DECOLIST{DECOSTOPS,FUTURESTOPS,BAILOUTSTOPS,NULLZEIT,OFF = -1}; // order is important!!
enum VPM_CALC_STATUS{CALC_END, CALC_BEGIN, CALC_CRITICAL, CALC_FINAL_DECO, CALC_NULLZEIT };
//int calc_crushing_pressure(float *starting_depth, float *ending_depth, float *rate);
//void vpm_check_calc(unsigned short* stoplist);
//void vpm_init(void);
//_Bool vpm_crush(void);

void vpm_reset_variables(void);
_Bool vpm_build_variables_from_file(unsigned long in_sdram_start);
int vpm_store_variables_in_sdram_for_transfer(unsigned long in_sdram_start);
void vpm_calc_crushing_pressure(float starting_ambient_pressure, float ending_ambient_pressure, float rate);
void vpm_deco_plan(unsigned short divetime,unsigned short * divetime_first_stop, float* first_stop_depth);

float schreiner_equation__2(float *initial_inspired_gas_pressure,float *rate_change_insp_gas_pressure,float *interval_time_minutes,  const float *gas_time_constant,float *initial_gas_pressure);

int  vpm_calc(SLifeData* pINPUT, SVpm* pVPM, SDecoinfo*  pDECOINFO);
void  vpm_saturation_after_ascent(SLifeData* input);
extern  const float helium_time_constant[16];
extern  const float nitrogen_time_constant[16];

#endif /* VPM_H */

///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Inc/tCCR.h
/// \brief
/// \author heinrichs weikamp gmbh
/// \date   16-Dec-2014
///
/// $Id$
///////////////////////////////////////////////////////////////////////////////
/// \par Copyright (c) 2014-2018 Heinrichs Weikamp gmbh
///
///     This program is free software: you can redistribute it and/or modify
///     it under the terms of the GNU General Public License as published by
///     the Free Software Foundation, either version 3 of the License, or
///     (at your option) any later version.
///
///     This program is distributed in the hope that it will be useful,
///     but WITHOUT ANY WARRANTY; without even the implied warranty of
///     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
///     GNU General Public License for more details.
///
///     You should have received a copy of the GNU General Public License
///     along with this program.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////////////

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef TCCR_H
#define TCCR_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported functions --------------------------------------------------------*/

void tCCR_init(void);
void tCCR_control(void);
void tCCR_restart(void);
void tCCR_tick(void);

void tCCR_SetRXIndication(void);

float get_ppO2Sensor_bar(uint8_t sensor_id);
float get_sensorVoltage_mV(uint8_t sensor_id);
float get_HUD_battery_voltage_V(void);
uint8_t get_ppO2SensorWeightedResult_cbar(void);
void test_HUD_sensor_values_outOfBounds(int8_t * outOfBouds1, int8_t * outOfBouds2, int8_t * outOfBouds3);

#endif /* TCCR_H */

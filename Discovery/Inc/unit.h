///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Inc/unit.h
/// \brief  input to meter/celsius or feet/farenheit
/// \author heinrichs weikamp gmbh
/// \date   24-Feb-2015
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
#ifndef UNIT_H
#define UNIT_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/** @addtogroup Template
	* @{
	*/

/* Exported variables --------------------------------------------------------*/


/* Exported functions --------------------------------------------------------*/
float unit_depth_float(float input_meter);
uint16_t unit_depth_integer(uint16_t input_meter);
float unit_temperature_float(float input_celsius);
uint16_t unit_temperature_integer(uint16_t input_celsius);
uint16_t unit_speed_integer(uint16_t input_meterPerMinute);
int unit_SeaLevelRelation_integer(int input_atmospheric_mbar);
char unit_depth_char1(void);
char unit_depth_char2(void);
char unit_depth_char1_T105(void);
char unit_depth_char2_T105(void);

#ifdef __cplusplus
}
#endif

#endif /* UNIT_H */

/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/


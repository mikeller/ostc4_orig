///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Inc/bonex_mini.h
/// \brief  voltage to battery percentage
/// \author heinrichs weikamp gmbh
/// \date   26-March-2017
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

#ifndef BONEX_MINI_H
#define BONEX_MINI_H

#include "stdio.h"
#include "stm32f4xx_hal.h"


void BONEX_calc_new_ResidualCapacity(uint8_t * residualC, uint32_t voltage_mV, int32_t current_mA, uint8_t scooterType);
uint8_t BONEX_mini_ResidualCapacityVoltageBased(float voltage_V, uint16_t ageInMilliSecondsSinceLast);

#endif // BONEX_MINI_H

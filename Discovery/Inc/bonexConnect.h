///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Inc/bonexConnect.h
/// \brief  connect to bluetooth LTE of BonexInfoSystem
/// \author heinrichs weikamp gmbh
/// \date   29-Sept-2015
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
#ifndef BONEX_CONNECT_H
#define BONEX_CONNECT_H

#include "stm32f4xx_hal.h"

enum
{
	BC_DISCONNECTED 	= 0,
	BC_SEARCHING 			= 1,
	BC_CONNECTED 			= 2,
};

enum
{
	BONEX_OK = 0,		//= HAL_OK
	BONEX_BUS_ERROR 	= (uint8_t)HAL_ERROR,
	BONEX_BUS_BUSY 		= (uint8_t)HAL_BUSY,
	BONEX_BUS_TIMEOUT = (uint8_t)HAL_TIMEOUT,
	BONEX_NOTFOUND,
	BONEX_TYPEMISMATCH,
	BONEX_TIMEOUT,
	BONEX_BUSY,
	BONEX_NOCONNECT,
};

void bonexControl(void);
void bC_setConnectRequest(void);
uint8_t bC_getName(char *name);
uint8_t bC_getStatus(void);
uint8_t bC_getData(float *watt, float *temperature, uint16_t *drehzahl, uint8_t *residualcapacity);

#endif // BONEX_CONNECT_H

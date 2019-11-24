///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Inc/t6_apnea.h
/// \brief  header for dive screen for Gauge mode
/// \author heinrichs weikamp gmbh
/// \date   1-Feb-2017
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
#ifndef T6_H
#define T6_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "gfx_engine.h"

/* Exported functions --------------------------------------------------------*/
void t6_init(void);
void t6_refresh(void);
void t6_change_customview(uint8_t action);

#endif /* T6_H */

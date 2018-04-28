///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Inc/t9.h
/// \brief  Header file of Divemode with 9 windows, based on t7
/// \author heinrichs weikamp gmbh
/// \date   7-July-2016
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
#ifndef T9_H
#define T9_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "gfx_engine.h"

/* Exported functions --------------------------------------------------------*/
void t9_init(void);

void t9_refresh(void);
void t9_refresh_sleepmode_fun(void);
void t9_refresh_customview_old(void);

void t9_change_field(void);
void t9_change_customview(void);

void t9_set_field_to_primary(void);
void t9_set_customview_to_primary(void);

void init_t9_compass(void);

/*
	 void t9c_refresh(uint32_t FramebufferStartAddress);
*/

#endif /* T9_H */

///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Inc/firmwareJumpToApplication.h
/// \brief  jump to application in higher flash region
/// \author heinrichs weikamp gmbh
/// \date   05-May-2015
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
#ifndef FIRMWARE_JUMP_TO_APPLICATION_H
#define FIRMWARE_JUMP_TO_APPLICATION_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

uint8_t firmware_MainCodeIsProgammed(void);
void firmware_JumpTo_Application(void);

#endif // FIRMWARE_JUMP_TO_APPLICATION_H

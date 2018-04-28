///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Inc/externCPU2bootloader.h
/// \brief  Header File to communicate with the second CPU in bootloader mode
/// \author heinrichs weikamp gmbh
/// \date   23-Oct-2014
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
#ifndef EXTERN_CPU2_BOOTLOADER_H
#define EXTERN_CPU2_BOOTLOADER_H

/* Includes ------------------------------------------------------------------*/

#include "stm32f4xx_hal.h"

/* Exported functions --------------------------------------------------------*/

uint8_t extCPU2bootloader_start(uint8_t *version, uint16_t *chipID);
void extCPU2bootloader_continue(void);
uint8_t extCPU2bootloader( uint8_t* buffer, uint16_t length, char* display_text);

#endif /* EXTERN_CPU2_BOOTLOADER_H */

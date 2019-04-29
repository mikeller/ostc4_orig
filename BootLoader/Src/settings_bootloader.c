///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   BootLoader/Src/settings_bootloader.c
/// \brief  mini version for firmwareDataGetPointer() etc.
/// \author heinrichs weikamp gmbh
/// \date   21-March-2016
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

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "settings.h"

/* always at 0x8080000, do not move -> bootloader access */
const SFirmwareData* firmwareDataGetPointer(void)
{
	return (const SFirmwareData*)(0x08040000 + 0x00010000);
}


void getActualRTEandFONTversion(uint8_t *RTEhigh, uint8_t *RTElow, uint8_t *FONThigh, uint8_t *FONTlow)
{
	if(RTEhigh && RTElow)
	{
		*RTEhigh = 0;
		*RTElow = 0;
	}
	if(FONThigh && FONTlow)
	{
		*FONThigh = *(uint8_t *)0x08132000;
		*FONTlow = *(uint8_t *)0x08132001;
	}
}


uint8_t getLicence(void)
{
//return 0xFF;
//return LICENCEBONEX;
	return hardwareDataGetPointer()->primaryLicence;
}

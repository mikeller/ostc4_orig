///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   BootLoader/Inc/tInfoBootloader.h
/// \brief  Header file communication with PC
/// \author heinrichs weikamp gmbh
/// \date   08-Aug-2014
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
#ifndef TINFO_BOOTLOADER_H
#define TINFO_BOOTLOADER_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported functions --------------------------------------------------------*/

void tInfoBootloader_init(void);
void tInfo_newpage(const char * text);
void tInfo_write(const char * text);
void tInfo_button_text(const char *text_left, const char *text_mid, const char *text_right);

#endif /* TINFO_BOOTLOADER_H */

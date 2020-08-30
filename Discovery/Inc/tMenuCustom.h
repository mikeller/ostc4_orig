///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Inc/tMenuCustom.h
/// \brief  Menu Custom View - Provide access to custom view options
/// \author heinrichs weikamp gmbh
/// \date   25-Aug-2020
///
/// $Id$
///////////////////////////////////////////////////////////////////////////////
/// \par Copyright (c) 2014-2020 Heinrichs Weikamp gmbh
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
#ifndef TMENU_CUSTOM_H
#define TMENU_CUSTOM_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/** @addtogroup Template
	* @{
	*/

/* Exported variables --------------------------------------------------------*/


/* Exported functions --------------------------------------------------------*/

uint32_t tMCustom_refresh(uint8_t line, char *text, uint16_t *tab, char *subtext);

	 #ifdef __cplusplus
}
#endif

#endif /* TMENU_CUSTOM_H */

/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/

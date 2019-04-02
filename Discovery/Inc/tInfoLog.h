///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Inc/tInfoLog.h
/// \brief  Header file of Menu Lines for Deco settings
/// \author heinrichs weikamp gmbh
/// \date   31-July-2014
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
#ifndef TINFO_LOG_H
#define TINFO_LOG_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Exported functions --------------------------------------------------------*/

void tInfoLog_init(void);
void openLog(_Bool recallKeepPage);
void openInfoLogLastDive(void);
void sendActionToInfoLogList(uint8_t sendAction);
void sendActionToInfoLogShow(uint8_t sendAction);
void exitLog(void);

#endif /* TINFO_LOG_H */

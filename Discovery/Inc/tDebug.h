///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Inc/tDebug.h
/// \brief  Screen with Terminal Out
/// \author heinrichs weikamp gmbh
/// \date   06-April-2016
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
#ifndef TDEBUG_H
#define TDEBUG_H

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>

/* Exported functions --------------------------------------------------------*/
uint8_t inDebugMode(void);
void setDebugMode(void);
void exitDebugMode(void);

void tDebug_start(void);
void tDebug_refresh(void);
void tDebugControl(uint8_t sendAction);
void tDebug_action(void);
void tDebug_exit(void);

#endif /* TDEBUG_H */

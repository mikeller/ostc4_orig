///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Inc/buehlmann.h
/// \brief	ZHL16+GF decompression algorithm.
/// \author Heinrichs Weikamp
/// \date   2018
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

#ifndef BUEHLMANN_H
#define BUEHLMANN_H

#include "data_central.h"

void buehlmann_init(void);
void buehlmann_calc_deco(SLifeData* pLifeData, SDiveSettings * pDiveSettings, SDecoinfo * pDecoInfo);
void buehlmann_ceiling_calculator(SLifeData* pLifeData, SDiveSettings * pDiveSettings, SDecoinfo * pDecoInfo);
void buehlmann_super_saturation_calculator(SLifeData* pLifeData, SDecoinfo * pDecoInfo);
float buehlmann_get_gCNS(void);

#endif /* BUEHLMANN_H */

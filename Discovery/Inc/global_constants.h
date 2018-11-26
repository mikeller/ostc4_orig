///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Common/Inc/global_constants.h
/// \brief
/// \author Dmitry Romanov<kitt@bk.ru>
/// \date   11.2018
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

#ifndef GLOBAL_CONSTANTS_H
#define GLOBAL_CONSTANTS_H
//Rename it ahead.
#define DEFAULT_BUTTONRESPONSIVENESS_GUI 90
#define MIN_BUTTONRESPONSIVENESS_GUI 50 //50 (-10correction)
#define MAX_BUTTONRESPONSIVENESS_GUI 110//100 (+10correction)
#define MIN_BUTTONRESPONSIVENESS MIN_BUTTONRESPONSIVENESS_GUI-20 //MIN_BUTTONRESPONSIVENESS_GUI-10 correction
#define MAX_BUTTONRESPONSIVENESS MAX_BUTTONRESPONSIVENESS_GUI+20//MIN_BUTTONRESPONSIVENESS_GUI+10correction

#endif

/**
  ******************************************************************************
  * @file    gfx_fonts.c
  * @author  heinrichs/weikamp, JD Gascuel
  * @version V1.4.0
  * @date    17-Decembre-2017
  * @brief   Font data (and images) stored in UPPER ROM.
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 heinrichs weikamp</center></h2>
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "gfx_fonts.h"

//#ifdef DEBUG
# 	define INCLUDE_FONTS_BINARY
//#endif

#ifdef INCLUDE_FONTS_BINARY

/* Fonts fixed in upper region */
#include "Fonts/font_awe48.h"
#include "Fonts/font_T24.h"
#include "Fonts/font_T42.h"
#include "Fonts/font_T48_plus.h"
#include "Fonts/font_T54.h"
#include "Fonts/font_T84.h"
#include "Fonts/font_T105.h"
#include "Fonts/font_T144_plus.h"

/* Images fixed in upper region */
#include "Fonts/image_battery.h"
#include "Fonts/image_heinrichs_weikamp.h"
#include "Fonts/image_ostc.h"

#endif // INCLUDE_FONTS_BINARY

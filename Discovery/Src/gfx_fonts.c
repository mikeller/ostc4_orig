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

/* Fonts in the firmware image */
#include "Fonts/font_T54_extra.h"
#include "Fonts/font_T84_extra.h"
#include "Fonts/font_T105_extra.h"

#ifdef DEBUG
# 	define INCLUDE_FONTS_BINARY
#endif

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
#else

/* Fonts fixed in upper region */
const tFont Awe48 __attribute__ (( used, section(".upper_font_directory.Awe48") ));
const tFont FontT24 __attribute__ (( used, section(".upper_font_directory.FontT24") ));
const tFont FontT42 __attribute__ (( used, section(".upper_font_directory.FontT42") ));
const tFont FontT48 __attribute__ (( used, section(".upper_font_directory.FontT48") ));
const tFont FontT54 __attribute__ (( used, section(".upper_font_directory.FontT54") ));
const tFont FontT84 __attribute__ (( used, section(".upper_font_directory.FontT84") ));
const tFont FontT84Spaced __attribute__ (( used, section(".lower_font_directory.FontT84Spaced") ));
const tFont FontT105 __attribute__ (( used, section(".upper_font_directory.FontT105") ));
const tFont FontT144 __attribute__ (( used, section(".upper_font_directory.FontT144") ));

/* Images fixed in upper region */
#include "Fonts/image_battery.h"
#include "Fonts/image_heinrichs_weikamp.h"
#include "Fonts/image_ostc.h"


#endif // INCLUDE_FONTS_BINARY

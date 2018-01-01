/**
  ******************************************************************************
	* @copyright heinrichs weikamp
  * @file   		base_upperRegion.c
  * @author 		heinrichs/weikamp, Christian Weikamp
  * @date   		31-August-2015
  * @version		V0.0.3
  * @since			03-Dez-2016
  * @brief			The beginning of it all. main() is part of this.
	* @bug
	* @warning
  @verbatim
  ==============================================================================
              ##### New characters in fonts #####
  ==============================================================================
  [..] Use font_tmore.c and add line to corresponding font like font_t54c
			 Don't forget to adjust the length of the font in the last line starting
			 const tFont .....

	[..] last char vor der gro�en L�cke, Stand 160217
			 image_data_FontT24_0x002b[364]
			 __attribute__((at( START_T24_FONT + (1647 * 28) ))), START_T24_FONT  (0x08142F00 - MINUS_BANK)

			 -> frei ab 0x0814E490
			 geht dann weiter mit image_data_ostc_fuer_Tauchcomputer_240px
			 sind ca. 120 kByte frei!
			 
	 @endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "FirmwareData.h"
#include "gfx_fonts.h"

#include "stm32f4xx_hal.h"

const SFirmwareData font_FirmwareData   __attribute__(( section(".upper_firmware_data") )) =
{
    .versionFirst   = 0,
    .versionSecond  = 9,
    .versionThird	= 0,
    .versionBeta	= 0,

    /* 4 bytes, including trailing 0 */
    .signature      = "cw",

    .release_year   = 16,
    .release_month  = 1,
    .release_day    = 13,
    .release_sub    = 0,

    /* max 48, including trailing 0 */
    .release_info   ="",

    /* for safety reasons and coming functions */
    .magic[0] = FIRMWARE_MAGIC_FIRST,
    .magic[1] = FIRMWARE_MAGIC_SECOND,
    .magic[2] = FIRMWARE_MAGIC_FONT, /* the magic byte for fonts*/
    .magic[3] = FIRMWARE_MAGIC_END
};

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

int main(void)
{
    return 0;
}

/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/

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

	[..] last char vor der großen Lücke, Stand 160217 
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
#include "stdio.h"
#include <string.h> // for memcopy

#include "stm32f4xx_hal.h"
#include "gfx_engine.h"

/* HINT chsw 160114
 * using Jef Driesen dctool.exe I found out that from 150K to 230K there is only '0's!!!
 */
 
#define VARIABLE_UPPER_REGION
//#define FIXED_UPPER_REGION

#ifdef VARIABLE_UPPER_REGION
//static const uint8_t image_data_HelloWorld[] __attribute__((at(0x08132000 )))  = "Hello World    ";

typedef struct
{
	uint8_t versionFirst;
	uint8_t versionSecond;
	uint8_t versionNOTUSED;
	uint8_t versionBeta;
	uint8_t signature[4];
	uint8_t release_year;
	uint8_t release_month;
	uint8_t release_day;
	uint8_t release_sub;
	char release_info[48];
	char dummy[4];
} SHelloWorldData;

static const SHelloWorldData FirmwareData __attribute__((at(0x08132000))) = {

	.versionFirst		= 0,
	.versionSecond	= 9,
	.versionNOTUSED	= 0,
	.versionBeta		= 0,

	/* 4 bytes with trailing 0 */
	.signature = "cw",
	
	.release_year = 16,
	.release_month = 1,
	.release_day = 13,
	.release_sub = 0,
	
	/* max 48 with trailing 0 */
	//release_info ="12345678901234567890123456789012345678901"
	.release_info  ="",
	
	/* for safety reasons and coming functions */
	.dummy[0] = 0,
	.dummy[1] = 0,
	.dummy[2] = 0xF0, /* the magic byte for fonts*/
	.dummy[3] = 0xFF
};
static const uint8_t image_data_end_of_header[] __attribute__((at(0x0813217e - 5 )))  = "chsw";


// at the moment 09. Sept. 2015, font_T144_plus is the first code starting at 0x0813217e
#include "font_T144_plus.c"			// 0x0813217e, max. 69 kB
#include "font_T24.c"						// 0x08142F00  max. 47 kB
// grosse Lücke von 120 kB
#include "image_ostc.c"					// 0x0816BAD2
#include "font_T84.c" 					// 0x08170000
#include "font_T54.c"						// 0x081762EE
#include "font_T105.c"					// 0x0817bd00
// der letzte Font ist T42, dahinter ist Platz von 0x081aa5a0 (Stand 151214) bis 0x081DFFFF
#include "font_T42.c"						// 0x0818f9c0
// starts at BASE_TMORE = 0x081aa5a0, defined in fontT_config.h
#include "font_Tmore.c"					// 0x081aa5a0


// im regulären unteren Speicher
// for checked / unchecked only at the moment
//#include "font_awe48.c"
//#include "battery2_complete.c"
#endif

// starts at 0x081E0000
#ifdef FIXED_UPPER_REGION
#include "font_T48_plus.c"
#include "image_heinrichs_weikamp.c" /* with no fixed region color lookup table */
#endif


int main(void) //__attribute__((at(0x08000000)))
{
}
	

/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/

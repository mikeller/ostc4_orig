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

// From Common/Inc:
#include "FirmwareData.h"

// From Discovery/Inc
#include "gfx_fonts.h"

// From AC6 support:
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////

static int errors = 0;
#define ASSERT(e) \
    do { if( ! (e) ) {++errors; printf("FAIL at %3d: %s", __LINE__, #e);}} while(0)

#define ASSERT_RANGE(e, min, max) \
    ASSERT(min <= e); ASSERT( e <= max)

int main(void)
{
    //---- Check the linker puts the directory at the requested address ------
    ASSERT( & Awe48    == (tFont*)0x8100000 );
    ASSERT( & FontT24  == (tFont*)0x810000c );
    ASSERT( & FontT42  == (tFont*)0x8100018 );
    ASSERT( & FontT48  == (tFont*)0x8100024 );
    ASSERT( & FontT54  == (tFont*)0x8100030 );
    ASSERT( & FontT84  == (tFont*)0x810003c );
    ASSERT( & FontT105 == (tFont*)0x8100048 );
    ASSERT( & FontT144 == (tFont*)0x8100052 );

    //---- Check the linker puts the font data in the requested section ------
    extern tChar __upper_font_data;
    extern tChar __upper_font_data_end;
    ASSERT( &__upper_font_data == (tChar*)0x08132040 );
    ASSERT_RANGE( (int)&__upper_font_data_end, 0x08132040, 0x081E0000);

    //---- Walk through the directory data -----------------------------------
    extern const tFont __font_directory;
    extern const tFont __font_directory_end;
    for(const tFont* font = & __font_directory; font < &__font_directory_end; ++font)
    {
        // Check END-OF-DIRECTORY magic marker
        if( font->length == (uint32_t)-1 )
        {
            ASSERT( font == &FontT144 + 1 );
            break;
        }

        // Check font descriptors are inside a safe range.
        ASSERT_RANGE( font->length,               10, 103 );
        ASSERT_RANGE( font->spacesize,             0,  18 );
        ASSERT_RANGE( font->spacesize2Monospaced, 13,  72 );
        ASSERT_RANGE( font->height,               28, 108 );

        //---- Walk through each char ----------------------------------------
        for(int i = 0; i < font->length; ++i)
        {
            const tChar* c = &font->chars[i];

            // Check char data is indeed stored in the actual data section
            ASSERT_RANGE( c, &__upper_font_data, &__upper_font_data_end);

            // Check char data sanity
            ASSERT_RANGE( c->code, 0x0000, 0xF143);

            // Check image sanity
            const tImage* image = c->image;
            ASSERT_RANGE(image,  (tImage*)&__upper_font_data, (tImage*)&__upper_font_data_end);
            ASSERT_RANGE(image->width, font->spacesize, font->spacesize2Monospaced);
            ASSERT(image->height == font->height);

            // Uncompress image bytes
            const uint8_t* byte = image->data;
            ASSERT_RANGE(byte,  (uint8_t*)&__upper_font_data, (uint8_t*)&__upper_font_data_end);

            for(int w=0; w <image->width; ++w)
            {
                // Compression: special 0x01 byte at start of column means just skip it.
                if( *byte++ == 0x01 )
                    continue;

                int zeros = (byte[-1] == 0x00) ? 1 : 0;
                for(int h = 1; h < image->height; ++h)
                {
                    if( *byte == 0x00 )
                        ++zeros;

                    // Other bytes cannot have the 0x01 value...
                    ASSERT( *byte++ != 0x01 );
                }

                if( zeros == image->height )
                    printf("Font[%d] char[%d]: could skip column %d",
                            &__font_directory - font, i, w);
            }

            // Check the byte stream do not collide with the next char,
            // or with the first tImage struct of the font.
            if( (i+1) < font->length )
                ASSERT( byte < font->chars[i+1].image->data );
            else
                ASSERT( byte < (uint8_t*)font->chars[0].image );

            // TODO: check image bytes are contiguous between chars.
        }
    }

    if( errors )
    {
        printf("Font Check: %d errors.", errors);
        return -1;
    }

    printf("Font Check: no errors.");
    return 0;
}

/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/

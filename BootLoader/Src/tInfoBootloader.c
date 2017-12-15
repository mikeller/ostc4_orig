/**
  ******************************************************************************
  * @file    tInfoBootloader.c
  * @author  heinrichs/weikamp, Christian Weikamp
  * @version V0.0.1
  * @date    08-May-2015
  * @brief   Write something on the screen in between steps
  *
  @verbatim
  ==============================================================================
                        ##### How to use #####
  ==============================================================================
	* a little bit of text (DMA is not running for fast clean)
	@endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 heinrichs weikamp</center></h2>
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "base_bootloader.h"
#include "ostc.h"
#include "tInfoBootloader.h"
#include "gfx_engine.h"
#include "gfx_colors.h"
/* Exported variables --------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

	GFX_DrawCfgScreen	tIBscreen;
	GFX_DrawCfgWindow	tIBwindow;
	uint8_t line = 1;
	
	char textButtonLeft[30] = { 0 };
	char textButtonMid[31] = { 0 };
	char textButtonRight[31] = { 0 };

/* Private types -------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

void tInfoBootloader_init(void)
{
	tIBscreen.FBStartAdress = 0;
	tIBscreen.ImageHeight = 480;
	tIBscreen.ImageWidth = 800;
	tIBscreen.LayerIndex = 1;

	tIBwindow.Image = &tIBscreen;
	tIBwindow.WindowNumberOfTextLines = 6;
	tIBwindow.WindowLineSpacing = 65;
	tIBwindow.WindowTab = 400;
	tIBwindow.WindowX0 = 20;
	tIBwindow.WindowX1 = 779;
	tIBwindow.WindowY0 = 0;
	tIBwindow.WindowY1 = 799;
	
	line = 1;
}


void tInfo_button_text(const char *text_left, const char *text_mid, const char *text_right)
{
	if(text_left)
		strncpy(textButtonLeft,text_left,30);
	if(text_mid)
	{
		textButtonMid[0] = '\001';
		strncpy(&textButtonMid[1],text_mid,30);
	}
	if(text_right)
	{
		textButtonRight[0] = '\002';
		strncpy(&textButtonRight[1],text_right,30);
	}
}


void tInfo_newpage(const char *text)
{
	uint32_t backup  = tIBscreen.FBStartAdress;
	
	tIBscreen.FBStartAdress = getFrame(18);
	line = 1;
	if(text)
		GFX_write_string(&FontT48, &tIBwindow, text,line);
	line++;

	if(*textButtonLeft)
		write_content_simple(&tIBscreen, 0, 800, 480-24, &FontT24,textButtonLeft,CLUT_ButtonSurfaceScreen);
	if(*textButtonMid)
		write_content_simple(&tIBscreen, 0, 800, 480-24, &FontT24,textButtonMid,CLUT_ButtonSurfaceScreen);
	if(*textButtonRight)
		write_content_simple(&tIBscreen, 0, 800, 480-24, &FontT24,textButtonRight,CLUT_ButtonSurfaceScreen);
	
	GFX_SetFrameTop(tIBscreen.FBStartAdress);
	GFX_change_LTDC();
	
	if(backup != 0)
			releaseFrame(18,backup);
}


void tInfo_write(const char *text)
{
	if((line > 6) || (tIBscreen.FBStartAdress == 0))
		tInfo_newpage(text);
	else
	{
		if(text)
			GFX_write_string(&FontT48, &tIBwindow, text,line);
		line++;

	}	
}

/* Private functions ---------------------------------------------------------*/

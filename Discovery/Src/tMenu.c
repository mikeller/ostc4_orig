///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Src/tMenu.c
/// \brief  Major menu with extra page 0 for edit functionality since V0.0.2
/// \author heinrichs weikamp gmbh
/// \date   30-April-2014
///
/// \details
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

/* Includes ------------------------------------------------------------------*/
#include "tMenu.h"

#include "gfx_fonts.h"
#include "tHome.h"
#include "tMenuDeco.h"
#include "tMenuDecoParameter.h"
#include "tMenuEditDeco.h"
#include "tMenuEditDecoParameter.h"
#include "tMenuEditGasOC.h"
#include "tMenuEditHardware.h"
#include "tMenuEditPlanner.h"
#include "tMenuEditSetpoint.h"
#include "tMenuEditSystem.h"
#include "tMenuEditXtra.h"
#include "tMenuGas.h"
#include "tMenuHardware.h"
#include "tMenuPlanner.h"
#include "tMenuSetpoint.h"
#include "tMenuSystem.h"
#include "tMenuXtra.h"

/* Private types -------------------------------------------------------------*/
#define MAXPAGES 10

typedef struct
{
    uint32_t	StartAddressForPage[MAXPAGES+1];
    uint8_t		lineMemoryForNavigationForPage[MAXPAGES+1];
    uint8_t		pageMemoryForNavigation;
    uint8_t		linesAvailableForPage[MAXPAGES+1];
    uint8_t 	pagesAvailable;
    uint8_t 	pageCountNumber[MAXPAGES+1];
    uint8_t 	pageCountTotal;
    uint8_t		modeFlipPages;
} SMenuMemory;

/* Exported variables --------------------------------------------------------*/

/* Announced Private variables -----------------------------------------------*/
GFX_DrawCfgScreen	tMdesignSolo;
GFX_DrawCfgScreen	tMdesignCursor;

/* Private variables ---------------------------------------------------------*/
GFX_DrawCfgWindow	tMwindow;
GFX_DrawCfgScreen	tMscreen;

uint32_t FramebufferStartAddressForPage[10];

SMenuMemory	menu;

uint32_t callerID;

uint8_t actual_menu_content  = MENU_UNDEFINED;

/* TEM HAS TO MOVE TO GLOBAL--------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
void draw_tMheader(uint8_t page);
void draw_tMcursorDesign(void);

void draw_tMdesignSubUnselected(uint32_t *ppDestination);
void draw_tMdesignSubSelected(uint32_t *ppDestination);
void draw_tMdesignSubSelectedBorder(uint32_t *ppDestination);
void tMenu_write(uint8_t page, char *text, char *subtext);

void clean_line_actual_page(void);
void tM_build_pages(void);

void gotoMenuEdit(void);

/* Exported functions --------------------------------------------------------*/

GFX_DrawCfgScreen * get_PointerMenuCursorScreen(void)
{
    return &tMdesignCursor;
}


GFX_DrawCfgScreen * get_PointerMenuCursorDesignSoloScreen(void)
{
    return &tMdesignSolo;
}


void nextline(char * text, uint8_t *textPointer)
{
    text[(*textPointer)++] = '\n';
    text[(*textPointer)++] = '\r';
    text[*textPointer] = 0;
}


void tM_init(void)
{
    uint8_t i;

    tMdesignCursor.FBStartAdress = getFrame(3);
    tMdesignCursor.ImageHeight = 390;
    tMdesignCursor.ImageWidth = 800;
    tMdesignCursor.LayerIndex = 0;

    tMdesignSolo.FBStartAdress = getFrame(4);
    tMdesignSolo.ImageHeight = 390;
    tMdesignSolo.ImageWidth = 800;
    tMdesignSolo.LayerIndex = 0;

    menu.pagesAvailable = 0;
    menu.pageMemoryForNavigation = 0;
    for(i=0;i<=MAXPAGES;i++)
    {
        menu.lineMemoryForNavigationForPage[i] = 0;
        menu.StartAddressForPage[i] = 0;
        menu.linesAvailableForPage[i] = 0;
    }

    tMscreen.FBStartAdress = 0;
    tMscreen.ImageHeight = 480;
    tMscreen.ImageWidth = 800;
    tMscreen.LayerIndex = 1;

    draw_tMcursorDesign();

    tMwindow.Image = &tMscreen;
    tMwindow.WindowNumberOfTextLines = 6;
    tMwindow.WindowLineSpacing = 65;
    tMwindow.WindowTab = 400;
    tMwindow.WindowX0 = 20;
    tMwindow.WindowX1 = 779;
    tMwindow.WindowY0 = 4 + 25;
    tMwindow.WindowY1 = 390 + 25;

    actual_menu_content = MENU_UNDEFINED;
}

void tM_refresh(char *text, uint8_t *textPointer, uint8_t line, const char content[6])
{
    for(uint8_t i=0; i<6; i++)
    {
        if(((line == 0) || (line == i)) && content[i])
        {
                text[(*textPointer)++] = content[i];
        }
        text[(*textPointer)++] = '\n';
        text[(*textPointer)++] = '\r';
        text[*textPointer] = 0;
    }
}


void tM_rebuild_pages(void)
{
    menu.pagesAvailable = 0;
//		menu.pageMemoryForNavigation = 0;
    for(int i=0;i<=MAXPAGES;i++)
    {
        menu.lineMemoryForNavigationForPage[i] = 0;
        menu.linesAvailableForPage[i] = 0;
        menu.StartAddressForPage[i] = 0; // only with GFX_forceReleaseFramesWithId(5); !!!!!
    }
    GFX_forceReleaseFramesWithId(5);
    tM_build_pages();
}


void tM_rebuild_menu_after_tComm(void)
{
    tM_rebuild_pages();
}

#
    void tM_check_content(void)
{
    uint8_t mode = 0;

    if(stateUsed->mode == MODE_DIVE)
    {
        if(stateUsed == stateRealGetPointer())
            mode = MENU_DIVE_REAL;
        else
            mode = MENU_DIVE_SIM;
    }
    else
        mode = MENU_SURFACE;

    if(actual_menu_content != mode)
    {
        actual_menu_content = mode;
        tM_rebuild_pages();
    }
}


void clean_line_actual_page(void)
{
    uint8_t line, page;

    page = menu.pageMemoryForNavigation;
    line =  menu.lineMemoryForNavigationForPage[page];

    tMscreen.FBStartAdress = menu.StartAddressForPage[page];
    GFX_clean_line(&tMwindow, line);
}


void update_content_actual_page(char *text, uint16_t tab, char *subtext)
{
    uint8_t page;

    page = menu.pageMemoryForNavigation;

    tMscreen.FBStartAdress = menu.StartAddressForPage[page];
    if(tab == 0)
        tMwindow.WindowTab = 400;
    else
        tMwindow.WindowTab = tab;

    tMenu_write(page, text, subtext);
}


void clean_line(uint8_t page, uint8_t line)
{
    tMscreen.FBStartAdress = menu.StartAddressForPage[page];
    GFX_clean_line(&tMwindow, line);
}

void update_content_with_new_frame(uint8_t page, char *text, uint16_t tab, char *subtext)
{
    char localtext[32];

    uint32_t rememberPage = menu.StartAddressForPage[page];
    menu.StartAddressForPage[page] = getFrame(5);
    tMscreen.FBStartAdress = menu.StartAddressForPage[page];

    if(tab == 0)
        tMwindow.WindowTab = 400;
    else
        tMwindow.WindowTab = tab;

    draw_tMheader(page);
    tMenu_write(page, text, subtext);

    localtext[0] = TXT_2BYTE;
    localtext[1] = TXT2BYTE_ButtonBack;
    localtext[2] = 0;
    write_content_simple(&tMscreen, 0, 800, 480-24, &FontT24,localtext,CLUT_ButtonSurfaceScreen);

    localtext[0] = '\001';
    localtext[1] = TXT_2BYTE;
    localtext[2] = TXT2BYTE_ButtonEnter;
    localtext[3] = 0;
    write_content_simple(&tMscreen, 0, 800, 480-24, &FontT24,localtext,CLUT_ButtonSurfaceScreen);

    localtext[0] = '\002';
    localtext[1] = TXT_2BYTE;
    localtext[2] = TXT2BYTE_ButtonNext;
    localtext[3] = 0;
    write_content_simple(&tMscreen, 0, 800, 480-24, &FontT24,localtext,CLUT_ButtonSurfaceScreen);

//	gfx_write_page_number(&tMscreen ,menu.pageCountNumber[page],menu.pageCountTotal,0);

    if(page == menu.pageMemoryForNavigation)
        GFX_SetFrameTop(tMscreen.FBStartAdress);
    releaseFrame(5,rememberPage);
}

void update_content(uint8_t page, char *text, uint16_t tab, char *subtext)
{
    tMscreen.FBStartAdress = menu.StartAddressForPage[page];
    if(tab == 0)
        tMwindow.WindowTab = 400;
    else
        tMwindow.WindowTab = tab;

    tMenu_write(page, text, subtext);
}


void tM_create_pagenumbering(void)
{
    menu.pageCountTotal = 0;

    for(int i=0;i<=MAXPAGES;i++)
    {
        if(menu.pageCountNumber[i])
        {
                menu.pageCountTotal++;
                menu.pageCountNumber[i] = menu.pageCountTotal;
        }
    }
}


void tM_build_page(uint32_t id, char *text, uint16_t tab, char *subtext)
{
    uint8_t linesFound;
    uint16_t i;
    SStateList idList;
    uint8_t page;

    char localtext[32];

    if(menu.pagesAvailable > MAXPAGES)
        return;

    get_idSpecificStateList(id, &idList);

    if(idList.base != BaseMenu)
        return;

    if(idList.page == 0)
        return;

    if(idList.page > MAXPAGES)
        return;

    page = idList.page;

    if(!menu.pageCountNumber[page])
        return;

    if(menu.pagesAvailable == 0)
        tM_create_pagenumbering();

    if(*text == 0)
        return;

    linesFound = 1;

    if(menu.StartAddressForPage[page])
        releaseFrame(5,menu.StartAddressForPage[page]);

    menu.StartAddressForPage[page] = getFrame(5);

    if(menu.StartAddressForPage[page] == 0)
        return;

    i = 0;
    while((i < MAX_PAGE_TEXTSIZE) && text[i])
    {
        if((text[i] == '\n') && ((i + 2) < MAX_PAGE_TEXTSIZE) && text[i+1] && text[i+2])
            linesFound += 1;
        i++;
    }

    menu.linesAvailableForPage[page] = linesFound;
    menu.pagesAvailable++; /* even if it was used before */

    tMscreen.FBStartAdress = menu.StartAddressForPage[page];
    if(tab == 0)
        tMwindow.WindowTab = 400;
    else
        tMwindow.WindowTab = tab;

    draw_tMheader(page);

    tMenu_write(page, text, subtext);

    localtext[0] = TXT_2BYTE;
    localtext[1] = TXT2BYTE_ButtonBack;
    localtext[2] = 0;
    write_content_simple(&tMscreen, 0, 800, 480-24, &FontT24,localtext,CLUT_ButtonSurfaceScreen);

    localtext[0] = '\001';
    localtext[1] = TXT_2BYTE;
    localtext[2] = TXT2BYTE_ButtonEnter;
    localtext[3] = 0;
    write_content_simple(&tMscreen, 0, 800, 480-24, &FontT24,localtext,CLUT_ButtonSurfaceScreen);

    localtext[0] = '\002';
    localtext[1] = TXT_2BYTE;
    localtext[2] = TXT2BYTE_ButtonNext;
    localtext[3] = 0;
    write_content_simple(&tMscreen, 0, 800, 480-24, &FontT24,localtext,CLUT_ButtonSurfaceScreen);
}

/*
_Bool skipCCRpage(uint8_t page)
{
    if(menu.ccrOnlyContentForPage[page])
    {
        if(actual_menu_content == MENU_SURFACE)
        {
                SSettings *data = settingsGetPointer();
                if((data->dive_mode != DIVEMODE_CCR) && data->hideCCRinOCmode)
                    return 1;
        }
    }
    return 0;
}
*/

void findValidPosition(uint8_t *pageOuput, uint8_t *lineOutput)
{
    uint8_t page, line, first;

    *pageOuput = 0;
    *lineOutput = 0;

    /* test */
    if(menu.pagesAvailable == 0)
        return;

    for(int i=1;i<=MAXPAGES;i++)
    {
        if((menu.pageCountNumber[i] != 0)
            && (menu.linesAvailableForPage[i] != 0)
            && (menu.StartAddressForPage[i] != 0))
        {
            first = i;
            break;
        }
    }

    /* select */
    if(menu.pageMemoryForNavigation > MAXPAGES)
        menu.pageMemoryForNavigation = first;

    page = menu.pageMemoryForNavigation;

    if(page == 0)
        page = first;

    while((page <= MAXPAGES) && ((menu.linesAvailableForPage[page] == 0) || (menu.StartAddressForPage[page] == 0) || (menu.pageCountNumber[page] == 0)))
        page += 1;

    if(page > MAXPAGES)
        page = first;

    line = menu.lineMemoryForNavigationForPage[page];

    if(line == 0)
        line = 1;

    if(line > menu.linesAvailableForPage[page])
        line = 1;

    *pageOuput = page;
    *lineOutput = line;
}

/*
void tM_insert_page_numbers(void)
{
    uint8_t page, line, pageMemoryBackup, pageFirst, total;
    GFX_DrawCfgScreen	tTscreen;

    tTscreen.FBStartAdress = 0;
    tTscreen.ImageHeight = 480;
    tTscreen.ImageWidth = 800;
    tTscreen.LayerIndex = 1;

    pageMemoryBackup = menu.pageMemoryForNavigation;

    menu.pageMemoryForNavigation = 1;
    findValidPosition(&page, &line);
    pageFirst = page;
    total = 0;

    do
    {
        total++;
        menu.pageMemoryForNavigation += 1;
        findValidPosition(&page, &line);
    }
    while((pageFirst != page) &&  (total < MAXPAGES));

    menu.pageCountTotal = total;
    menu.pageMemoryForNavigation = 0;
    for(int i = 1; i<= total; i++)
    {
        menu.pageMemoryForNavigation += 1;
        findValidPosition(&page, &line);
        tTscreen.FBStartAdress = menu.StartAddressForPage[page];
        menu.pageCountNumber[page] = i;
        gfx_write_page_number(&tTscreen ,i,total,0);
    }

    menu.pageMemoryForNavigation = pageMemoryBackup;
}
*/

void tM_add(uint32_t id)
{
    SStateList idList;
    uint8_t page;

    get_idSpecificStateList(id, &idList);

    page = idList.page;

    if(page > MAXPAGES)
        return;

    menu.pageCountNumber[page] = 1;
}


void tM_build_pages(void)
{
    char text[MAX_PAGE_TEXTSIZE];
    char subtext[MAX_PAGE_TEXTSIZE];
    uint32_t id;
    uint16_t tabPosition;
    SSettings *pSettings = settingsGetPointer();

    menu.pagesAvailable = 0;
    for(int i=0;i<=MAXPAGES;i++)
        menu.pageCountNumber[i] = 0;

    tabPosition = 400;
    *text = 0;
    *subtext = 0;

    /* 2015 Feb 02, hw
     * max 8 Menu Pages
     */


    tM_add(StMSYS); //now in both modes
    if(actual_menu_content == MENU_SURFACE)
    {
        tM_add(StMDECO);
        tM_add(StMHARD);
//		tM_add(StMSYS); now in both modes
    }
    else
    {
        tM_add(StMXTRA);
    }
    if(actual_menu_content == MENU_SURFACE)
    {
        tM_add(StMPLAN);
    }
//	if((pSettings->dive_mode != DIVEMODE_Gauge) && (pSettings->dive_mode != DIVEMODE_Apnea))
//	{
        tM_add(StMOG);
        tM_add(StMDECOP);
//	}
    if((pSettings->dive_mode == DIVEMODE_CCR) || (stateUsed->diveSettings.ccrOption == 1))
    {
        tM_add(StMCG);
        tM_add(StMSP);
    }

    id = tMOG_refresh(0, text, &tabPosition, subtext);
    tM_build_page(id, text, tabPosition, subtext);

    id = tMCG_refresh(0, text, &tabPosition, subtext);
    tM_build_page(id, text, tabPosition, subtext);

    id = tMSP_refresh(0, text, &tabPosition, subtext);
    tM_build_page(id, text, tabPosition, subtext);

    id = tMXtra_refresh(0, text, &tabPosition, subtext);
    tM_build_page(id, text, tabPosition, subtext);

    id = tMPlanner_refresh(0, text, &tabPosition, subtext);
    tM_build_page(id, text, tabPosition, subtext);

    id = tMDeco_refresh(0, text, &tabPosition, subtext);
    tM_build_page(id, text, tabPosition, subtext);

    id = tMDecoParameters_refresh(0, text, &tabPosition, subtext);
    tM_build_page(id, text, tabPosition, subtext);

    id = tMPlanner_refresh(0, text, &tabPosition, subtext);
    tM_build_page(id, text, tabPosition, subtext);

    id = tMHardware_refresh(0, text, &tabPosition, subtext);
    tM_build_page(id, text, tabPosition, subtext);

    id = tMSystem_refresh(0, text, &tabPosition, subtext);
    tM_build_page(id, text, tabPosition, subtext);
}


void tM_refresh_live_content(void)
{
    uint8_t page;
    char text[MAX_PAGE_TEXTSIZE];
    char subtext[MAX_PAGE_TEXTSIZE];
    uint16_t tabPosition;

//	if(menu.modeFlipPages == 0)
//		return;

    if((get_globalState() == StMSYS) && (actual_menu_content == MENU_SURFACE))
    {
        page = menu.pageMemoryForNavigation;
        tMSystem_refresh(0, text, &tabPosition, subtext);
        update_content_with_new_frame(page, text, tabPosition, subtext);
    }
    else
    if(get_globalState() == StMHARD)
    {
        page = menu.pageMemoryForNavigation;
        tMHardware_refresh(0, text, &tabPosition, subtext);
        update_content_with_new_frame(page, text, tabPosition, subtext);
    }

    tMscreen.FBStartAdress = menu.StartAddressForPage[page];
    tHome_show_lost_connection_count(&tMscreen);
    /*
    SStateList idList;
    if(actual_menu_content == MENU_SURFACE)
    {
        page = menu.pageMemoryForNavigation;
        get_idSpecificStateList(StMSYS, &idList);
        if(page == idList.page)
        {
            tMSystem_refresh(0, text, &tabPosition, subtext);
            update_content_with_new_frame(page, text, tabPosition, subtext);
        }
    }
    */
}


/* new frame only! */
void updateSpecificMenu(uint32_t id)
{
    uint8_t  page;
    SStateList idList;

    char text[MAX_PAGE_TEXTSIZE];
    char subtext[MAX_PAGE_TEXTSIZE];
    uint16_t tabPosition;

    *subtext = 0;
    *text = 0;
    tabPosition = 400;

    get_idSpecificStateList(id, &idList);
    page = idList.page;

    switch(id)
    {
    case StMOG:
        tMOG_refresh(0, text, &tabPosition, subtext);
        update_content_with_new_frame(page, text, tabPosition, subtext);
        break;
    case StMCG:
        tMCG_refresh(0, text, &tabPosition, subtext);
        update_content_with_new_frame(page, text, tabPosition, subtext);
        break;
    case StMSP:
        tMSP_refresh(0, text, &tabPosition, subtext);
        update_content_with_new_frame(page, text, tabPosition, subtext);
        break;
    default:
        break;
    }
}


void updateMenu(void)
{
    uint8_t page, line;

    char text[MAX_PAGE_TEXTSIZE];
    char subtext[MAX_PAGE_TEXTSIZE];
    uint16_t tabPosition;

    *subtext = 0;
    *text = 0;
    tabPosition = 400;

    page = menu.pageMemoryForNavigation;
    line = menu.lineMemoryForNavigationForPage[page];

    switch(get_globalState())
    {
    case StMOG:
        tMOG_refresh(0, text, &tabPosition, subtext);
        update_content_with_new_frame(page, text, tabPosition, subtext);
        break;
    case StMCG:
        tMCG_refresh(0, text, &tabPosition, subtext);
        update_content_with_new_frame(page, text, tabPosition, subtext);
        break;
    case StMSP:
        tMSP_refresh(0, text, &tabPosition, subtext);
        update_content_with_new_frame(page, text, tabPosition, subtext);
        break;
    case StMXTRA:
        tMXtra_refresh(0, text, &tabPosition, subtext);
        update_content_with_new_frame(page, text, tabPosition, subtext);
        break;
    case StMDECO:
        if((line == 1) || (line == 3)) // dive mode or ppO2 limits (the later for correct MOD in gaslists)
        {
            tM_rebuild_pages();
            menu.lineMemoryForNavigationForPage[page] = line; // fix 160623
            GFX_SetFrameTop(menu.StartAddressForPage[page]);
        }
        else
        {
            tMDeco_refresh(line, text, &tabPosition, subtext);
            clean_line_actual_page();
            update_content_actual_page(text, tabPosition, subtext);
        }
        break;
    case StMDECOP:
        tMDecoParameters_refresh(line, text, &tabPosition, subtext);
        clean_line_actual_page();
        update_content_actual_page(text, tabPosition, subtext);
        break;
    case StMPLAN:
        tMPlanner_refresh(line, text, &tabPosition, subtext);
        clean_line_actual_page();
        update_content_actual_page(text, tabPosition, subtext);
        break;
    case StMHARD:
        tMHardware_refresh(line, text, &tabPosition, subtext);
        clean_line_actual_page();
        update_content_actual_page(text, tabPosition, subtext);
        break;
    case StMSYS:
        if((line == 2) || (line == 3) || (line == 6))
        {
            tM_rebuild_pages();
            menu.lineMemoryForNavigationForPage[page] = line; // fix 160623
            GFX_SetFrameTop(menu.StartAddressForPage[page]);
            menu.lineMemoryForNavigationForPage[page] = line;
        }
        else
        {
            tMSystem_refresh(line, text, &tabPosition, subtext);
            clean_line_actual_page();
            update_content_actual_page(text, tabPosition, subtext);
        }
        break;
    default:
        break;
    }
}

void openMenu_first_page_with_OC_gas_update(void)
{
    menu.pageMemoryForNavigation = 1;
    for(int i=0;i<=MAXPAGES;i++)
        menu.lineMemoryForNavigationForPage[i] = 0;

    set_globalState(StMOG);
    updateMenu();
    openMenu(1);
}


void openMenu(uint8_t freshWithFlipPages)
{
    uint8_t page, line;

    callerID = get_globalState();

    findValidPosition(&page, &line);
    if((page == 0) || (line == 0))
        return;

    menu.pageMemoryForNavigation = page;
    /* new test for 3button design */
    if(freshWithFlipPages)
    {
        menu.lineMemoryForNavigationForPage[page] = 0;
        menu.modeFlipPages = 1;
    }
    else
    {
        menu.lineMemoryForNavigationForPage[page] = line;
        menu.modeFlipPages = 0;
    }

    set_globalState_Menu_Page(page);


    change_CLUT_entry(CLUT_MenuLineSelectedSides, 		(CLUT_MenuPageGasOC + page - 1));
    change_CLUT_entry(CLUT_MenuLineSelectedSeperator, (CLUT_MenuPageGasOC + page - 1));


    if(((page == 6) ||  (page == 8)) && (menu.pageCountNumber[page-1] == 0))
    {
        change_CLUT_entry(CLUT_MenuLineSelectedSides, 		(CLUT_MenuPageGasOC + page - 2));
        change_CLUT_entry(CLUT_MenuLineSelectedSeperator, (CLUT_MenuPageGasOC + page - 2));

    }

    GFX_SetFrameTop(menu.StartAddressForPage[page]);
    /* new test for 3button design */
    if(freshWithFlipPages)
        GFX_SetFrameBottom(tMdesignSolo.FBStartAdress, 0, 25, 800, 390);
    else
        GFX_SetFrameBottom((tMdesignCursor.FBStartAdress) + 65*2*(line - 1), 0, 25, 800, 390);
}

void block_diluent_handler(_Bool Unblock)
{
    SStateList list;
    static uint8_t	linesAvailableForPageDiluent = 0;
    get_idSpecificStateList(StMCG, &list);

    if(Unblock && linesAvailableForPageDiluent)
    {
        menu.linesAvailableForPage[list.page] = linesAvailableForPageDiluent;
    }
    else
    {
        linesAvailableForPageDiluent = menu.linesAvailableForPage[list.page];
        menu.linesAvailableForPage[list.page] = 0;
    }
}

void block_diluent_page(void)
{
    block_diluent_handler(0);
}


void unblock_diluent_page(void)
{
    block_diluent_handler(1);
}


void nextPage(void)
{
    uint8_t page, line;

    menu.pageMemoryForNavigation += 1;

    findValidPosition(&page, &line);
    menu.pageMemoryForNavigation = page;
    /* new test for 3button design */
    //menu.lineMemoryForNavigationForPage[page] = line;
    menu.lineMemoryForNavigationForPage[page] = 0;
    menu.modeFlipPages = 1;

    set_globalState_Menu_Page(page);

    change_CLUT_entry(CLUT_MenuLineSelectedSides, 		(CLUT_MenuPageGasOC + page - 1));
    change_CLUT_entry(CLUT_MenuLineSelectedSeperator, (CLUT_MenuPageGasOC + page - 1));

    GFX_SetFrameTop(menu.StartAddressForPage[page]);
    /* new test for 3button design */
    //GFX_SetFrameBottom((tMdesignCursor.FBStartAdress) + 65*2*(line - 1), 0, 25, 800, 390);
    GFX_SetFrameBottom(tMdesignSolo.FBStartAdress, 0, 25, 800, 390);
}


void nextLine(void)
{
    uint8_t page, line;

    page = menu.pageMemoryForNavigation;
    menu.lineMemoryForNavigationForPage[page] += 1;

    findValidPosition(&page, &line);
    menu.lineMemoryForNavigationForPage[page] = line;

    /* new test for 3button design */
    menu.modeFlipPages = 0;

    GFX_SetFrameBottom((tMdesignCursor.FBStartAdress) + 65*2*(line - 1), 0, 25, 800, 390);
}


void stepBackMenu(void)
{
    if(menu.modeFlipPages == 0)
    {
        menu.lineMemoryForNavigationForPage[menu.pageMemoryForNavigation] = 0;
        menu.modeFlipPages = 1;
        GFX_SetFrameBottom(tMdesignSolo.FBStartAdress, 0, 25, 800, 390);
    }
    else
        exitMenu();
}


void exitMenu(void)
{
    set_globalState_tHome();
}


void stepForwardMenu(void)
{
    if(menu.modeFlipPages == 1)
    {
        nextLine();
    }
    else
        gotoMenuEdit();
}

void gotoMenuEdit(void)
{
    uint8_t line;

    line = menu.lineMemoryForNavigationForPage[menu.pageMemoryForNavigation];

    switch(get_globalState())
    {
    case StMOG:
        openEdit_GasOC(line);
        break;
    case StMCG:
        openEdit_GasCC(line);
        break;
    case StMSP:
        openEdit_Setpoint(line);
        break;
    case StMXTRA:
        openEdit_Xtra(line);
        break;
    case StMDECO:
        openEdit_Deco(line);
        break;
    case StMDECOP:
        openEdit_DecoParameter(line);
        break;
    case StMPLAN:
        openEdit_Planner(line);
        break;
    case StMHARD:
        openEdit_Hardware(line);
        break;
    case StMSYS:
        openEdit_System(line);
        break;
    default:
        break;
    }
}


void sendActionToMenu(uint8_t sendAction)
{
    switch(sendAction)
    {
    case ACTION_BUTTON_ENTER:
        stepForwardMenu();
        break;
    case ACTION_BUTTON_NEXT:
        if(menu.modeFlipPages)
            nextPage();
        else
            nextLine();
        break;
    case ACTION_TIMEOUT:
    case ACTION_MODE_CHANGE:
    case ACTION_BUTTON_BACK:
        /* new test for 3button design */
        stepBackMenu();
        break;
    default:
        break;
    case ACTION_IDLE_TICK:
    case ACTION_IDLE_SECOND:
        break;
    }
/* 	tMC_OC_Gas(StMOG1, pSettings); */
}

void timeoutTestMenu(uint32_t seconds_since_last_button_press)
{
}

void tMenu_write(uint8_t page, char *text, char *subtext)
{
    if(page > MAXPAGES)
        return;
    if(menu.linesAvailableForPage[page] == 0)
        return;

    tMscreen.FBStartAdress = menu.StartAddressForPage[page];
    GFX_write_string(&FontT48, &tMwindow, text,1);
    if((*subtext) && (menu.linesAvailableForPage[page] < 6))
    {
        GFX_write_string(&FontT42, &tMwindow, subtext, (menu.linesAvailableForPage[page] + 1));
    }
}


/* Private functions ---------------------------------------------------------*/

void draw_tMdesignSubUnselected(uint32_t *ppDestination)
{
    union al88_u
    {
        uint8_t al8[2];
        uint16_t al88;
    };

    union al88_u color_seperator;
    union al88_u color_unselected;
    int i;

    color_seperator.al8[0] = CLUT_MenuLineUnselectedSeperator;
    color_unselected.al8[0] = CLUT_MenuLineUnselected;

    color_seperator.al8[1] = 0xFF;
    color_unselected.al8[1] = 0xFF;

    *(__IO uint16_t*)*ppDestination = color_seperator.al88;
        *ppDestination += 2;
    *(__IO uint16_t*)*ppDestination = color_seperator.al88;
        *ppDestination += 2;

    for(i = 61; i > 0; i--)
    {
        *(__IO uint16_t*)*ppDestination = color_unselected.al88;
        *ppDestination += 2;
    }

    *(__IO uint16_t*)*ppDestination = color_seperator.al88;
        *ppDestination += 2;
    *(__IO uint16_t*)*ppDestination = color_seperator.al88;
        *ppDestination += 2;
}


void draw_tMdesignSubSelected(uint32_t *ppDestination)
{
    union al88_u
    {
        uint8_t al8[2];
        uint16_t al88;
    };

    union al88_u color_selected;
    union al88_u color_seperator;
    int i;

    color_selected.al8[0] = CLUT_MenuLineSelected;
    color_selected.al8[1] = 0xFF;

    color_seperator.al8[0] = CLUT_MenuLineSelectedSeperator;
    color_seperator.al8[1] = 0xFF;

    *(__IO uint16_t*)*ppDestination = color_seperator.al88;
        *ppDestination += 2;
    *(__IO uint16_t*)*ppDestination = color_seperator.al88;
        *ppDestination += 2;

    for(i = 61; i > 0; i--)
    {
        *(__IO uint16_t*)*ppDestination = color_selected.al88;
        *ppDestination += 2;
    }

    *(__IO uint16_t*)*ppDestination = color_seperator.al88;
        *ppDestination += 2;
    *(__IO uint16_t*)*ppDestination = color_seperator.al88;
        *ppDestination += 2;
}


void draw_tMdesignSubSelectedBorder(uint32_t *ppDestination)
{
    union al88_u
    {
        uint8_t al8[2];
        uint16_t al88;
        };

    union al88_u color_selected_sides;

    int i;

    color_selected_sides.al8[0] = CLUT_MenuLineSelectedSides;
    color_selected_sides.al8[1] = 0xFF;

    for(i = 65; i > 0; i--)
    {
    *(__IO uint16_t*)*ppDestination = color_selected_sides.al88;
        *ppDestination += 2;
    }
}


void draw_tMcursorDesign(void)
{
    int i,j;
    uint32_t pDestination;

    pDestination = tMdesignCursor.FBStartAdress;

    for(j = 801; j > 0; j--)
    {
        for(i = 5; i > 0; i--)
        {
            draw_tMdesignSubUnselected(&pDestination);
        }
        if((j > 787) || (j < 17))
            draw_tMdesignSubSelectedBorder(&pDestination);
        else
            draw_tMdesignSubSelected(&pDestination);
    }

    pDestination = tMdesignSolo.FBStartAdress;

    for(j = 801; j > 0; j--)
    {
        for(i = 6; i > 0; i--)
        {
            draw_tMdesignSubUnselected(&pDestination);
        }
    }
}


void draw_tMheader(uint8_t page)
{
    union al88_u
    {
        uint8_t al8[2];
        uint16_t al88;
    };
    union al88_u color_top;
    int i,j, k, k4text;
    uint32_t pBackup;
    uint32_t pDestination;
    uint8_t colorText;
    uint16_t positionText;
    uint8_t pageText;

    const char text8max[MAXPAGES+1][8] =
    {   "",
        "OC",
        "CC",
        "SP",
        "DATA",
        "DECO",
        "",
        "SYS",
        "",
        "SIM",
        ""
    };

    const _Bool spacing[MAXPAGES+1] =
    {   0,
        0, // behind OC
        0, // behind CC
        1, // behind SP
        1, // behind DATA
        0, // behind DECO1
        1, // behind DECO2
        0, // behind SYS1
        1, // behind SYS2
        1, // behind SIM
        0
    };

    pBackup = tMscreen.FBStartAdress;
    tMscreen.FBStartAdress = menu.StartAddressForPage[page];
    pDestination = menu.StartAddressForPage[page];
    positionText = 10;
    pageText = page;

    gfx_write_page_number(&tMscreen ,menu.pageCountNumber[page],menu.pageCountTotal,0);

    while((text8max[pageText][0] == 0) && (pageText > 1))
    {
        pageText--;
    }

    for(k = 1; k <= MAXPAGES; k++)
    {
        if(menu.pageCountNumber[k] != 0)
        {
            k4text = k; // new hw 170522
            if((text8max[k][0] == 0) && (menu.pageCountNumber[k-1] == 0))
                k4text = k-1;

            color_top.al8[0] = CLUT_MenuPageGasOC + k - 1;
            if(k4text == page)
            {
                color_top.al8[1] = 0xFF;
            }
            else
            {
                color_top.al8[1] = 0x50;
            }

            if(k4text == pageText)
            {
                colorText = CLUT_Font020;
            }
            else
            {
                colorText = CLUT_Font021;
            }

            write_content_simple(&tMscreen,positionText,775,0,&FontT42,text8max[k4text],colorText);
            /*
            write_content_simple(&tMscreen,positionText,775,0,&FontT42,text8max[k],colorText);
            if((text8max[k][0] == 0) && (menu.pageCountNumber[k-1] == 0))
                write_content_simple(&tMscreen,positionText,775,0,&FontT42,text8max[k-1],colorText);
    */
            pDestination += 2 * 5 * 480;

            for(j = 60; j > 0; j--)
            {
                pDestination += (390 + 26)* 2;

//				for(i = 64; i > 0; i--)
                for(i = 16; i > 0; i--)
                {
                *(__IO uint16_t*)pDestination = color_top.al88;
                    pDestination += 2;
                }
                pDestination += 2 * 48;
            }

            pDestination += 2 * 5 * 480;
            positionText += 70;

            if((k == 4) || ((k == 6) && (menu.pageCountNumber[5] == 0)))
            {
                pDestination += 2 * 70 * 480;
                positionText += 70;
            }

            if(spacing[k])
            {
                pDestination += 35 * 2 * 480;
                positionText += 35;
            }
        }
    }
    tMscreen.FBStartAdress = pBackup;
}

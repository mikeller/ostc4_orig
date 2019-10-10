/*
 * motion.c
 *
 *  Created on: 20.05.2019
 *      Author: Thorsten Sonntag
 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "motion.h"
#include "data_central.h"
#include "t7.h"
#include "settings.h"

#define	STABLE_STATE_COUNT			2	/* number of count to declare a state as stable (at the moment based on 100ms) */
#define STABLE_STATE_TIMEOUT		5	/* Detection shall be aborted if a movement state is stable for more than 500ms */

#define SECTOR_WINDOW				80.0  	/* Pitch window which is used for custom view projection */
#define SECTOR_WINDOW_MAX			120.0  	/* Pitch window which will be greater than the divers field of view */
#define SECTOR_HYSTERY				3		/* Additional offset to avoid fast changing displays */
#define SECTOR_BORDER				400.0	/* Define a value which is out of limit to avoid not wanted key events */
#define SECTOR_FILTER				10		/* Define speed for calculated angle to follow real value */

#define SECTOR_MAX					30		/* maximum number of sectors */
#define SECTOR_SCROLL				7		/* number of sectors used for scroll detection */

static detectionState_t detectionState = DETECT_NOTHING;

uint8_t curSector;
static uint8_t targetSector;
static float sectorSize;
static float sectorwindow;
static uint8_t sectorCount;

static float sector_upperborder;
static float sector_lowerborder;


uint8_t GetSectorForPitch(float pitch)
{
	static uint8_t lastsector = 0xFF;
	float newPitch;
	uint8_t sector = 0;


	newPitch = pitch + (sectorwindow / 2.0);		/* do not use negativ values */
	if (newPitch < 0.0)							/* clip value */
	{
		newPitch = 0.0;
	}
	if (newPitch > sectorwindow)							/* clip value */
	{
		newPitch = sectorwindow;
	}

	if(lastsector == 0xFF)						/* First call of function => make sure a new sector is set */
	{
		sector_lowerborder = SECTOR_BORDER;
		sector_upperborder = SECTOR_BORDER * -1.0;

	}

	/* switch to other sector? */
	if((newPitch > sector_upperborder) || (newPitch <= sector_lowerborder))
	{
		sector = (uint16_t) newPitch / sectorSize;
		sector_lowerborder = sector * sectorSize - SECTOR_HYSTERY;
		sector_upperborder = (sector + 1) * sectorSize + SECTOR_HYSTERY;
		lastsector = sector;
	}

	return lastsector;
}

void DefinePitchSectors(float centerPitch,uint8_t numOfSectors)
{
	if(numOfSectors == CUSTOMER_DEFINED_VIEWS)
	{
		sectorCount =  t7_GetEnabled_customviews();
		if(sectorCount > 7)
		{
			sectorCount = 7;	/* more views are hard to manually control */
		}
	}
	else
	if(numOfSectors != CUSTOMER_KEEP_LAST_SECTORS)
	{
		sectorCount = numOfSectors;
	}

	if(sectorCount == SECTOR_MAX)
	{
		sectorwindow = 	SECTOR_WINDOW_MAX;
	}
	else
	{
		sectorwindow = 	SECTOR_WINDOW;
	}

	sectorSize = sectorwindow / sectorCount;

/* get the current sector */
	curSector = GetSectorForPitch(stateRealGetPointer()->lifeData.compass_pitch);
	targetSector = curSector;
}

void InitMotionDetection(void)
{
	targetSector = 0;
	curSector = 0;
	sectorSize = 0;
	sectorCount = 0;

	switch(settingsGetPointer()->MotionDetection)
	{
		case MOTION_DETECT_SECTOR: DefinePitchSectors(0,CUSTOMER_DEFINED_VIEWS);
			break;
		case MOTION_DETECT_MOVE: DefinePitchSectors(0,SECTOR_MAX);
			break;
		case MOTION_DETECT_SCROLL: DefinePitchSectors(0,SECTOR_SCROLL);
			break;
		default:
			break;
	}

}

/* Map the current pitch value to a sector and create button event in case the sector is left */
detectionState_t detectSectorButtonEvent(float curPitch)
{
	static uint8_t lastTargetSector = 0;
	uint8_t newTargetSector;
	uint8_t PitchEvent = DETECT_NOTHING;

/* only change sector if reading is stable */
	newTargetSector = GetSectorForPitch(stateRealGetPointer()->lifeData.compass_pitch);
	if(lastTargetSector == newTargetSector)
	{
		targetSector = newTargetSector;
	}
	lastTargetSector = newTargetSector;
	if(targetSector != curSector)
	{
		 if(targetSector > curSector)
		 {
			 curSector++;
			PitchEvent = DETECT_POS_PITCH;
		 }
		 else
		 {
			 curSector--;
			 PitchEvent = DETECT_NEG_PITCH;
		 }
	}
	return PitchEvent;
}

/* Check if pitch is not in center position and trigger a button action if needed */
detectionState_t detectScrollButtonEvent(float curPitch)
{
	static uint8_t	delayscroll = 0;		/* slow down the number of scroll events */

	uint8_t PitchEvent = DETECT_NOTHING;
	uint8_t newSector;

	if(delayscroll == 0)
	{
		newSector = GetSectorForPitch(stateRealGetPointer()->lifeData.compass_pitch);
		/* for scroll detection the motion windoe is split into 6 sectors => set event accoring to the sector number*/
		switch(newSector)
		{
			case 0:
			case 1:	PitchEvent = DETECT_POS_PITCH;
				break;
			case 5:
			case 6:	PitchEvent = DETECT_NEG_PITCH;
				break;
			default:
				break;
		}
		if(PitchEvent != DETECT_NOTHING)
		{
			delayscroll = 5;
		}
	}
	else
	{
		delayscroll--;
	}
	return PitchEvent;
}


uint8_t sectorhist[20];
uint8_t sectorindex = 0;
/* Detect if user is generating an pitch including return to starting position */
/* This is done by feeding the past movements value per value into a state machine */
detectionState_t detectPitch(float currentPitch)
{
	static uint8_t lastSector = 0;
	static uint8_t startSector = 0;
	static uint8_t stableCnt = 0;

	if((detectionState == DETECT_NEG_PITCH) || (detectionState == DETECT_POS_PITCH))	/* discard last detection */
	{
		detectionState = DETECT_NOTHING;
	}

	curSector = GetSectorForPitch(stateRealGetPointer()->lifeData.compass_pitch);

	/* feed value into state machine */
	switch (detectionState)
	{
			case DETECT_NOTHING: 	if(curSector != lastSector)	/* detect a stable condition before evaluating for the next move */
									{
										stableCnt=0;
									}

									if(stableCnt > STABLE_STATE_COUNT)
									{
										detectionState = DETECT_START;
										stableCnt = 0;
										startSector = lastSector;
									}
				break;
			case DETECT_START:		if(curSector != lastSector)
									{
										if(abs(curSector - startSector) > 1)
										{
											if(curSector > lastSector)
											{
												detectionState = DETECT_POS_MOVE;
											}
											else
											{
												detectionState = DETECT_NEG_MOVE;
											}
											stableCnt = 0;
											startSector = lastSector;
										}
									}
				break;
			case DETECT_NEG_MOVE:
			case DETECT_POS_MOVE:	if(curSector == lastSector)		/* Moved to a max? */
									{
										if(abs(startSector - curSector) > 2)
										{
											detectionState++;
											stableCnt = 0;
										}
										if(stableCnt > 2)
										{
											detectionState = DETECT_NOTHING;
											stableCnt = 0;
										}
									}
				break;
			case DETECT_MAXIMA:
			case DETECT_MINIMA:		if(curSector != lastSector)		/* reset timeout detection */
									{
										detectionState++;
										stableCnt = 0;
									}
				break;
			case DETECT_RISEBACK:
			case DETECT_FALLBACK:
									if(curSector == lastSector)		/* check if we are back at start position at end of movement */
									{
										if(abs(startSector - curSector) <= 1)
										{
											if(stableCnt > 2)
											{
												detectionState++;
												stableCnt = 0;
											}
										}
									}
								break;
			default:
				detectionState = DETECT_NOTHING;
				break;
	}
	if(detectionState != DETECT_START)
	{
		stableCnt++;
	}
	lastSector = curSector;
	if(stableCnt > STABLE_STATE_TIMEOUT)
	{
		detectionState = DETECT_NOTHING;
		stableCnt = 0;
	}

	return detectionState;
}

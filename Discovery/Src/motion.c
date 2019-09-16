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
#define MOVE_DELTA_SPEED			4	/* Delta speed needed to identify a valid movement */
#define PITCH_DELTA_COUNT			10	/* Delta count needed to identify a valid minima / maxima */
#define PITCH_DELTA_END				10	/* Delta allowed between start and end position */


#define SECTOR_WINDOW				80.0  	/* Pitch window which is used for custom view projection */
#define SECTOR_HYSTERY				3		/* Additional offset to avoid fast changing displays */
#define SECTOR_BORDER				400.0	/* Define a value which is out of limit to avoid not wanted key events */
#define SECTOR_FILTER				10		/* Define speed for calculated angle to follow real value */

#define SECTOR_MAX					19		/* maximum number of sectors */
#define SECTOR_SCROLL				7		/* number of sectors used for scroll detection */

detectionState_t detectionState = DETECT_NOTHING;

uint8_t curSector;
static uint8_t targetSector;
static uint8_t sectorSize;
static uint8_t sectorCount;

SSector PitchSector[SECTOR_MAX];			/* max number of enabled custom views */


uint8_t GetSectorForPitch(float pitch)
{
	static float lastPitch = 1000;
	float newPitch;
	uint8_t index;
	uint8_t sector = 0;

	if(lastPitch == 1000)						/* init at first call */
	{
		lastPitch = pitch;
	}

	newPitch = lastPitch + (pitch / SECTOR_FILTER);

	for(index = 1; index < sectorCount; index++)
	{
		if((pitch < PitchSector[index].upperlimit) && (pitch > PitchSector[index].lowerlimit ))
		{
			sector = index;
			break;
		}
	}
	lastPitch = newPitch;
	return sector;
}

void DefinePitchSectors(float centerPitch,uint8_t numOfSectors)
{
	uint8_t index;

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
	sectorSize = SECTOR_WINDOW / sectorCount;

	PitchSector[0].upperlimit = centerPitch + (SECTOR_WINDOW / 2);
	PitchSector[0].lowerlimit = PitchSector[0].upperlimit - sectorSize - SECTOR_HYSTERY;

	for(index = 1; index < sectorCount; index++)
	{
		PitchSector[index].upperlimit = PitchSector[0].upperlimit - index * sectorSize + SECTOR_HYSTERY;
		PitchSector[index].lowerlimit = PitchSector[0].upperlimit - (index + 1) * sectorSize - SECTOR_HYSTERY;
	}

	PitchSector[0].upperlimit = SECTOR_BORDER;
	PitchSector[index - 1].lowerlimit = SECTOR_BORDER * -1.0;

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
	lastTargetSector = 	newTargetSector;
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

	sectorhist[sectorindex++] = curSector;
	if(sectorindex == 20) sectorindex=0;

	/* feed value into state machine */
	switch (detectionState)
	{
			case DETECT_NOTHING: 	if(curSector == lastSector)	/* detect a stable condition before evaluating for the next move */
									{
										stableCnt++;
									}
									else
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
										else
										{
											stableCnt++;		/* reset start sector in case of slow movement */
										}
									}
				break;
			case DETECT_NEG_MOVE:
			case DETECT_POS_MOVE:	if(curSector != lastSector) /* still moving?  */
									{
										stableCnt++;
									}
									else
									{
										if(abs(startSector - curSector) > 2)
										{
											detectionState++;
											stableCnt = 0;
										}
										else
										{
												stableCnt++;	/* maybe on the boundary of a sector => handle as stable */
										}								
									}
				break;
			case DETECT_MINIMA:
			case DETECT_MAXIMA:		/* stay at maximum for short time to add a pattern for user interaction */
									if(curSector == lastSector)
									{
										stableCnt++;
									}
									else
									{
										if(stableCnt > 0)	/* restart movement after a short break? */
										{
											detectionState++;
											stableCnt = 0;
										}
										else
										{
											stableCnt++;	/* maybe on the boundary of a sector => handle as stable */
										}
									}
				break;
			case DETECT_RISEBACK:
			case DETECT_FALLBACK:	if(curSector == lastSector)		/* check if we are back at start position at end of movement */
									{
										if(abs(startSector - curSector) <= 1) //(curSector == startSector)
										{
											detectionState++;
											stableCnt = 0;
										}
										else
										{
											stableCnt++;	/* maybe on the boundary of a sector => handle as stable */
										}
									}
									else
									{
										stableCnt++;
									}
				break;
			default:
				detectionState = DETECT_NOTHING;
				break;

	}
	lastSector = curSector;
	if(stableCnt > STABLE_STATE_TIMEOUT)
	{
		detectionState = DETECT_NOTHING;
		stableCnt = 0;
	}

	return detectionState;
}

/*
 * motion.c
 *
 *  Created on: 20.05.2019
 *      Author: Thorsten Sonntag
 */

#include <stdint.h>
#include <string.h>
#include <math.h>
#include "motion.h"
#include "data_central.h"
#include "t7.h"

#define	STABLE_STATE_COUNT			2	/* number of count to declare a state as stable (at the moment based on 100ms) */
#define STABLE_STATE_TIMEOUT		5	/* Detection shall be aborted if a movement state is stable for more than 500ms */
#define MOVE_DELTA_SPEED			4	/* Delta speed needed to identify a valid movement */
#define PITCH_DELTA_COUNT			10	/* Delta count needed to identify a valid minima / maxima */
#define PITCH_DELTA_END				10	/* Delta allowed between start and end position */


#define SECTOR_WINDOW				40.0  	/* Pitch window which is used for custom view projection */
#define SECTOR_HYSTERY				3		/* Additional offset to avoid fast changing displays */
#define SECTOR_BORDER				400.0	/* Define a value which is out of limit to avoid not wanted key events */
#define SECTOR_FILTER				10		/* Define speed for calculated angle to follow real value */


static detectionState_t detectionState = DETECT_NOTHING;

static uint8_t curSector;
static uint8_t targetSector;
static uint8_t sectorSize;
static uint8_t sectorCount;

SSector PitchSector[10];			/* max number of enabled custom views */


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

void DefinePitchSectors(float centerPitch)
{
	uint8_t index;

	sectorCount =  t7_GetEnabled_customviews();
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
	DefinePitchSectors(0);
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


/* Detect if user is generating an pitch including return to starting position */
/* This is done by feeding the past movements value per value into a state machine */
detectionState_t detectPitch(float currentPitch)
{
	static uint8_t stableCnt = 0;
	static float lastPitch = 0.0;
	static float startPitch = 0.0;
	float curSpeed;


	if((detectionState == DETECT_NEG_PITCH) || (detectionState == DETECT_POS_PITCH))	/* discard last detection */
	{
		detectionState = DETECT_NOTHING;
	}

	curSpeed = currentPitch - lastPitch;

	/* feed value into state machine */
	switch (detectionState)
	{
			case DETECT_NOTHING: 	if(fabsf(curSpeed) < MOVE_DELTA_SPEED)	/* detect a stable condition before evaluating for the next move */
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
									}
				break;
			case DETECT_START:		if(fabsf(curSpeed) > MOVE_DELTA_SPEED)
									{
										if(curSpeed > 0)
										{
											detectionState = DETECT_POS_MOVE;
										}
										else
										{
											detectionState = DETECT_NEG_MOVE;
										}
										stableCnt = 0;
										startPitch = lastPitch;
									}
				break;
			case DETECT_NEG_MOVE:
			case DETECT_POS_MOVE:	if(fabsf(curSpeed) > MOVE_DELTA_SPEED )
									{
										stableCnt++;
									}
									else
									{
										if(stableCnt >= STABLE_STATE_COUNT)	/* debounce movement */
										{
											if(fabsf(startPitch - currentPitch) > PITCH_DELTA_COUNT)
											{
												detectionState++;
											}
											else
											{
												detectionState = DETECT_NOTHING;
											}
										}
										else
										{
											detectionState = DETECT_NOTHING;
										}
										stableCnt = 0;
									}
				break;
			case DETECT_MINIMA:
			case DETECT_MAXIMA:		/* stay at maximum for short time to add a pattern for user interaction */
									if(fabsf(curSpeed) < MOVE_DELTA_SPEED )
									{
										stableCnt++;
									}
									else
									{
										if(stableCnt > 0)
										{
											detectionState++;
										}
										else
										{
											detectionState = DETECT_NOTHING;
										}
										stableCnt = 0;
									}
				break;
			case DETECT_RISEBACK:
			case DETECT_FALLBACK:	if(fabsf(curSpeed) < MOVE_DELTA_SPEED)
									{
										if(fabsf(startPitch - currentPitch) < PITCH_DELTA_END)
										{
											detectionState++;
										}
									}
									stableCnt++;
				break;
			default:
				detectionState = DETECT_NOTHING;
				break;

	}
	if(stableCnt > STABLE_STATE_TIMEOUT)
	{
		detectionState = DETECT_NOTHING;
		stableCnt = 0;
	}

	lastPitch = currentPitch;
	return detectionState;
}

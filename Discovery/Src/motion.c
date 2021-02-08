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
#include "t3.h"
#include "settings.h"
#include "base.h"

#define	STABLE_STATE_COUNT			2	/* number of count to declare a state as stable (at the moment based on 100ms) */
#define STABLE_STATE_TIMEOUT		5	/* Detection shall be aborted if a movement state is stable for more than 500ms */

#define SECTOR_MAX					24		/* maximum number of sectors */
#define SECTOR_SCROLL				7		/* number of sectors used for scroll detection */
#define SECTOR_MAX_CNT				5		/* max number of views used for sector control */

#define MOTION_DELTA_STABLE			0
#define MOTION_DELTA_JITTER			1
#define MOTION_DELTA_RAISE			2
#define MOTION_DELTA_FALL			3

#define MOTION_DELTA_JITTER_LEVEL	2.0		/* lower values are considered as stable */
#define MOTION_DELTA_RAISE_LEVEL	4.0		/* Movement causing a significant change detected */
#define MOTION_DELTA_FALL_LEVEL		-4.0	/* Movement causing a significant change detected */

#define MOTION_DELTA_HISTORY_SIZE	20		/* Number of history data sets */

detectionState_t detectionState = DETECT_NOTHING;
SSector sectorDetection;

static uint8_t motionDeltaHistory[3][MOTION_DELTA_HISTORY_SIZE];			/* Change history of roll, pitch and yaw */
static uint8_t motionDeltaHistoryIdx;										/* Current index of history data */

static uint8_t focusCnt = 0;
static uint8_t inFocus = 0;
static uint8_t sectorMap[SECTOR_MAX_CNT];

static uint8_t suspendMotionDetectionSec = 0;

void resetMotionDeltaHistory()
{
	motionDeltaHistoryIdx = 0;
	memset(motionDeltaHistory, 0, sizeof(motionDeltaHistory));
}

void evaluateMotionDelta(float roll, float pitch, float yaw)
{
	static float lastValue[3] = {0.0,0.0,0.0};
	uint8_t nextIndex = motionDeltaHistoryIdx + 1;
	uint8_t axis;
	float curValue;

	if(nextIndex == MOTION_DELTA_HISTORY_SIZE)
	{
		nextIndex = 0;
	}
	for(axis=0; axis < 3; axis++)
	{
		switch(axis)
		{
			case MOTION_HISTORY_ROLL:	curValue = roll;
				break;
			case MOTION_HISTORY_PITCH:	curValue = pitch;
				break;
			default:
			case MOTION_HISTORY_YAW:	if((yaw < 90) && (lastValue[MOTION_HISTORY_YAW] > 270.0))		/* transition 360 => 0 */
										{
											lastValue[MOTION_HISTORY_YAW] -= 360;
										}
										else if((yaw > 270) && (lastValue[MOTION_HISTORY_YAW] < 90.0))	/* transition 0 => 360 */
										{
											lastValue[MOTION_HISTORY_YAW] += 360;
										}
										curValue = yaw;
				break;
		}
		if(curValue - lastValue[axis] > MOTION_DELTA_RAISE_LEVEL)
		{
			motionDeltaHistory[axis][nextIndex] = MOTION_DELTA_RAISE;
		}
		if(fabsf(curValue - lastValue[axis]) < MOTION_DELTA_RAISE_LEVEL)
		{
			motionDeltaHistory[axis][nextIndex] = MOTION_DELTA_JITTER;
		}
		if(fabsf(curValue - lastValue[axis]) < MOTION_DELTA_JITTER_LEVEL)
		{
			motionDeltaHistory[axis][nextIndex] = MOTION_DELTA_STABLE;
		}
		if(curValue - lastValue[axis] < MOTION_DELTA_FALL_LEVEL)
		{
			motionDeltaHistory[axis][nextIndex] = MOTION_DELTA_FALL;
		}
		lastValue[axis] = curValue;
	}
	motionDeltaHistoryIdx = nextIndex;
}

SDeltaHistory GetDeltaHistory(uint8_t stepback)
{
	uint8_t loop;
	uint8_t index = motionDeltaHistoryIdx;

	SDeltaHistory result = {0,0,0};

	stepback++;						/* motionDeltaHistoryIdx is pointing to future entry => step back one to get the latest */
	loop = stepback;
	if(stepback < MOTION_DELTA_HISTORY_SIZE)
	{
		while(loop != 0)			/* find requested entry */
		{
			loop--;
			index--;
			if(index == 0)
			{
				index = MOTION_DELTA_HISTORY_SIZE - 1;
			}
		}
		result.roll = motionDeltaHistory[MOTION_HISTORY_ROLL][index];
		result.pitch = motionDeltaHistory[MOTION_HISTORY_PITCH][index];
		result.yaw = motionDeltaHistory[MOTION_HISTORY_YAW][index];
	}
	return result;
}

uint8_t GetSectorForFocus(float focusOffset)
{
	uint8_t sector = 0;
	float compare = 0.1;

	while(compare <= 0.5)
	{
		if(focusOffset > compare)
		{
			sector++;
		}
		else
		{
			break;
		}
		compare += 0.1;
	}
	if(sector > sectorDetection.count)
	{
		sector = sectorDetection.count;
	}
	return sector;
}

void DefineSectorCount(uint8_t numOfSectors)
{
	if(numOfSectors == CUSTOMER_DEFINED_VIEWS)
	{
		if(settingsGetPointer()->design == 3)		/* Big font view ? */
		{
			sectorDetection.count =  t3_GetEnabled_customviews();
		}
		else
		{
			sectorDetection.count =  t7_GetEnabled_customviews();
		}
		if(sectorDetection.count > SECTOR_MAX_CNT)
		{
			sectorDetection.count = SECTOR_MAX_CNT;	/* more views are hard to manually control */
		}
	}
	else
	if(numOfSectors != CUSTOMER_KEEP_LAST_SECTORS)
	{
		sectorDetection.count = numOfSectors;
	}
}


uint8_t GetCVForSector(uint8_t selSector)
{
	if(selSector < sectorDetection.count)
	{
		return sectorMap[selSector];
	}
	else
	{
		return 0;
	}
}

void MapCVToSector()
{
	uint8_t ViewIndex = 0;

	memset(sectorMap, 0, sizeof(sectorMap));

	if(settingsGetPointer()->design == 3)		/* Big font view ? */
	{
		t3_set_customview_to_primary();
		sectorMap[ViewIndex] = t3_change_customview(ACTION_END);
	}
	else
	{
		t7_set_customview_to_primary();
		sectorMap[ViewIndex] = t7_change_customview(ACTION_END);

	}

	ViewIndex++;
	while(ViewIndex < sectorDetection.count)
	{
		if(settingsGetPointer()->design == 3)		/* Big font view ? */
		{
			sectorMap[ViewIndex] = t3_change_customview(ACTION_BUTTON_ENTER);
		}
		else
		{
			sectorMap[ViewIndex] = t7_change_customview(ACTION_BUTTON_ENTER);
		}
		ViewIndex++;
	}

}

void InitMotionDetection(void)
{
	sectorDetection.target = 0;
	sectorDetection.current = 0;
	sectorDetection.size = 0;
	sectorDetection.count = 0;

	switch(settingsGetPointer()->MotionDetection)
	{
		case MOTION_DETECT_SECTOR: DefineSectorCount(CUSTOMER_DEFINED_VIEWS);
									MapCVToSector();
			break;
		case MOTION_DETECT_MOVE: DefineSectorCount(SECTOR_MAX);
			break;
		case MOTION_DETECT_SCROLL: DefineSectorCount(SECTOR_SCROLL);
			break;
		default:
			break;
	}

	resetMotionDeltaHistory();
}

/* Map the current pitch value to a sector and create button event in case the sector is left */
detectionState_t detectSectorButtonEvent(float focusOffset)
{
	uint8_t newTargetSector;

	newTargetSector = GetSectorForFocus(focusOffset);

	if(settingsGetPointer()->design == 3)		/* Big font view ? */
	{
		t3_select_customview(GetCVForSector(newTargetSector));
	}
	else
	{
		t7_select_customview(GetCVForSector(newTargetSector));
	}

	return DETECT_NOTHING;
}

/* Check if pitch is not in center position and trigger a button action if needed */
detectionState_t detectScrollButtonEvent(float focusOffset)
{
	static uint8_t	delayscroll = 0;		/* slow down the number of scroll events */

	uint8_t PitchEvent = DETECT_NOTHING;

	if(delayscroll == 0)
	{
		if(focusOffset > 0.3)
		{
			PitchEvent = DETECT_POS_PITCH;
			delayscroll = 7;
		}
	}
	else
	{
		delayscroll--;
	}
	return PitchEvent;
}


/* Detect if user is generating an pitch including return to starting position */
/* This is done by feeding the past movements value per value into a state machine */
detectionState_t detectPitch(float currentPitch)
{
	static int8_t lastStart = 0;
	uint8_t exit = 0;
	int8_t step = 0;
	uint8_t duration = 0;
	SDeltaHistory test;

	if(lastStart < 0)
	{
		detectionState = DETECT_NOTHING;
		lastStart = 0;
	}
	else
	{
		detectionState = DETECT_START;
	}
	step = lastStart;
	do
	{
		test = GetDeltaHistory(step);
		duration++;
		switch (detectionState)
		{
				case DETECT_NOTHING: 	if(test.pitch > MOTION_DELTA_STABLE)
										{
											exit = 1;
											lastStart = -2;
										}
										else
										{
											detectionState = DETECT_START;
											lastStart = -1;
										}
					break;
				case DETECT_START:		if(test.pitch == MOTION_DELTA_RAISE)
										{
											detectionState = DETECT_POS_MOVE;
											lastStart = step;
										}
										else
										if(test.pitch == MOTION_DELTA_FALL)
										{
											detectionState = DETECT_NEG_MOVE;
											lastStart = step;
										}
										else
										{
											lastStart = -1;
										}
										duration = 0;
					break;
				case DETECT_NEG_MOVE:	if((test.pitch <= MOTION_DELTA_JITTER) || (test.pitch == MOTION_DELTA_RAISE))
										{
											detectionState++;
										}
					break;
				case DETECT_POS_MOVE:	if((test.pitch <= MOTION_DELTA_JITTER) || (test.pitch == MOTION_DELTA_FALL))
										{
											detectionState++;
										}
					break;
				case DETECT_MAXIMA:		if(test.pitch == MOTION_DELTA_FALL)
										{
											detectionState = DETECT_FALLBACK;
										}
					break;
				case DETECT_MINIMA:		if(test.pitch == MOTION_DELTA_RAISE)
										{
											detectionState = DETECT_RISEBACK;
										}
					break;
				case DETECT_RISEBACK:
				case DETECT_FALLBACK:	if(test.pitch == MOTION_DELTA_STABLE)
										{
											if(duration > 4)	/* avoid detection triggered by short moves */
											{
												detectionState++;
											}
											exit = 1;
											lastStart = -2;
										}
									break;
				default:
					detectionState = DETECT_NOTHING;
					exit = 1;
				break;
		}
		step--;
	} while((step >= 0) && (!exit));

	if((lastStart < MOTION_DELTA_HISTORY_SIZE))
	{
		lastStart++;	/* prepare value for next iteration (history index will be increased) */
	}
	else
	{
		lastStart = -1;
	}
	if((detectionState != DETECT_POS_PITCH) && (detectionState != DETECT_NEG_PITCH))	/* nothing found */
	{
		detectionState = DETECT_NOTHING;
	}
	else																				/* dont detect the same event twice */
	{
		resetMotionDeltaHistory();
	}
	return detectionState;
}

void anglesToCoord(float roll, float pitch, float yaw, SCoord *pCoord)
{
	pCoord->x = ((cosf(yaw) * cosf(pitch)) * pCoord->x + (cosf(yaw)*sinf(pitch)*sinf(roll) - (sinf(yaw)* cosf(roll))) * pCoord->y + (cosf(yaw)*sinf(pitch)*cosf(roll) + sinf(yaw)*sinf(roll)) * pCoord->z);
	pCoord->y = ((sinf(yaw) * cosf(pitch)) * pCoord->x + (sinf(yaw)*sinf(pitch)*sinf(roll) + cosf(yaw) * cosf(roll)) * pCoord->y + ( sinf(yaw) * sinf(pitch) * cosf(roll) - cosf(yaw) * sinf(roll))* pCoord->z);
	pCoord->z = ((-1*sinf(pitch)) * pCoord->x + (cosf(pitch) *sinf(roll)) * pCoord->y + (cosf(pitch) * cosf(roll))* pCoord->z);
}

SCoord CoordAdd(SCoord cA, SCoord cB)
{
	SCoord result;

	result.x = cA.x + cB.x;
	result.y = cA.y + cB.y;
	result.z = cA.z + cB.z;
	return result;
}

SCoord CoordSub(SCoord cA, SCoord cB)
{
	SCoord result;

	result.x = cA.x - cB.x;
	result.y = cA.y - cB.y;
	result.z = cA.z - cB.z;
	return result;
}

SCoord CoordCross(SCoord cA, SCoord cB)
{
	SCoord result;

	result.x = (cA.y * cB.z) - (cA.z * cB.y);
	result.y = (cA.z * cB.x) - (cA.x * cB.z);
	result.z = (cA.x * cB.y) - (cA.y * cB.x);

	return result;

}

SCoord CoordMulF(SCoord op, float factor)
{
	SCoord result;
	result.x = (op.x * factor);
	result.y = (op.y * factor);
	result.z = (op.z * factor);

	return result;
}

SCoord CoordDivF(SCoord op, float factor)
{
	SCoord result;
	result.x = (op.x / factor);
	result.y = (op.y / factor);
	result.z = (op.z / factor);

	return result;
}

float CoordDot(SCoord cA, SCoord cB)
{
	float result;

	result = cA.x * cB.x + cA.y * cB.y + cB.z*cA.z;
	return result;
}

void calibrateViewport(float roll, float pitch, float yaw)
{
    SSettings* pSettings = settingsGetPointer();

    pSettings->viewPitch = pitch + 180;
	pSettings->viewRoll = roll+ 180;
	pSettings->viewYaw = yaw;
}


float checkViewport(float roll, float pitch, float yaw, uint8_t enableAxis)
{
	uint8_t retval = 0;
	float angleYaw;
	float anglePitch;
	float angleRoll;
	float distance = 0;
	float _a, _b;
	SCoord u,v,n;
	float r;

	SCoord refVec;
	SCoord axis_1;
	SCoord axis_2;
	SCoord curVec;
	SCoord resultVec;

	SSettings* pSettings = settingsGetPointer();

	roll += 180;
	pitch += 180;

	/* calculate base vector taking calibration delta into account yaw (heading) */
	float compYaw;

	if(enableAxis & MOTION_ENABLE_YAW)
	{
		compYaw = 360.0 - yaw; 				/* turn to 0° */
		compYaw +=  pSettings->viewYaw; 	/* consider calib yaw value */
		compYaw += yaw;

		if (compYaw < 0.0)
		{
			compYaw = 360.0 + compYaw;
		}

		if (compYaw > 360.0)
		{
			compYaw = compYaw - 360.0;
		}
		if (compYaw > 360.0)
		{
			compYaw = compYaw - 360.0;
		}
		angleYaw = pSettings->viewYaw * M_PI / 180.0;
	}
	else
	{
		compYaw = 0.0;
		angleYaw = 0.0;
	}

	if(enableAxis & MOTION_ENABLE_PITCH)
	{
		anglePitch = pSettings->viewPitch * M_PI / 180.0;
	}
	else
	{
		anglePitch = 0;
	}
	if(enableAxis & MOTION_ENABLE_ROLL)
	{
		angleRoll = pSettings->viewRoll * M_PI / 180.0;
	}
	else
	{
		angleRoll = 0;
	}

	refVec.x = 0;
	refVec.y = 0;
	refVec.z = 1.0;

    anglesToCoord(angleRoll,anglePitch,angleYaw, &refVec);

    /* assume x = 0 and y = 1 => find matching vector so axis_1 is 90° to axis_2 */
    axis_1.x = 0;
    if(refVec.y >=0)
    {
    	axis_2.y = 1; /* => Spawn y == refVec y */
    }
    else axis_1.y = -1;
    axis_1.z = -1.0 * refVec.y / refVec.z;
    axis_2 = CoordCross(refVec, axis_1);	/* Cross is 90° to refVec and Spawn as well => Plane Spawn / cross */

    /* check if detection plane is correct */
	u = CoordSub(axis_1,refVec);
    v = CoordSub(axis_2,refVec);
    n = CoordCross(u,v);

    if((fabsf(n.x) <= 0.0001) && (fabsf(n.y) <= 0.0001) && (fabsf(n.z) <= 0.0001))
    {
    	retval = 2;
    }
    else
    {
    	if(enableAxis & MOTION_ENABLE_PITCH)
    	{
    		anglePitch = pitch * M_PI / 180.0;
    	}
    	else
    	{
    		anglePitch = 0.0;
    	}
    	if(enableAxis & MOTION_ENABLE_ROLL)
    	{
    		angleRoll = roll * M_PI / 180.0;
    	}
    	else
    	{
    		angleRoll = 0.0;
    	}
    	if(enableAxis & MOTION_ENABLE_YAW)
    	{
    		angleYaw = compYaw * M_PI / 180.0;
    	}
    	else
    	{
    		angleYaw = 0.0;
    	}

    	curVec.x = 0;
    	curVec.y = 0;
    	curVec.z = 1.0;
		anglesToCoord(angleRoll,anglePitch,angleYaw, &curVec);

		_a = CoordDot(curVec,n);
		_b = CoordDot(refVec,n);

		if(_b>=(-0.0001)&&_b<=0.0001)		/* Check if view port is parallel (no matchpoint) */
		{
			retval = 3;
		}
		else
		{
			r=_a/_b;
			if(r<0.00||r>1.40)				/* are we looking into wrong direction? */
			{
				retval = 4;
			}
		}
		distance = retval * 1.0;			/* just for debugging */
		if(retval == 0)
		{
			/* start calculating the matchpoint */
			curVec = CoordMulF(curVec,r);
			resultVec = CoordSub(refVec,curVec);

			/* calculate the distance between reference and actual vector */
			resultVec.x = resultVec.x * resultVec.x;
			resultVec.y = resultVec.y * resultVec.y;
			resultVec.z = resultVec.z * resultVec.z;

			if((resultVec.x == 0) && (resultVec.y == 0) && (resultVec.z == 0))
			{
				distance = 0.0;
			}
			else
			{
				distance = sqrtf((resultVec.x + resultVec.y + resultVec.z));
			}
		}
    }

    if(distance < 0.5)		/* handle focus counter to avoid fast in/out focus changes */
    {
		if(focusCnt < 10)
		{
			if((focusCnt == 9) && (inFocus == 0)) /* we will get into focus */
			{
				resetMotionDeltaHistory();
			}
			focusCnt++;
		}
		if((focusCnt == 10) && (inFocus == 0))
		{
			inFocus = 1;
		}
	}
	else
	{
		if(focusCnt >= 5)						/* Reset focus faster then setting focus */
		{
			focusCnt--;
		}
		else
		{
			focusCnt = 0;
			inFocus = 0;
		}
	}
    return distance;
}
uint8_t viewInFocus(void)
{
	return inFocus;
}
void resetFocusState(void)
{
	inFocus = 0;
}

void suspendMotionDetection(uint8_t seconds)
{
	suspendMotionDetectionSec = seconds * 10;		/* detection function is called every 100ms */
}

void HandleMotionDetection(void)
{
    detectionState_t pitchstate = DETECT_NOTHING;
    static uint8_t wasInFocus = 0;
    float focusOffset = 0.0;

	evaluateMotionDelta(stateUsed->lifeData.compass_roll, stateUsed->lifeData.compass_pitch, stateUsed->lifeData.compass_heading);
	if(viewInFocus())
	{
		focusOffset = checkViewport(stateUsed->lifeData.compass_roll, stateUsed->lifeData.compass_pitch, stateUsed->lifeData.compass_heading, (MOTION_ENABLE_PITCH | MOTION_ENABLE_YAW));
	}
	else
	{
		focusOffset = checkViewport(stateUsed->lifeData.compass_roll, stateUsed->lifeData.compass_pitch, stateUsed->lifeData.compass_heading, MOTION_ENABLE_ALL);
	}
	if(viewInFocus())
	{
		wasInFocus = 1;
		set_Backlight_Boost(settingsGetPointer()->viewPortMode & 0x03);

		if(suspendMotionDetectionSec == 0)					/* suspend detection while diver is manually operating the OSTC */
		{
			switch(settingsGetPointer()->MotionDetection)
			{
				case MOTION_DETECT_MOVE: pitchstate = detectPitch(stateRealGetPointer()->lifeData.compass_pitch);
					break;
				case MOTION_DETECT_SECTOR: pitchstate = detectSectorButtonEvent(focusOffset);
					break;
				case MOTION_DETECT_SCROLL: pitchstate = detectScrollButtonEvent(focusOffset);
					 break;
				default:
					pitchstate = DETECT_NOTHING;
					break;
			}
		}

		if(DETECT_NEG_PITCH == pitchstate)
		{
			StoreButtonAction((uint8_t)ACTION_PITCH_NEG);
		}
		if(DETECT_POS_PITCH == pitchstate)
		{
			StoreButtonAction((uint8_t)ACTION_PITCH_POS);
		}
	}
	else
	{
		if(wasInFocus)
		{
			wasInFocus = 0;
			if(suspendMotionDetectionSec == 0)
			{
				if(settingsGetPointer()->design == 7)
				{
					t7_set_customview_to_primary();
				}
				else
				{
					t3_set_customview_to_primary();
				}
			}
		}
		set_Backlight_Boost(0);
	}
	if(suspendMotionDetectionSec != 0)
	{
		suspendMotionDetectionSec--;
	}
}



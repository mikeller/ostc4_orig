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

#define	STABLE_STATE_COUNT			2	/* number of count to declare a state as stable (at the moment based on 100ms) */
#define STABLE_STATE_TIMEOUT		5	/* Detection shall be aborted if a movement state is stable for more than 500ms */

#define SECTOR_WINDOW				30.0  	/* Pitch window which is used for custom view projection */
#define SECTOR_WINDOW_MAX			120.0  	/* Pitch window which will be greater than the divers field of view */
#define SECTOR_HYSTERY				2		/* Additional offset to avoid fast changing displays */
#define SECTOR_BORDER				400.0	/* Define a value which is out of limit to avoid not wanted key events */
#define SECTOR_FILTER				10		/* Define speed for calculated angle to follow real value */

#define SECTOR_MAX					24		/* maximum number of sectors */
#define SECTOR_SCROLL				7		/* number of sectors used for scroll detection */

#define MOTION_DELTA_STABLE			0
#define MOTION_DELTA_JITTER			1
#define MOTION_DELTA_RAISE			2
#define MOTION_DELTA_FALL			3

#define MOTION_DELTA_JITTER_LEVEL	3.0		/* lower values are considered as stable */
#define MOTION_DELTA_RAISE_LEVEL	6.0		/* Movement causing a significant change detected */
#define MOTION_DELTA_FALL_LEVEL		-6.0	/* Movement causing a significant change detected */

#define MOTION_DELTA_HISTORY_SIZE	20		/* Number of history data sets */

detectionState_t detectionState = DETECT_NOTHING;
SSector sectorDetection;

static uint8_t motionDeltaHistory[3][MOTION_DELTA_HISTORY_SIZE];			/* Change history of roll, pitch and yaw */
static uint8_t motionDeltaHistoryIdx;										/* Current index of history data */

static uint8_t focusCnt = 0;
static uint8_t inFocus = 0;

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

uint8_t GetSectorForPitch(float pitch)
{
	static uint8_t lastsector = 0;
	float newPitch;
	uint8_t sector = 0;

	newPitch = pitch + sectorDetection.offset + sectorDetection.center;		/* do not use negative values and consider offset to center position */
	if (newPitch < 0.0)							/* clip value */
	{
		newPitch = 0.0;
	}
	if (newPitch > sectorDetection.window)							/* clip value */
	{
		newPitch = sectorDetection.window;
	}

	/* switch to other sector? */
	if((newPitch > sectorDetection.upperborder) || (newPitch <= sectorDetection.lowerborder))
	{
		sector = (uint16_t) newPitch / sectorDetection.size;
		sectorDetection.lowerborder = sector * sectorDetection.size - SECTOR_HYSTERY;
		sectorDetection.upperborder = (sector + 1) * sectorDetection.size + SECTOR_HYSTERY;
		lastsector = sector;
	}

	return lastsector;
}

void DefinePitchSectors(float centerPitch,uint8_t numOfSectors)
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
		if(sectorDetection.count > 5)
		{
			sectorDetection.count = 5;	/* more views are hard to manually control */
		}
	}
	else
	if(numOfSectors != CUSTOMER_KEEP_LAST_SECTORS)
	{
		sectorDetection.count = numOfSectors;
	}

	if(sectorDetection.count == SECTOR_MAX)
	{
		sectorDetection.window = SECTOR_WINDOW_MAX;
	}
	else
	{
		sectorDetection.window = SECTOR_WINDOW;
	}

	sectorDetection.offset = (centerPitch - (sectorDetection.window / 2)) * -1.0;
	sectorDetection.size = sectorDetection.window / sectorDetection.count;
	sectorDetection.center = 0;

/* reset border values */
	sectorDetection.lowerborder = SECTOR_BORDER;
	sectorDetection.upperborder = SECTOR_BORDER * -1.0;
/* get the current sector */
	sectorDetection.current = GetSectorForPitch(stateRealGetPointer()->lifeData.compass_pitch);
	sectorDetection.target = sectorDetection.current;
/* do a small adjustment to center pitch to make sure the actual pitch is in the center of the current sector */
	sectorDetection.center = (sectorDetection.upperborder) - ((sectorDetection.size + 2 *SECTOR_HYSTERY) / 2.0) - (centerPitch + sectorDetection.offset);

}

void InitMotionDetection(void)
{
	sectorDetection.target = 0;
	sectorDetection.current = 0;
	sectorDetection.size = 0;
	sectorDetection.count = 0;

	switch(settingsGetPointer()->MotionDetection)
	{
		case MOTION_DETECT_SECTOR: DefinePitchSectors(settingsGetPointer()->viewPitch,CUSTOMER_DEFINED_VIEWS);
			break;
		case MOTION_DETECT_MOVE: DefinePitchSectors(settingsGetPointer()->viewPitch,SECTOR_MAX);
			break;
		case MOTION_DETECT_SCROLL: DefinePitchSectors(settingsGetPointer()->viewPitch,SECTOR_SCROLL);
			break;
		default:
			break;
	}

	resetMotionDeltaHistory();
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
		sectorDetection.target = newTargetSector;
	}
	lastTargetSector = newTargetSector;
	if(sectorDetection.target != sectorDetection.current)
	{
		 if(sectorDetection.target > sectorDetection.current)
		 {
			 sectorDetection.current++;
			PitchEvent = DETECT_POS_PITCH;
		 }
		 else
		 {
			 sectorDetection.current--;
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
		/* for scroll detection the motion window is split into 6 sectors => set event accoring to the sector number*/
		switch(newSector)
		{
			case 0: PitchEvent = DETECT_POS_PITCH;
				break;
			case 6:	PitchEvent = DETECT_NEG_PITCH;
				break;
			default:
				break;
		}
		if(PitchEvent != DETECT_NOTHING)
		{
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

    pSettings->viewPitch = pitch;
	pSettings->viewRoll = roll;
	pSettings->viewYaw = yaw;
}


float checkViewport(float roll, float pitch, float yaw)
{
	static float freezeRoll = 0;
	static float freezeYaw = 0;

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
	SDeltaHistory test;

	SSettings* pSettings = settingsGetPointer();

	/* calculate base vector taking calibration delta into account yaw (heading) */
	float compYaw = yaw + pSettings->viewYaw;

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
	anglePitch = pSettings->viewPitch * M_PI / 180.0;
	angleRoll = pSettings->viewRoll * M_PI / 180.0;

	refVec.x = 0;
	refVec.y = 0;
	refVec.z = 1.0;

    anglesToCoord(angleRoll,anglePitch,angleYaw, &refVec);

	anglePitch = pitch * M_PI / 180.0;
	angleRoll = roll * M_PI / 180.0;
    angleYaw = yaw * M_PI / 180.0;

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
    	angleYaw = compYaw * M_PI / 180.0;
    	anglePitch = pitch * M_PI / 180.0;
    	angleRoll = roll * M_PI / 180.0;
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
			freezeRoll = roll;
			freezeYaw = yaw;
		}
	}
	else
	{
		if(focusCnt >= 5)												/* Reset focus faster then setting focus */
		{
			if(pSettings->MotionDetection != MOTION_DETECT_MOVE)		/* only apply extended focus for methods using absolute pitch values */
			{
				test = GetDeltaHistory(0);
				if((test.yaw == MOTION_DELTA_STABLE) && (test.roll == MOTION_DELTA_STABLE)) 
				{
					if((fabsf(freezeRoll - roll) < MOTION_DELTA_JITTER_LEVEL) && (fabsf(freezeYaw - yaw) < MOTION_DELTA_JITTER_LEVEL))
					{
						focusCnt++;
					}
				}
				else
				{
					if(freezeRoll != 0.0)
					{
						focusCnt = 1;
					}
				}
			}
			focusCnt--;
		}
		else
		{
			focusCnt = 0;
			inFocus = 0;
			freezeRoll = 0;
			freezeYaw = 0;
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

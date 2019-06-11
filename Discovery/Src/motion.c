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

#define PITCH_HISTORY_ENTRIES		20	/* number of pitch value stored in buffer */
#define	STABLE_STATE_COUNT			2	/* number of count to declare a state as stable (at the moment based on 100ms) */
#define MOVE_DELTA_COUNT			10	/* Delta count needed to identify a valid movement */
#define SHAKE_DELTA_COUNT			30	/* Delta count needed to identify a valid minima / maxima */


typedef enum
{
		RELATIVE_MOVE_STATIC = 0,
		RELATIVE_MOVE_INCREASE,
		RELATIVE_MOVE_DECREASE,
		RELATIVE_MOVE_INVALID
} relativeMove_t;





float pitchHistory[PITCH_HISTORY_ENTRIES];			/* Ringbuffer of last read pich values */
static uint8_t pitchWriteIdx;			/* Index of current write slot */

/* Init data structures */
void InitMotion()
{
	uint8_t tmp;

	for(tmp = 0; tmp < PITCH_HISTORY_ENTRIES; tmp++)
	{
		pitchHistory[tmp] = 0;
	}
	pitchWriteIdx = 0;

}

detectionState_t detectionState = DETECT_NOTHING;

/* Detect if user is generating an pitch including return to starting position (shake) */
/* This is done by feeding the past movements value per value into a state machine */
detectionState_t detectShake(float currentPitch)
{
	static uint8_t runningIdx = 0;
	static uint8_t stableCnt = 0;
	static float lastPitch = 0.0;
	static float startPitch = 0.0;

	relativeMove_t relativeMove = RELATIVE_MOVE_INVALID;


	pitchHistory[pitchWriteIdx] = currentPitch;
	runningIdx = pitchWriteIdx;
#if 0
	runningIdx = pitchWriteIdx + 1;

	if(runningIdx == PITCH_HISTORY_ENTRIES)
	{
		runningIdx = 0;
	}
#endif

	if((detectionState == DETECT_NEG_SHAKE) || (detectionState == DETECT_POS_SHAKE))	/* discard last detection */
	{
		detectionState = DETECT_NOTHING;
	}
//	do
//	{
//		lastPitch = pitchHistory[runningIdx];
#if 0
		runningIdx++;
		if(runningIdx == PITCH_HISTORY_ENTRIES)
		{
			runningIdx = 0;
		}
#endif
	/* define relative movement compared to last position */
		if(fabsf(lastPitch -  pitchHistory[runningIdx]) < MOVE_DELTA_COUNT )		/* more or less no movement */
		{
			relativeMove = RELATIVE_MOVE_STATIC;
			stableCnt++;
		}
		else
		{
			if(lastPitch > pitchHistory[runningIdx])				/* decreasing */
			{
				relativeMove = RELATIVE_MOVE_DECREASE;
			}
			else
			{
				relativeMove = RELATIVE_MOVE_INCREASE;				/* increasing */
			}
		}

	/* feed value into statemachine */
		switch (detectionState)
		{
			case DETECT_NOTHING: 	if(relativeMove == RELATIVE_MOVE_STATIC)
									{
										if(stableCnt > 4)
										{
											detectionState = DETECT_START;
											startPitch = lastPitch;
										}
									}
				break;
			case DETECT_START:		switch(relativeMove)
									{
										case RELATIVE_MOVE_INCREASE: detectionState = DETECT_POS_MOVE;
											break;
										case RELATIVE_MOVE_DECREASE: detectionState = DETECT_NEG_MOVE;
											break;
										default:
											break;
									}
				break;
			case DETECT_POS_MOVE:	switch(relativeMove)
									{
										case RELATIVE_MOVE_INCREASE: detectionState = DETECT_POS_MOVE;
											break;
										case RELATIVE_MOVE_STATIC: if(fabsf(startPitch - lastPitch) > SHAKE_DELTA_COUNT)
																	{
																		detectionState = DETECT_MAXIMA;
																	}
																	else
																	{
																		detectionState = DETECT_NOTHING;
																	}
											break;
										default:
											detectionState = DETECT_NOTHING;
											break;
									}
				break;
			case DETECT_NEG_MOVE:	switch(relativeMove)
									{
										case RELATIVE_MOVE_DECREASE: detectionState = DETECT_NEG_MOVE;
											break;
										case RELATIVE_MOVE_STATIC:	if(fabsf(startPitch - lastPitch) > SHAKE_DELTA_COUNT)   /* significant movment */
																	{
																		detectionState = DETECT_MINIMA;
																	}
																	else
																	{
																		detectionState = DETECT_NOTHING;
																	}
											break;
										default:
											detectionState = DETECT_NOTHING;
											break;
									}
				break;
			case DETECT_MAXIMA:		if((relativeMove != RELATIVE_MOVE_STATIC) && (stableCnt < STABLE_STATE_COUNT ))
									{
										detectionState = DETECT_NOTHING;
									}
									else
									{
										if(relativeMove == RELATIVE_MOVE_DECREASE)
										{
											detectionState = DETECT_FALLBACK;
										}
										if(relativeMove == RELATIVE_MOVE_INCREASE)
										{
											detectionState = DETECT_POS_MOVE;
										}
									}
				break;
			case DETECT_MINIMA:		if((relativeMove != RELATIVE_MOVE_STATIC) && (stableCnt < STABLE_STATE_COUNT ))
									{
										detectionState = DETECT_NOTHING;
									}
									else
									{
										if(relativeMove == RELATIVE_MOVE_DECREASE)
										{
											detectionState = DETECT_NEG_MOVE;
										}
										if(relativeMove == RELATIVE_MOVE_INCREASE)
										{
											detectionState = DETECT_RISEBACK;
										}
									}
				break;

			case DETECT_FALLBACK:	switch(relativeMove)
									{
										case RELATIVE_MOVE_DECREASE: detectionState = DETECT_FALLBACK;
											break;
										case RELATIVE_MOVE_STATIC:  if( stableCnt >= STABLE_STATE_COUNT)
																	{
																		if(fabsf(startPitch - lastPitch) < MOVE_DELTA_COUNT) 		/* are we where started, again? */
																		{
																			detectionState = DETECT_POS_SHAKE;
																			memset(pitchHistory, 0, sizeof(pitchHistory));
																		}
																		else
																		{
																			detectionState = DETECT_START;							/* start new detection */
																			startPitch = lastPitch;
																		}
																	}
											break;
										default:
											detectionState = DETECT_NOTHING;
											break;
									}
				break;
			case DETECT_RISEBACK:	switch(relativeMove)
									{
										case RELATIVE_MOVE_INCREASE: detectionState = DETECT_RISEBACK;
											break;
										case RELATIVE_MOVE_STATIC:  if(stableCnt >= STABLE_STATE_COUNT)
																	{
																		if(fabsf(startPitch - lastPitch) < MOVE_DELTA_COUNT)
																		{
																			detectionState = DETECT_NEG_SHAKE;
																			memset(pitchHistory, 0, sizeof(pitchHistory));
																		}
																		else
																		{
																			detectionState = DETECT_START;
																			startPitch = lastPitch;
																		}
																	}
											break;
										default:
											detectionState = DETECT_NOTHING;
											break;
									}
				break;

			default:
				break;

		}
		if(relativeMove != RELATIVE_MOVE_STATIC)		/* reset counter for stable time detection */
		{
			stableCnt = 0;
		}
//	} while ((runningIdx != pitchWriteIdx) && (detectionState != DETECT_NEG_SHAKE) && (detectionState != DETECT_POS_SHAKE));

	lastPitch = currentPitch;
	pitchWriteIdx++;
	if(pitchWriteIdx == PITCH_HISTORY_ENTRIES)
	{
		pitchWriteIdx = 0;
	}
	return detectionState;
}

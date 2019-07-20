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

#define	STABLE_STATE_COUNT			2	/* number of count to declare a state as stable (at the moment based on 100ms) */
#define STABLE_STATE_TIMEOUT		5	/* Detection shall be aborted if a movement state is stable for more than 500ms */
#define MOVE_DELTA_SPEED			4	/* Delta speed needed to identify a valid movement */
#define SHAKE_DELTA_COUNT			10	/* Delta count needed to identify a valid minima / maxima */
#define SHAKE_DELTA_END				10	/* Delta allowed between start and end position */

static detectionState_t detectionState = DETECT_NOTHING;

/* Detect if user is generating an pitch including return to starting position (shake) */
/* This is done by feeding the past movements value per value into a state machine */
detectionState_t detectShake(float currentPitch)
{
	static uint8_t stableCnt = 0;
	static float lastPitch = 0.0;
	static float startPitch = 0.0;
	static float minmax = 0.0;
	float curSpeed;

	if((detectionState == DETECT_NEG_SHAKE) || (detectionState == DETECT_POS_SHAKE))	/* discard last detection */
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
											if(fabsf(startPitch - currentPitch) > SHAKE_DELTA_COUNT)
											{
												detectionState++;
												minmax = lastPitch;
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
			case DETECT_MAXIMA:		if(fabsf(currentPitch - minmax ) < SHAKE_DELTA_COUNT)		/* stay at maximum for short time to add a pattern for user interaction */
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
										if(fabsf(startPitch - currentPitch) < SHAKE_DELTA_END)
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

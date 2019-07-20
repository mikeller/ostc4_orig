/*
 * motion.h
 *
 *  Created on: 20.05.2019
 *      Author: Thorsten Sonntag
 */

#ifndef INC_MOTION_H_
#define INC_MOTION_H_


/* exported data types */
typedef enum
{
		DETECT_START = 0,
		DETECT_POS_MOVE,
		DETECT_MAXIMA,
		DETECT_FALLBACK,
		DETECT_POS_SHAKE,
		DETECT_NEG_MOVE,
		DETECT_MINIMA,
		DETECT_RISEBACK,
		DETECT_NEG_SHAKE,
		DETECT_NOTHING
} detectionState_t;

detectionState_t detectShake(float currentPitch);

#endif /* INC_MOTION_H_ */

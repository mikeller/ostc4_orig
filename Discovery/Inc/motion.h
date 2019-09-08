/*
 * motion.h
 *
 *  Created on: 20.05.2019
 *      Author: Thorsten Sonntag
 */

#ifndef INC_MOTION_H_
#define INC_MOTION_H_


/* exported data types */
#define CUSTOMER_DEFINED_VIEWS	(100u)	/* value will cause the function to detect the number of selected views */
#define CUSTOMER_KEEP_LAST_SECTORS	(200u)	/* do not update number of sectors, just define the new center position */

typedef enum
{
		MOTION_DETECT_OFF = 0,
		MOTION_DETECT_SECTOR,
		MOTION_DETECT_MOVE,
		MOTION_DETECT_SCROLL
} MotionDetectMethod_t;

typedef enum
{
		DETECT_START = 0,
		DETECT_POS_MOVE,
		DETECT_MAXIMA,
		DETECT_FALLBACK,
		DETECT_POS_PITCH,
		DETECT_NEG_MOVE,
		DETECT_MINIMA,
		DETECT_RISEBACK,
		DETECT_NEG_PITCH,
		DETECT_NOTHING
} detectionState_t;

typedef struct
{
    float upperlimit;
    float lowerlimit;
} SSector;

void InitMotionDetection(void);
void DefinePitchSectors(float centerAngle, uint8_t numOfSectors);
detectionState_t detectPitch(float currentPitch);
detectionState_t detectSectorButtonEvent(float curPitch);
detectionState_t detectScrollButtonEvent(float curPitch);

#endif /* INC_MOTION_H_ */

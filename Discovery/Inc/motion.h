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

#define MOTION_ENABLE_ROLL	(0x01u)
#define MOTION_ENABLE_PITCH	(0x02u)
#define MOTION_ENABLE_YAW	(0x04u)
#define MOTION_ENABLE_ALL	(MOTION_ENABLE_ROLL | MOTION_ENABLE_PITCH | MOTION_ENABLE_YAW)

typedef enum
{
		MOTION_DETECT_OFF = 0,
		MOTION_DETECT_SECTOR,
		MOTION_DETECT_MOVE,
		MOTION_DETECT_SCROLL,
		MOTION_DETECT_END
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
    float upperborder;			/* current sector is changed if pitch exceeds these borders */
    float lowerborder;

    uint8_t current;			/* number of the current visible sector */
    uint8_t target;				/* used for sector switch => number of the sector which shall be finnaly displayed */

    float offset;				/* offset to adjust minimum pitch value used for detection to zero */
    float size;					/* delta of upper and lower boarder defining the sector in degree */
    float window;				/* defines which range of pitch values are used for detection */
    float center;				/* defines the offset from pitch to middle of active sector (avoid center position is close to the calculated borders) */
    uint8_t count;				/* number of sectors used for detection */
} SSector;

typedef struct
{
	float x;
	float y;
	float z;
} SCoord;

typedef enum
{
		MOTION_HISTORY_ROLL = 0,
		MOTION_HISTORY_PITCH,
		MOTION_HISTORY_YAW
} MotionHistoryEntry_t;

typedef struct
{
	uint8_t roll;
	uint8_t pitch;
	uint8_t yaw;
} SDeltaHistory;

void InitMotionDetection(void);
void suspendMotionDetection(uint8_t seconds);
void HandleMotionDetection(void);
void resetMotionDeltaHistory();
void evaluateMotionDelta(float roll, float pitch, float yaw);
void DefineSectorCount(uint8_t numOfSectors);
void MapCVToSector(void);
detectionState_t detectPitch(float currentPitch);
detectionState_t detectSectorButtonEvent(float focusOffset);
detectionState_t detectScrollButtonEvent(float focusOffset);

void anglesToCoord(float roll, float pitch, float yaw, SCoord *pCoord);
void calibrateViewport(float roll, float pitch, float yaw);
float checkViewport(float roll, float pitch, float yaw, uint8_t enableAxis);
uint8_t viewInFocus(void);
uint8_t viewDetectionSuspended(void);
void resetFocusState(void);

#endif /* INC_MOTION_H_ */

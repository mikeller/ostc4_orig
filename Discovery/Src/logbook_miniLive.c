/**
  ******************************************************************************
	* @copyright heinrichs weikamp
  * @file   		logbook_miniLive.c
  * @author 		heinrichs weikamp gmbh
  * @date   		13-March-2015
  * @version		V0.0.1
  * @since			13-March-2015
  * @brief			little logbook for during the dive
	* @bug
	* @warning
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 heinrichs weikamp</center></h2>
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/


#include <string.h>
#include "logbook_miniLive.h"
#include "data_exchange.h"
#include "logbook.h"

 /*
  ******************************************************************************
  * @brief   t7_updateMiniLiveLogbook. /  Create depth samples for view during dive
  * @author  heinrichs weikamp gmbh
  * @version V0.0.1
  * @date    13-March-2015
  ******************************************************************************
	*
  */
	
#define MLLsize (296)
static uint16_t MLLdataDepth[MLLsize];
static uint16_t MLLpointer  = 0;
static uint8_t MLLtickIntervallSeconds = 2;

/* Replay Block data storage */
#define DEPTH_DATA_LENGTH	(1800u)				/* Resolution: 5 hours dive, sampling every 10 seconds */
uint16_t depthdata[DEPTH_DATA_LENGTH];
uint16_t livedepthdata[DEPTH_DATA_LENGTH * 2];
static uint16_t historyIndex = 0;

static uint8_t	ReplayDataResolution = 2;		/* Time represented by one sample (second) */
static uint16_t ReplayDataLength = 0;			/* Number of data entries */
static uint16_t ReplayDataMaxDepth = 0;
static uint16_t ReplayDataMinutes = 0;
static uint16_t ReplayDataOffset = 0xFFFF;		/* Stepbackwards format used by log functions */

uint16_t *getMiniLiveLogbookPointerToData(void)
{
	return MLLdataDepth;
}


uint16_t getMiniLiveLogbookActualDataLength(void)
{
	return MLLpointer;
}
	
void compressBuffer_uint16(uint16_t* pdata, uint16_t size)
{
	uint16_t* pTarget = pdata;
	uint16_t* pSource = pdata;
	
	uint16_t index = 0;
	
	for(index = 0; index < size/2; index++)
	{
		*pTarget = *pSource++;
		*pTarget += *pSource++;
		*pTarget++ /= 2;
	}
	memset(pTarget,0,size/2);
}

void updateMiniLiveLogbook( _Bool checkOncePerSecond)
{
	static uint8_t bDiveMode = 0;
	static uint32_t last_second = 0;
	static uint8_t secondsCount = 0;
	static uint8_t historysecondsCount = 0;

	if(checkOncePerSecond)
	{
		uint32_t now =  current_second();
		if( last_second == now)
				return;
		last_second = now;
	}
	secondsCount++;
	historysecondsCount++;
	
	if(!bDiveMode)
	{
		if((stateUsed->mode == MODE_DIVE) && (stateUsed->lifeData.dive_time_seconds >= 5))
		{
			secondsCount = 0;
			MLLtickIntervallSeconds = 2;
			bDiveMode = 1;
			MLLpointer = 1;
			for(int i=0;i<MLLsize;i++)
				MLLdataDepth[i] = 0;

			for(historyIndex = 0; historyIndex < DEPTH_DATA_LENGTH; historyIndex++)
			{
				livedepthdata[historyIndex] = 0xFFFF;
			}
			historysecondsCount = 0;
			historyIndex = 0;
			livedepthdata[historyIndex++] = 0;	/* start at 0 */
		}
	}
	else if(stateUsed->mode == MODE_DIVE)
	{
		bDiveMode = 3;
		//
		if(secondsCount >= MLLtickIntervallSeconds)
		{
			secondsCount = 0;
			/* in case of a buffer overrun the buffer is divided and the first half is filled with a compressed image of the complete buffer */
			if((MLLpointer >= MLLsize) && (MLLtickIntervallSeconds < 127))
			{
				MLLpointer = 0;
				MLLtickIntervallSeconds *= 2;

				compressBuffer_uint16(MLLdataDepth,MLLsize);
				MLLpointer = MLLsize/2;
			}
			if(MLLpointer < MLLsize)
				MLLdataDepth[MLLpointer++] = (int)(stateUsed->lifeData.depth_meter * 10);
		}
		if(historysecondsCount > ReplayDataResolution)
		{
			historysecondsCount = 0;

			if(historyIndex >= 2*DEPTH_DATA_LENGTH)		/* compress data */
			{
				ReplayDataResolution *= 2;
				compressBuffer_uint16(livedepthdata,2*DEPTH_DATA_LENGTH);
				historyIndex = DEPTH_DATA_LENGTH;
			}
			livedepthdata[historyIndex++] = (int)(stateUsed->lifeData.depth_meter * 100);
		}
	}
	else if(bDiveMode == 3)
	{
		//End of Dive
		for(int i=0;i<MLLsize;i++)
			MLLdataDepth[i] = 0;
		bDiveMode = 0;
	}
}

uint8_t prepareReplayLog(uint8_t StepBackwards)
{
	uint8_t retVal = 0;
	uint16_t dataLength = 0;

    SLogbookHeader logbookHeader;

    if(ReplayDataOffset == StepBackwards)				/* Entry already selected => reset selection */
    {
    	ReplayDataOffset = 0xFFFF;
    	ReplayDataResolution = 2;
    	retVal = 1;
    }
    else
    {
    	ReplayDataOffset = StepBackwards;
		logbook_getHeader(StepBackwards ,&logbookHeader);

		dataLength = logbook_readSampleData(StepBackwards, DEPTH_DATA_LENGTH, depthdata,NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

		if( dataLength == DEPTH_DATA_LENGTH)		/* log data has been compressed to fit into buffer */
		{
			ReplayDataResolution = (logbookHeader.diveTimeMinutes * 60 + logbookHeader.diveTimeSeconds) / dataLength;
		}
		else
		{
			ReplayDataResolution = logbookHeader.samplingRate;
		}
		ReplayDataLength = dataLength;
		ReplayDataMaxDepth = logbookHeader.maxDepth;
		ReplayDataMinutes =  logbookHeader.diveTimeMinutes;
		if(dataLength != 0)
		{
			retVal = 1;
		}
    }
	return retVal;
}

uint8_t getReplayInfo(uint16_t** pReplayData, uint16_t* DataLength, uint16_t* MaxDepth, uint16_t* diveMinutes)
{
	uint8_t retVal = 0;

	if((ReplayDataOffset != 0xFFFF) && (pReplayData != NULL) && (DataLength != NULL) && (MaxDepth != NULL))
	{
		*pReplayData = depthdata;
		*DataLength = ReplayDataLength;
		*MaxDepth = ReplayDataMaxDepth;
		*diveMinutes = ReplayDataMinutes;
		retVal = 1;
	}

	return retVal;
}

uint16_t *getMiniLiveReplayPointerToData(void)
{
	return livedepthdata;
}
uint16_t getMiniLiveReplayLength(void)
{
	return historyIndex;
}

uint16_t getReplayOffset(void)
{
	return ReplayDataOffset;
}

uint16_t getReplayDataResolution(void)
{
	return ReplayDataResolution;
}

/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/

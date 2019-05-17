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
#include "logbook_miniLive.h"
#include "data_exchange.h"

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
	

uint16_t *getMiniLiveLogbookPointerToData(void)
{
	return MLLdataDepth;
}


uint16_t getMiniLiveLogbookActualDataLength(void)
{
	return MLLpointer;
}
	
	
	
void updateMiniLiveLogbook( _Bool checkOncePerSecond)
{
	static uint8_t bDiveMode = 0;
	static uint32_t last_second = 0;
	static uint8_t secondsCount = 0;

	if(checkOncePerSecond)
	{
		uint32_t now =  current_second();
		if( last_second == now)
				return;
		last_second = now;
	}
	secondsCount++;
	
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
		}
	}
	else if(stateUsed->mode == MODE_DIVE)
	{
		bDiveMode = 3;
		//
		if(secondsCount >= MLLtickIntervallSeconds)
		{
			secondsCount = 0;
			if((MLLpointer >= MLLsize) && (MLLtickIntervallSeconds < 127))
			{
				MLLpointer = 0;
				MLLtickIntervallSeconds *= 2;
				for(int i=0;i<MLLsize/2;i++)
				{
					MLLdataDepth[i] = MLLdataDepth[MLLpointer++];
					MLLdataDepth[i] += MLLdataDepth[MLLpointer++];
					MLLdataDepth[i] /= 2;
				}
				MLLpointer = MLLsize/2;
				for(int i=MLLsize/2;i<MLLsize;i++)
					MLLdataDepth[i] = 0;
			}
			if(MLLpointer < MLLsize)
				MLLdataDepth[MLLpointer++] = (int)(stateUsed->lifeData.depth_meter * 10);
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



/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/

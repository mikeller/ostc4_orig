/**
  ******************************************************************************
  * @file    rtc.c 
  * @author  heinrichs weikamp gmbh
  * @version V0.0.1
  * @date    10-Oct-2014
  * @brief   Source code for rtc control
  *           
  @verbatim                 
  ============================================================================== 
                        ##### How to use #####
  ============================================================================== 
  @endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 heinrichs weikamp</center></h2>
  *
  ******************************************************************************
  */ 
/* Includes ------------------------------------------------------------------*/
#include "rtc.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_conf.h"
#include "baseCPU2.h"

RTC_HandleTypeDef RTCHandle;

static void RTC_Error_Handler(void);


void RTC_SetTime(RTC_TimeTypeDef stimestructure)
{

	stimestructure.SubSeconds = 0;
  stimestructure.TimeFormat = RTC_HOURFORMAT12_AM;
  stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
  stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;

  if(HAL_RTC_SetTime(&RTCHandle, &stimestructure, FORMAT_BIN) != HAL_OK)
  {
    RTC_Error_Handler(); 
  }
}


void RTC_SetDate(RTC_DateTypeDef sdatestructure)
{
  if(HAL_RTC_SetDate(&RTCHandle, &sdatestructure, FORMAT_BIN) != HAL_OK)
  {
    RTC_Error_Handler(); 
  }
}


/*
static void RTC_CalendarConfig(void)
{
  RTC_DateTypeDef sdatestructure;
  RTC_TimeTypeDef stimestructure;

  //##-1- Configure the Date #################################################
  // Set Date: Monday April 14th 2014 
  sdatestructure.Year = 0;
  sdatestructure.Month = RTC_MONTH_JANUARY;
  sdatestructure.Date = 1;
  sdatestructure.WeekDay = RTC_WEEKDAY_MONDAY;
  
  if(HAL_RTC_SetDate(&RTCHandle,&sdatestructure,FORMAT_BCD) != HAL_OK)
  {
    RTC_Error_Handler(); 
  } 
  
  //##-2- Configure the Time #################################################
  // Set Time: 02:00:00 
  stimestructure.Hours = 0;
  stimestructure.Minutes = 0;
  stimestructure.Seconds = 0;
  stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
  stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;
  
  if(HAL_RTC_SetTime(&RTCHandle,&stimestructure,FORMAT_BCD) != HAL_OK)
  {
    RTC_Error_Handler(); 
  }
  
  //##-3- Writes a data in a RTC Backup data Register0 #######################
//  HAL_RTCEx_BKUPWrite(&RTCHandle,RTC_BKP_DR0,0x32F2);  
}
*/


  /* ##-1- Configure the RTC peripheral #######################################
		Configure RTC prescaler and RTC data registers 
		RTC configured as follow:
      - Hour Format    = Format 24
      - Asynch Prediv  = Value according to source clock
      - Synch Prediv   = Value according to source clock
      - OutPut         = Output Disable
      - OutPutPolarity = High Polarity
      - OutPutType     = Open Drain 
*/


void MX_RTC_init(void)
{

  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;
//  RTC_AlarmTypeDef sAlarm;

    /**Initialize RTC and set the Time and Date 
    */
  RTCHandle.Instance = RTC;
  RTCHandle.Init.HourFormat = RTC_HOURFORMAT_24;
  RTCHandle.Init.AsynchPrediv = 127;
  RTCHandle.Init.SynchPrediv = 255;
  RTCHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
  RTCHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  RTCHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  HAL_RTC_Init(&RTCHandle);

  sTime.Hours = 11;
  sTime.Minutes = 0;
  sTime.Seconds = 0;
  sTime.SubSeconds = 0;
  sTime.TimeFormat = RTC_HOURFORMAT12_AM;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  HAL_RTC_SetTime(&RTCHandle, &sTime, FORMAT_BCD);

  sDate.WeekDay = RTC_WEEKDAY_SUNDAY;
  sDate.Month = RTC_MONTH_FEBRUARY;
  sDate.Date = 15;
  sDate.Year = 17;
  HAL_RTC_SetDate(&RTCHandle, &sDate, FORMAT_BCD);


/*
  RTCHandle.Instance = RTC; 
  RTCHandle.Init.HourFormat = RTC_HOURFORMAT_24;
  RTCHandle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
  RTCHandle.Init.SynchPrediv = RTC_SYNCH_PREDIV;
  RTCHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
  RTCHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  RTCHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	
	HAL_RTC_Init(&RTCHandle);
*/	
}


void RTC_StopMode_2seconds(void)
{
    /* Enable Power Control clock */
    __HAL_RCC_PWR_CLK_ENABLE();

  /* Disable Wake-up timer */
  HAL_RTCEx_DeactivateWakeUpTimer(&RTCHandle);

	/* Enable Wake-up timer */
	HAL_RTCEx_SetWakeUpTimer_IT(&RTCHandle, (0x1000-1), RTC_WAKEUPCLOCK_RTCCLK_DIV16);

	/* FLASH Deep Power Down Mode enabled */
	HAL_PWREx_EnableFlashPowerDown();

	/*## Enter Stop Mode #######################################################*/
	HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

	/* Configures system clock after wake-up from STOP: enable HSI, PLL and select 
	PLL as system clock source (HSI and PLL are disabled in STOP mode) */
	SYSCLKConfig_STOP();
	
	HAL_RTCEx_DeactivateWakeUpTimer(&RTCHandle);
}


void RTC_Stop_11ms(void)
{
  /* Disable Wake-up timer */
  HAL_RTCEx_DeactivateWakeUpTimer(&RTCHandle);

	/* Enable Wake-up timer */
	HAL_RTCEx_SetWakeUpTimer_IT(&RTCHandle, (0x18-1), RTC_WAKEUPCLOCK_RTCCLK_DIV16);

	/* FLASH Deep Power Down Mode enabled */
	HAL_PWREx_DisableFlashPowerDown();

	/*## Enter Stop Mode #######################################################*/
	HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFI);

	/* Configures system clock after wake-up from STOP: enable HSI, PLL and select 
	PLL as system clock source (HSI and PLL are disabled in STOP mode) */
	SYSCLKConfig_STOP();
	
	HAL_RTCEx_DeactivateWakeUpTimer(&RTCHandle);
}


static void RTC_Error_Handler(void)
{
	while(1);
}	


/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/

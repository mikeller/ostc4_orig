/**
  ******************************************************************************
  * @file    stm32f4xx_hal_msp.c 
  * @author  heinrichs/weikamp, Christian Weikamp
  * @date    05-Dec-2014
  * @version V0.0.1
  * @since   05-Dec-2014
  * @brief   loader for the hardware specific stm32f4xx_hal_msp.c 
  *           
  @verbatim                 
  ============================================================================== 
              ##### How to use #####
  ============================================================================== 
  @endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 heinrichs weikamp</center></h2>
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/

#include "ostc.h"

#ifdef OSTC_ON_DISCOVERY_HARDWARE
 #include "stm32f4xx_hal_msp_discovery.c"
#else
 #include "stm32f4xx_hal_msp_hw1.c"
#endif

/* Exported variables --------------------------------------------------------*/  

/* Private types -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private variables with external access via get_xxx() function -------------*/

/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/

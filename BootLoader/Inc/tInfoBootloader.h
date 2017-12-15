/**
  ******************************************************************************
  * @file    tInfoBootloader.h  - bootloader only -
  * @author  heinrichs/weikamp, Christian Weikamp
  * @version V0.0.1
  * @date    08-Aug-2014
  * @brief   Header file communication with PC
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 heinrichs weikamp</center></h2>
  *
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef TINFO_BOOTLOADER_H
#define TINFO_BOOTLOADER_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
/** @addtogroup Base
  * @{
  */

/* Exported variables --------------------------------------------------------*/  


/* Exported functions --------------------------------------------------------*/  

void tInfoBootloader_init(void);
void tInfo_newpage(const char * text);
void tInfo_write(const char * text);
void tInfo_button_text(const char *text_left, const char *text_mid, const char *text_right);

#ifdef __cplusplus
}
#endif

#endif /* TINFO_BOOTLOADER_H */

/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/

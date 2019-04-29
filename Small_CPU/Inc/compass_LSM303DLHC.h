/**
  ******************************************************************************
  * @file    compass_LSM303DLHC.h
  * @author  heinrichs weikamp gmbh
  * @date    17-August-2017
  * @version V0.1.0
  * @since   17-August-2017
  * @brief   STMicroelectronics LSM303DLHC accelerometer & magnetometer driver
  *           
  @verbatim                 
  ============================================================================== 
                        ##### How to use #####
  ============================================================================== 
  @endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 heinrichs weikamp</center></h2>
  *
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef COMPASS_LSM303DLHC_H
#define COMPASS_LSM303DLHC_H

/* Exported constants --------------------------------------------------------*/

//#include "compass_LSM303D.h"

#define DLHC_CTRL_REG1_A		0x20
#define DLHC_CTRL_REG2_A		0x21
#define DLHC_CTRL_REG3_A		0x22
#define DLHC_CTRL_REG4_A		0x23
#define DLHC_CTRL_REG5_A		0x24
#define DLHC_CTRL_REG6_A		0x25

#define DLHC_CRA_REG_M			0x00
#define DLHC_CRB_REG_M			0x01
#define DLHC_MR_REG_M				0x02

#define DLHC_OUT_X_L_M      0x03
#define DLHC_OUT_X_H_M			0x04
#define DLHC_OUT_Y_L_M      0x05
#define DLHC_OUT_Y_H_M			0x06
#define DLHC_OUT_Z_L_M			0x07
#define DLHC_OUT_Z_H_M			0x08

// identisch mit 303D
#define DLHC_OUT_X_L_A			0x28
#define DLHC_OUT_X_H_A			0x29
#define DLHC_OUT_Y_L_A			0x2A
#define DLHC_OUT_Y_H_A			0x2B
#define DLHC_OUT_Z_L_A			0x2C
#define DLHC_OUT_Z_H_A			0x2D

#endif /* COMPASS_LSM303DLHC_H */

/******************* (C) COPYRIGHT 2017 heinrichs weikamp *****END OF FILE****/

/**	
 * |----------------------------------------------------------------------
 * | Copyright (C) Tilen Majerle, 2014
 * | 
 * | This program is free software: you can redistribute it and/or modify
 * | it under the terms of the GNU General Public License as published by
 * | the Free Software Foundation, either version 3 of the License, or
 * | any later version.
 * |  
 * | This program is distributed in the hope that it will be useful,
 * | but WITHOUT ANY WARRANTY; without even the implied warranty of
 * | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * | GNU General Public License for more details.
 * | 
 * | You should have received a copy of the GNU General Public License
 * | along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * |----------------------------------------------------------------------
 */
#include "tm_stm32f4_otp.h"

HAL_StatusTypeDef TM_OTP_Write(uint8_t block, uint8_t byte, uint8_t data)
{
	HAL_StatusTypeDef answer;

	/* Check input parameters */
	if (
		block >= OTP_BLOCKS ||
		byte >= OTP_BYTES_IN_BLOCK
	) {
		/* Invalid parameters */
		return HAL_ERROR;
	}

	if(*(uint8_t *)(OTP_START_ADDR + block * OTP_BYTES_IN_BLOCK + byte) != 0xFF)
		return HAL_ERROR;	

	/* Unlock FLASH */
	HAL_FLASH_Unlock();

	answer = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, (OTP_START_ADDR + block * OTP_BYTES_IN_BLOCK + byte), data);

	/* Lock FLASH */
	HAL_FLASH_Lock();

	return answer;
}


uint8_t TM_OTP_Read(uint8_t block, uint8_t byte) {
	uint8_t data;
	
	/* Check input parameters */
	if (
		block >= OTP_BLOCKS ||
		byte >= OTP_BYTES_IN_BLOCK
	) {
		/* Invalid parameters */
		return HAL_ERROR;
	}
	
	/* Get value */
	data = *(__IO uint8_t *)(OTP_START_ADDR + block * OTP_BYTES_IN_BLOCK + byte);
	
	/* Return data */
	return data;
}

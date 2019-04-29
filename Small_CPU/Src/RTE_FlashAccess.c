/**
  ******************************************************************************
  * @file    RTE_FLashAccess.c based on BonexFlashAccess.c based on firmwareEraseProgram.v
  * @author  heinrichs weikamp gmbh
  * @version V0.0.1
  * @date    20-July-2016
  * @version V0.0.1
  * @since   20-July-2016
  * @brief   erase and program the STM32F4xx internal FLASH memory for compasss calib etc.
	*					 based on firmwareEraseProgram.c from OSTC4
  *
  @verbatim
  ==============================================================================
                        ##### How to use #####
  ==============================================================================
	
	4 x 32 Byte with first block can not be 0xFFFFFFFF



	@endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 heinrichs weikamp</center></h2>
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "RTE_FlashAccess.h"
#include "stdio.h"

/** @addtogroup BONEXINFOSYSTEM
  * @{
  */

/* Exported variables --------------------------------------------------------*/

/* Private types -------------------------------------------------------------*/
/* taken from
 * C:\Users\hw\STM32Cube\Repository\STM32Cube_FW_F3_V1.2.0\Projects\STM32F3-Discovery\Examples\FLASH\FLASH_EraseProgram
 */
 
#define FLASH_SECTOR_SIZE_128KB	  (0x00020000)
#define FLASH_USER_START_ADDR			(ADDR_FLASH_SECTOR_7)   			/* Start @ of user Flash area */
#define FLASH_USER_END_ADDR   	 	(ADDR_FLASH_SECTOR_7 + FLASH_SECTOR_SIZE_128KB)   /* End @ of user Flash area */

/* Base address of the Flash pages */

#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */




/* Private variables ---------------------------------------------------------*/

//static FLASH_EraseInitTypeDef EraseInitStruct; //Variable used for Erase procedure

uint32_t Address = 0;
uint32_t PageError = 0;
__IO uint32_t data32 = 0 , MemoryProgramStatus = 0;

/* Private function prototypes -----------------------------------------------*/
//uint8_t BFA_eraseSectors(uint32_t SectorAddress, uint32_t NbSectors);
//uint8_t BFA_eraseSectorsAll(void);
uint8_t BFA_FindLastDataBlockAndSetAddress(void);

/* Exported functions --------------------------------------------------------*/

uint8_t BFA_readLastDataBlock(uint32_t *dataArray4)
{
	uint8_t answer;
	
	answer = BFA_FindLastDataBlockAndSetAddress();
	if(answer != BFA_OK)
		return answer;

	dataArray4[0] = *(__IO uint32_t*)(Address +  0);
	dataArray4[1] = *(__IO uint32_t*)(Address +  4);
	dataArray4[2] = *(__IO uint32_t*)(Address +  8);
	dataArray4[3] = *(__IO uint32_t*)(Address + 12);
	return BFA_OK;
}


uint8_t BFA_writeDataBlock(const uint32_t *dataArray4)
{
	uint8_t answer;
	uint32_t dataTest[4];
	uint32_t StartAddress;

	answer = BFA_FindLastDataBlockAndSetAddress();
	Address = Address + 16;

	if((answer == BFA_EMPTY) || (Address >= FLASH_USER_END_ADDR) || (Address < FLASH_USER_START_ADDR))
		Address = FLASH_USER_START_ADDR;
			
	dataTest[0] = *(__IO uint32_t*)(Address +  0);
	dataTest[1] = *(__IO uint32_t*)(Address +  4);
	dataTest[2] = *(__IO uint32_t*)(Address +  8);
	dataTest[3] = *(__IO uint32_t*)(Address + 12);
	
	for(int i=0;i<4;i++)
	{
		if(dataTest[i] != 0xFFFFFFFF)
		{
			return 0;
//			answer = BFA_eraseSectorsAll();
//			break;
		}
		else 
			answer = BFA_OK;
	}

	// can I write?
	if(answer != BFA_OK)
		return answer;

	StartAddress = Address;
	HAL_FLASH_Unlock();
	for(int i=0;i<4;i++)
	{
		answer = HAL_FLASH_Program(TYPEPROGRAM_WORD, Address, dataArray4[i]);
		Address = Address + 4;
	}
	HAL_FLASH_Lock();
	Address = StartAddress; // back to start of this data set (for reading etc.)
	return answer;
}


/* Private functions ---------------------------------------------------------*/
/*
uint8_t BFA_eraseSectorsAll(void)
{
	return BFA_eraseSectors(FLASH_USER_START_ADDR, (FLASH_USER_END_ADDR - FLASH_USER_START_ADDR)/FLASH_SECTOR_SIZE_128KB);
}


uint8_t BFA_eraseSectors(uint32_t SectorAddress, uint32_t NbSectors)
{
	if((NbSectors > 1) || (SectorAddress != FLASH_USER_START_ADDR))
		return 0;
	
	uint8_t answer;
	uint32_t PageError = 0;

  HAL_FLASH_Unlock();

  EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;//ERASE_SECTORS;
  EraseInitStruct.Sector = SectorAddress;
  EraseInitStruct.NbSectors = NbSectors;
  
	answer = HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
	
	HAL_FLASH_Lock();
	return answer;
}
*/

uint8_t BFA_FindLastDataBlockAndSetAddress(void)
{
	uint32_t StartAddress;

	// first part	from here to the end
	// there it should be, most likely at Address itself
	if(Address == 0)
		Address = FLASH_USER_END_ADDR - 16;			
	else
		Address &= 0xFFFFFFF0; // align with 16Byte
	
	StartAddress = Address;
  while (Address >= FLASH_USER_START_ADDR)
	{
		data32 = *(__IO uint32_t*)Address;
		if(data32 != 0xFFFFFFFF)
		{
			return BFA_OK;
		}
		Address = Address - 16;
	}

	// second part from the end to here
	if(StartAddress == FLASH_USER_END_ADDR - 16)
		return BFA_EMPTY;
	
	Address = FLASH_USER_END_ADDR - 16;			
  while (Address > StartAddress)
	{
		data32 = *(__IO uint32_t*)Address;
		if(data32 != 0xFFFFFFFF)
		{
			return BFA_OK;
		}
		Address = Address - 16;
	}

	// empty flash
	return BFA_EMPTY;
}
/**
  * @}
  */ 

/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/



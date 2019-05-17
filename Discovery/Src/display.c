
#include "stm32f4xx_hal.h" /* for HAL_Delay() */
#include "ostc.h"
#include "display.h"

#define ENABLE_EXTENDED_COMMANDS	0xB9
#define SET_POWER									0xB1
#define SLEEP_OUT									0x11
#define DISPLAY_INVERSION_OFF			0x20
#define MEMORY_ACCESS_ONTROL			0x36
#define INTERFACE_PIXEL_FORMAT		0x3A
#define SET_RGB_INTERFACE_RELATED	0xB3
#define SET_DISPLAY_WAVEFORM			0xB4
#define SET_PANEL									0xCC
#define SET_GAMMA_CURVE_RELATED		0xE0
#define DISPLAY_ON								0x29
#define DISPLAY_OFF								0x28
#define SLEEP_IN									0x10


static void Display_Error_Handler(void);

void display_power_on__1_of_2__pre_RGB(void)
{
	/* reset system */ 
	HAL_GPIO_WritePin(DISPLAY_CSB_GPIO_PORT,DISPLAY_CSB_PIN,GPIO_PIN_SET); // chip select

	HAL_GPIO_WritePin(DISPLAY_RESETB_GPIO_PORT,DISPLAY_RESETB_PIN,GPIO_PIN_RESET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(DISPLAY_RESETB_GPIO_PORT,DISPLAY_RESETB_PIN,GPIO_PIN_SET);
	HAL_Delay(10);

	/* RGB signals should be now for 2 frames or more (datasheet) */
}


static void send(uint8_t *pData, uint16_t inputlength)
{
	HAL_GPIO_WritePin(DISPLAY_CSB_GPIO_PORT,DISPLAY_CSB_PIN,GPIO_PIN_RESET); // chip select

	if(HAL_SPI_Transmit(&hspiDisplay,(uint8_t*)pData, inputlength, 10000) != HAL_OK)
		Display_Error_Handler();

	while (HAL_SPI_GetState(&hspiDisplay) != HAL_SPI_STATE_READY)
  {
  }
	HAL_GPIO_WritePin(DISPLAY_CSB_GPIO_PORT,DISPLAY_CSB_PIN,GPIO_PIN_SET); // chip select
}


static uint16_t convert8to9to8(uint8_t *pInput, uint8_t *pOutput,uint16_t inputlength)
{
	uint16_t outputlength;
	uint8_t readbit =  0x80;//0b1000000;
	uint8_t writebit = 0x40;//0b0100000;
	uint16_t i,j,k;

	outputlength = ((inputlength+7)/8)*9;

	for(i=0;i<outputlength;i++)
		pOutput[i] = 0;

	k = 0;
	for(i=0;i<inputlength;i++)
	{
		if(i != 0)
		{
			pOutput[k] |= writebit; // 9. bit
			writebit = writebit >> 1;
			if(writebit == 0)
			{
				writebit = 0x80;
				k++;
			}
		}
		for(j=0;j<8;j++)
		{
			if((pInput[i] & readbit) != 0)
			{
				pOutput[k] |= writebit;
			}
			readbit = readbit >> 1;
			if(readbit == 0)
				readbit = 0x80;
			writebit = writebit >> 1;
			if(writebit == 0)
			{
				writebit = 0x80;
				k++;
			}
		}
	}
	return outputlength;
}

void display_power_on__2_of_2__post_RGB(void)
{
	uint8_t aTxBuffer[32];
	uint8_t bTxBuffer[36];
	uint16_t i,length;

	for(i=0;i<32;i++)
		aTxBuffer[i] = 0;
	for(i=0;i<36;i++)
		bTxBuffer[i] = 0;

	aTxBuffer[0] = ENABLE_EXTENDED_COMMANDS;
	aTxBuffer[1] = 0xFF;
	aTxBuffer[2] = 0x83;
	aTxBuffer[3] = 0x63;
	length = convert8to9to8((uint8_t*)aTxBuffer,(uint8_t*)bTxBuffer,4);
	send((uint8_t*)bTxBuffer, length);

	aTxBuffer[0] = SET_POWER;
	aTxBuffer[1] = 0x81;
	aTxBuffer[2] = 0x24;
	aTxBuffer[3] = 0x04;
	aTxBuffer[4] = 0x02;
	aTxBuffer[5] = 0x02;
	aTxBuffer[6] = 0x03;
	aTxBuffer[7] = 0x10;
	aTxBuffer[8] = 0x10;
	aTxBuffer[9] = 0x34;
	aTxBuffer[10] = 0x3C;
	aTxBuffer[11] = 0x3F;
	aTxBuffer[12] = 0x3F;
	length = convert8to9to8((uint8_t*)aTxBuffer,(uint8_t*)bTxBuffer,13);
	send((uint8_t*)bTxBuffer, length);

	aTxBuffer[0] = SLEEP_OUT;
	length = convert8to9to8((uint8_t*)aTxBuffer,(uint8_t*)bTxBuffer,1);
	send((uint8_t*)bTxBuffer, length);
	HAL_Delay(5+1);

	aTxBuffer[0] = DISPLAY_INVERSION_OFF;
	length = convert8to9to8((uint8_t*)aTxBuffer,(uint8_t*)bTxBuffer,1);
	send((uint8_t*)bTxBuffer, length);

	aTxBuffer[0] = MEMORY_ACCESS_ONTROL;
	aTxBuffer[1] = 0x00;
	length = convert8to9to8((uint8_t*)aTxBuffer,(uint8_t*)bTxBuffer,2);
	send((uint8_t*)bTxBuffer, length);

	aTxBuffer[0] = INTERFACE_PIXEL_FORMAT;
	aTxBuffer[1] = 0x70;
	length = convert8to9to8((uint8_t*)aTxBuffer,(uint8_t*)bTxBuffer,2);
	send((uint8_t*)bTxBuffer, length);
	HAL_Delay(120+20);

	aTxBuffer[0] = SET_POWER;
	aTxBuffer[1] = 0x78;
	aTxBuffer[2] = 0x24;
	aTxBuffer[3] = 0x04,
	aTxBuffer[4] = 0x02;
	aTxBuffer[5] = 0x02;
	aTxBuffer[6] = 0x03;
	aTxBuffer[7] = 0x10;
	aTxBuffer[8] = 0x10;
	aTxBuffer[9] = 0x34;
	aTxBuffer[10] = 0x3C;
	aTxBuffer[11] = 0x3F;
	aTxBuffer[12] = 0x3F;
	length = convert8to9to8((uint8_t*)aTxBuffer,(uint8_t*)bTxBuffer,13);
	send((uint8_t*)bTxBuffer, length);

	aTxBuffer[0] = SET_RGB_INTERFACE_RELATED;
	aTxBuffer[1] = 0x01;
	length = convert8to9to8((uint8_t*)aTxBuffer,(uint8_t*)bTxBuffer,2);
	send((uint8_t*)bTxBuffer, length);

	aTxBuffer[0] = SET_DISPLAY_WAVEFORM;
	aTxBuffer[1] = 0x00;
	aTxBuffer[2] = 0x08;
	aTxBuffer[3] = 0x56;
	aTxBuffer[4] = 0x07;
	aTxBuffer[5] = 0x01;
	aTxBuffer[6] = 0x01;
	aTxBuffer[7] = 0x4D;
	aTxBuffer[8] = 0x01;
	aTxBuffer[9] = 0x42;
	length = convert8to9to8((uint8_t*)aTxBuffer,(uint8_t*)bTxBuffer,10);
	send((uint8_t*)bTxBuffer, length);

	aTxBuffer[0] = SET_PANEL;
	aTxBuffer[1] = 0x0B;
	length = convert8to9to8((uint8_t*)aTxBuffer,(uint8_t*)bTxBuffer,2);
	send((uint8_t*)bTxBuffer, length);

	aTxBuffer[0] = SET_GAMMA_CURVE_RELATED;
	aTxBuffer[1] = 0x01;
	aTxBuffer[2] = 0x48;
	aTxBuffer[3] = 0x4D;
	aTxBuffer[4] = 0x4E;
	aTxBuffer[5] = 0x58;
	aTxBuffer[6] = 0xF6;
	aTxBuffer[7] = 0x0B;
	aTxBuffer[8] = 0x4E;
	aTxBuffer[9] = 0x12;
	aTxBuffer[10] = 0xD5;
	aTxBuffer[11] = 0x15;
	aTxBuffer[12] = 0x95;
	aTxBuffer[13] = 0x55;
	aTxBuffer[14] = 0x8E;
	aTxBuffer[15] = 0x11;
	aTxBuffer[16] = 0x01;
	aTxBuffer[17] = 0x48;
	aTxBuffer[18] = 0x4D;
	aTxBuffer[19] = 0x55;
	aTxBuffer[20] = 0x5F;
	aTxBuffer[21] = 0xFD;
	aTxBuffer[22] = 0x0A;
	aTxBuffer[23] = 0x4E;
	aTxBuffer[24] = 0x51;
	aTxBuffer[25] = 0xD3;
	aTxBuffer[26] = 0x17;
	aTxBuffer[27] = 0x95;
	aTxBuffer[28] = 0x96;
	aTxBuffer[29] = 0x4E;
	aTxBuffer[30] = 0x11;
	length = convert8to9to8((uint8_t*)aTxBuffer,(uint8_t*)bTxBuffer,31);
	send((uint8_t*)bTxBuffer, length);
	HAL_Delay(5+1);

	aTxBuffer[0] = DISPLAY_ON;
	length = convert8to9to8((uint8_t*)aTxBuffer,(uint8_t*)bTxBuffer,1);
	send((uint8_t*)bTxBuffer, length);
}


static void Display_Error_Handler(void)
{
  while(1)
  {
  }
}

/**
  ******************************************************************************
	* @copyright	heinrichs weikamp gmbh
  * @file    		bonexConnect.c
  * @author  		heinrichs weikamp gmbh
	* @date    		29-Sept-2015
  * @version 		0.2
  * @since	 		01-Okt-2015
  * @brief   		connect to bluetooth LTE of BonexInfoSystem
	* @bug
	* @warning
  @verbatim
  ==============================================================================
	
	BLE:
	first of all:
	AT+LEROLE=1 // set BLE role of the device to Central
	it is invisible and does not accept incoming BLE connections
	
	then:
	AT+LESCAN
	AT+LENAME=<bdaddr> request remote device name over BLE
	ATD <brad>,TIO Initiate Bluetooth Link (for outgoing connections)
	

	==============================================================================
  @endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 heinrichs weikamp</center></h2>
  *
  ******************************************************************************
  */

#include "bonexConnect.h"

#ifndef BONEXBLUETOOTH
void bonexControl(void)
{
	return;
}
void bC_setConnectRequest(void)
{
	return;
}
uint8_t bC_getStatus(void)
{
	return BC_DISCONNECTED;
}
uint8_t bC_getName(char *name)
{
	*name = 0;
	return 0;
}
uint8_t bC_getData(float *watt, float *temperature, uint16_t *drehzahl, uint8_t *residualcapacity)
{
	*watt = 0;
	*temperature = 0;
	*drehzahl = 0;
	*residualcapacity = 0;
	
	return BC_DISCONNECTED;
}

#else


/* Includes ------------------------------------------------------------------*/
#include "settings.h"
#include "ostc.h"
#include "string.h"
#include "data_central.h"



	union tempFloat16{
		uint16_t u16;
		uint8_t u8[2];
	} temp;

/* Private function prototypes -----------------------------------------------*/
void bC_connect(void);
uint8_t bC_connect_sub_Search(void);
uint8_t bC_connect_sub_Connect(void);
void bC_call(void);
void bC_evaluateData(void);


/* Private variables with external access ------------------------------------*/
uint8_t status = 0;
uint8_t searchrequest = 0;
char nameOfScooter[20]; 
uint8_t dataBuffer[9];
uint8_t StartListeningToUARTscooter = 1;

float scooterWattstunden = 0;
float scooterTemperature = 0;
uint8_t	scooterRestkapazitaet = 0;
uint16_t	scooterDrehzahl = 0;

/* Exported functions --------------------------------------------------------*/

uint8_t bC_evaluateData(void)
{
	for
	pStateReal->lifeData.wireless_data[i].data[j]

}


uint8_t bC_getData(float *watt, float *temperature, uint16_t *drehzahl, uint8_t *residualcapacity)
{
	if(watt)
		*watt = scooterWattstunden;
	if(temperature)
		*temperature = scooterTemperature;
	if(drehzahl)
		*drehzahl = scooterDrehzahl;
	if(residualcapacity)
		*residualcapacity = scooterRestkapazitaet;
	return status;
}

uint8_t bC_getStatus(void)
{
	return status;
}


void bC_setConnectRequest(void)
{
	searchrequest = 1;
}


uint8_t bC_getName(char *name)
{
	if(status != BC_CONNECTED)
		*name = 0;
	
	strncpy(name,nameOfScooter,20);
	name[19] = 0;
	return strlen(name);
}

/*
void bonexControl(void)
{
	static uint32_t time = 0;

	if(settingsGetPointer()->scooterControl == 0)
	{
		status = BC_DISCONNECTED;
		time = 0;
		return;
	}
	
	if(settingsGetPointer()->bluetoothActive == 0)
	{
		status = BC_DISCONNECTED;
		time = 0;
		return;
	}

	if(searchrequest)
	{
		searchrequest = 0;
		// maybe we have to disconnect first?
		bC_connect();
		StartListeningToUARTscooter = 1;
		time = 0;
		return;
	}

	if(status != BC_CONNECTED)
	{
		time = 0;
		return;
	}

	if(UartReady == SET)
	{
		UartReady = RESET;
		StartListeningToUARTscooter = 1;
		bC_evaluateData();
		return;
	}

	if(time_elapsed_ms(time, HAL_GetTick()) < 1000)
	{
		return;
	}
*/
// test	
/*
const char request[4] = {0xA3, 5, 0, 0xA6};
HAL_UART_Transmit(&UartHandle, (uint8_t*)request, 4, 1000);
time = HAL_GetTick();
return;
*/	
	if((UartReady == RESET) && StartListeningToUARTscooter)
	{
		bC_call();
		time = HAL_GetTick();
	}
}


/* Private functions ---------------------------------------------------------*/

void BONEX_to_16bit(uint16_t *dataOutUint16, int16_t *dataOutInt16, uint8_t *dataIn)
{
	union tempU16{
		int16_t i16;
		uint16_t u16;
		uint8_t u8[2];
	} temp;
	
	temp.u8[0] = dataIn[0];
	temp.u8[1] = dataIn[1];
	
	if(dataOutUint16)
		*dataOutUint16 = temp.u16;
	
	if(dataOutInt16)
		*dataOutInt16 = temp.i16;
}

void bC_evaluateData(void)
{
	uint8_t crc = dataBuffer[0];
	for(int i=1;i<=8;i++)
		crc ^= dataBuffer[i];
	
	if(crc != 0)
		return;

	uint16_t watt;
	int16_t temperatureL;

	BONEX_to_16bit(&watt,0, &dataBuffer[1]);	
	BONEX_to_16bit(0,&temperatureL, &dataBuffer[4]);	
	BONEX_to_16bit(&scooterDrehzahl,0, &dataBuffer[6]);	
	
//	scooterWattstunden = ((float)(dataBuffer[3]))/100;
//	scooterWattstunden += watt;
	scooterWattstunden = watt; // neu ohne milliWattSekunden hw 160113
	scooterRestkapazitaet = dataBuffer[3];
	scooterTemperature = ((float)(temperatureL))/10;

/*
	aTxBuffer[0] = uartSendNext;												// 0							UINT8_T
	BONEX_16to8(&aTxBuffer[1],&WattStunden); 						// 1+2 LSB first	UINT16_T
	alt: aTxBuffer[3] = (uint8_t)(milliWattSekunden/36000);// 3						UINT8_T
	neu: aTxBuffer[3] = (uint8_t)(RestKapazitaet);			// 3							UINT8_T
	BONEX_16to8(&aTxBuffer[4],&TemperaturLStufe);				// 4+5 LSB first 	INT16_T
	BONEX_16to8(&aTxBuffer[6],&DrehzahlNeu); 						// 6+7 LSB first 	UINT16_T
	crc																																		UINT8_T
*/
}

void bC_call(void)
{
	const char request[4] = {0xA3, 1, 0, 0xA2};
	uint8_t answer = BONEX_OK;

	answer = HAL_UART_Transmit(&UartHandle, (uint8_t*)request, 4, 1000);
	if(answer != HAL_OK)
		return;

	StartListeningToUARTscooter = 0;
	answer = HAL_UART_Receive_IT(&UartHandle, dataBuffer, 9);
}


void bC_connect(void)
{
	status = BC_SEARCHING;
	uint8_t answer = BONEX_OK;
	
	answer = BONEX_OK;
	if(settingsGetPointer()->scooterDeviceAddress[0] == 0)
	{
		answer = bC_connect_sub_Search();
	}
	
	if(answer == BONEX_OK)
	{
		answer = bC_connect_sub_Connect();
	}
	
	if(answer == BONEX_OK)
		status = BC_CONNECTED;
	else
		status = BC_DISCONNECTED;
}


uint8_t bC_connect_sub_Search(void)
{
	uint8_t answer = BONEX_OK;
	char buffer[256];
	uint8_t bufferPtr = 0;
	uint8_t length;
	uint32_t time;
	char *startOfBONEXString;
	char *startOfRemoteDeviceAddress;
	uint8_t okayNotSend;

	
	strncpy(buffer,"AT+BINQ\r",256);
	length = 	strlen(buffer);
	answer = HAL_UART_Transmit(&UartHandle, (uint8_t*)buffer, length, 1000);
	time = HAL_GetTick();
	bufferPtr = 0;
	okayNotSend = 6;
	while((time_elapsed_ms(time, HAL_GetTick()) < 20000) && (bufferPtr < 255) && (okayNotSend))
	{
		answer = HAL_UART_Receive(&UartHandle, (uint8_t*)&buffer[bufferPtr], 1, 1000);
		if(answer == HAL_OK)
		{
			switch(okayNotSend)
			{
				case 1:
					if(buffer[bufferPtr] == '\n')
					{
						okayNotSend = 0;
						break;
					}
				case 2:
					if(buffer[bufferPtr] == '\r')
					{
						okayNotSend = 1;
						break;
					}
				case 3:
					if(buffer[bufferPtr] == 'K')
					{
						okayNotSend = 2;
						break;
					}
				case 4:
					if(buffer[bufferPtr] == 'O')
					{
						okayNotSend = 3;
						break;
					}
				case 5:
					if(buffer[bufferPtr] == '\n')
					{
						okayNotSend = 4;
						break;
					}
				case 6:
					if(buffer[bufferPtr] == '\r')
					{
						okayNotSend = 5;
						break;
					}
				default:
					okayNotSend = 6;
					break;
			}
			bufferPtr++;
		}
	}

	buffer[bufferPtr] = 0; // just for safety of search routines
	length = bufferPtr;
	
	if(length < 1)
		return BONEX_NOTFOUND;
	
	startOfBONEXString = strstr(buffer,"BONEX");
	
	if(!startOfBONEXString)
		return BONEX_NOTFOUND;
	
	
	// copy to scooterDeviceAddress
	startOfRemoteDeviceAddress = startOfBONEXString - 1;
	while(startOfRemoteDeviceAddress >= buffer)
	{
		if(*startOfRemoteDeviceAddress == '\r')
			break;
		startOfRemoteDeviceAddress--;
	}
/*
	startOfBONEXString = 0; // for strrchr
	
	startOfRemoteDeviceAddress = strrchr(buffer, '\r');
*/	
	// first in list?
	if(!startOfRemoteDeviceAddress)
		startOfRemoteDeviceAddress = buffer;
	else
	{
		startOfRemoteDeviceAddress += 1;
		if(*startOfRemoteDeviceAddress == '\n')
			startOfRemoteDeviceAddress += 1;
	}
	strncpy(settingsGetPointer()->scooterDeviceAddress, startOfRemoteDeviceAddress, 12);

	for(int i=0;i<19;i++)
	{
		if((startOfBONEXString[i] == 0) || (startOfBONEXString[i] == '\r'))
		{
			settingsGetPointer()->scooterDeviceName[i] = 0;
			break;
		}
		else
			settingsGetPointer()->scooterDeviceName[i] = startOfBONEXString[i];
	}
	return BONEX_OK;
}


uint8_t bC_connect_sub_Connect(void)
{
	uint8_t answer = BONEX_OK;
	char buffer2[256];
	uint8_t bufferPtr = 0;
	uint8_t length;
	uint32_t time;

	strncpy(buffer2,"ATD ",256);
	length = strlen(buffer2);
	strncpy(&buffer2[length], settingsGetPointer()->scooterDeviceAddress, 12);
	length += 12;
	buffer2[length++] = '\r';
	buffer2[length++] = 0;
	answer = HAL_UART_Transmit(&UartHandle, (uint8_t*)buffer2, length, 1000);

	time = HAL_GetTick();
	bufferPtr = 0;
	while((time_elapsed_ms(time, HAL_GetTick()) < 5000) && (bufferPtr < 255))
	{
		answer = HAL_UART_Receive(&UartHandle, (uint8_t*)&buffer2[bufferPtr], 1, 1000);
		if(answer == HAL_OK)
			bufferPtr++;
	}

	if(bufferPtr < 7)
		return BONEX_NOTFOUND;

	if(strstr(buffer2,"CONNECT"))
	{
		strncpy(nameOfScooter, settingsGetPointer()->scooterDeviceName, 19);
		nameOfScooter[19] = 0;
		return BONEX_OK;
	}
	else
		return BONEX_NOCONNECT;
}

#endif // BONEXBLUETOOTH


/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/

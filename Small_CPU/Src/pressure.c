/**
  ******************************************************************************
  * @file    pressure.c 
  * @author  heinrichs weikamp gmbh
  * @date    2014
  * @version V0.0.2
  * @since   20-Oct-2016
  * @brief   
  *           
  @verbatim                 
  ============================================================================== 
                        ##### How to use #####
  ============================================================================== 
	V0.0.2		18-Oct-2016		pressure_calculation_AN520_004_mod_MS5803_30BA__09_2015
	
	@endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 heinrichs weikamp</center></h2>
  *
  ******************************************************************************
  */ 



/* surface time
 the last 30 minutes will be saved once per minute in a endless loop
 at the beginning of a dive the oldest value will be used
*/


#include "pressure.h"
#include "i2c.h"
#include "rtc.h"

#define CMD_RESET 0x1E // ADC reset command
#define CMD_ADC_READ 0x00 // ADC read command
#define CMD_ADC_CONV 0x40 // ADC conversion command
#define CMD_ADC_D1 0x00 // ADC D1 conversion
#define CMD_ADC_D2 0x10 // ADC D2 conversion
#define CMD_ADC_256 0x00 // ADC OSR=256
#define CMD_ADC_512 0x02 // ADC OSR=512
#define CMD_ADC_1024 0x04 // ADC OSR=1024
#define CMD_ADC_2048 0x06 // ADC OSR=2056
#define CMD_ADC_4096 0x08 // ADC OSR=4096
#define CMD_PROM_RD 0xA0 // Prom read command


//uint16_t  get_ci(uint8_t cmd);
//uint8_t  get_ci_crc(void);
uint16_t  get_ci_by_coef_num(uint8_t coef_num);
void pressure_calculation_new(void);
void pressure_calculation_old(void);
void pressure_calculation_AN520_004_mod_MS5803_30BA__09_2015(void);

uint8_t crc4(uint16_t n_prom[]);

HAL_StatusTypeDef pressure_sensor_get_data(void);
uint32_t get_adc(void);
uint8_t pressureSensorInitSuccess = 0;

//void test_calculation(void);

uint16_t C[8] = { 1 };
uint32_t D1 = 1;
uint32_t D2 = 1;
uint8_t n_crc;

int64_t C5_x_2p8 = 1;
int64_t C2_x_2p16 = 1;
int64_t C1_x_2p15 = 1;

/*
short C2plus10000 = -1;
short C3plus200 = -1;
short C4minus250 = -1;
short UT1 = -1;
short C6plus100 = -1;
*/

float ambient_temperature = 0;
float ambient_pressure_mbar = 0;
float surface_pressure_mbar = 1000;
float surface_ring_mbar[31] = { 0 };

uint8_t secondCounterSurfaceRing = 0;

float get_temperature(void)
{
	return ambient_temperature;
}

//float test = 1000;

float get_pressure_mbar(void)
{
//	return test;
	return ambient_pressure_mbar;
}


float get_surface_mbar(void)
{
	return surface_pressure_mbar;
}


void init_surface_ring(void)
{
	surface_ring_mbar[0] = 0;
	for(int i=1; i<31; i++)
		surface_ring_mbar[i] = ambient_pressure_mbar;
	surface_pressure_mbar = ambient_pressure_mbar;
}


/* the ring has one place with 0
 * after that comes the oldest value
 * the new pressure is written in this hole
 * the oldest value is read and then the new hole
*/
void update_surface_pressure(uint8_t call_rhythm_seconds)
{
	secondCounterSurfaceRing += call_rhythm_seconds;
	
	if(secondCounterSurfaceRing < 60)
		return;
	
	secondCounterSurfaceRing = 0;
	
	int hole;
	for(hole=30;hole>0;hole--)
		if(surface_ring_mbar[hole] == 0) { break; }

	surface_ring_mbar[hole] = ambient_pressure_mbar;

	hole++;
	if(hole > 30)
		hole = 0;
	surface_pressure_mbar = surface_ring_mbar[hole];
	surface_ring_mbar[hole] = 0;
}


float demo_modify_temperature_helper(float bottom_mbar_diff_to_surface)
{
	const float temperature_surface = 31.0;
	const float temperature_bottom = 14.0;

	const float temperature_difference = temperature_bottom - temperature_surface;
	
	// range 0.0 - 1.0
	float position_now = (ambient_pressure_mbar - surface_pressure_mbar) / bottom_mbar_diff_to_surface; 

	if(position_now <= 0)
		return temperature_surface;
	
	if(position_now >= 1)
		return temperature_bottom;

	return temperature_surface + (temperature_difference * position_now);
}


uint32_t demo_modify_temperature_and_pressure(int32_t divetime_in_seconds, uint8_t subseconds, float ceiling_mbar)
{
	
	const float descent_rate = 4000/60;
	const float ascent_rate = 1000/60;
	const uint32_t seconds_descend = (1 * 60) + 30;
	const uint32_t turbo_seconds_at_bottom_start = (0 * 60) + 0;
	const uint32_t seconds_descend_and_bottomtime = seconds_descend + turbo_seconds_at_bottom_start + (2 * 60) + 0;
	uint32_t time_elapsed_in_seconds;
	static float ambient_pressure_mbar_memory = 0;
	static uint32_t time_last_call = 0;
	
	if(divetime_in_seconds <= seconds_descend)
	{
		ambient_pressure_mbar = (divetime_in_seconds * descent_rate) + ((float)(subseconds) * descent_rate) + surface_pressure_mbar;
		ambient_temperature = demo_modify_temperature_helper(descent_rate * seconds_descend);

		time_last_call = divetime_in_seconds;
		return 0;
	}
	else
	if(divetime_in_seconds <= seconds_descend + turbo_seconds_at_bottom_start)
	{
		ambient_pressure_mbar = (seconds_descend * descent_rate) + surface_pressure_mbar;
		ambient_temperature = demo_modify_temperature_helper(descent_rate * seconds_descend);
		ambient_pressure_mbar_memory = ambient_pressure_mbar;
		time_last_call = divetime_in_seconds;
		return turbo_seconds_at_bottom_start;
	}
	else
	if(divetime_in_seconds <= seconds_descend_and_bottomtime)
	{
		ambient_pressure_mbar = (seconds_descend * descent_rate) + surface_pressure_mbar;
		ambient_temperature = demo_modify_temperature_helper(descent_rate * seconds_descend);
		ambient_pressure_mbar_memory = ambient_pressure_mbar;
		time_last_call = divetime_in_seconds;
		return 0;
	}
	else
	{
		time_elapsed_in_seconds = divetime_in_seconds - time_last_call;
		ambient_pressure_mbar = ambient_pressure_mbar_memory - time_elapsed_in_seconds * ascent_rate;

		if(ambient_pressure_mbar < surface_pressure_mbar)
			ambient_pressure_mbar = surface_pressure_mbar;
		else if(ambient_pressure_mbar < ceiling_mbar)
			ambient_pressure_mbar = ceiling_mbar;
		
		ambient_temperature = demo_modify_temperature_helper(descent_rate * seconds_descend);
		ambient_pressure_mbar_memory = ambient_pressure_mbar;
		time_last_call = divetime_in_seconds;
		return 0;
	}
}


/* called just once on power on */
void init_pressure_DRx(void)
{
	uint8_t resetCommand[1] = {0x1E};

	I2C_Master_Transmit(  DEVICE_PRESSURE, resetCommand, 1);
	HAL_Delay(3);

	C[1] =	get_ci_by_coef_num(0x02);
	C[2] =	get_ci_by_coef_num(0x04);
	C[3] =	get_ci_by_coef_num(0x06);
	C[4] =	get_ci_by_coef_num(0x08);
	C[5] =	get_ci_by_coef_num(0x0A);
	C[6] =	get_ci_by_coef_num(0x0C);
	
	C5_x_2p8  = C[5] * 256;
	C2_x_2p16 = C[2] * 65536;
	C1_x_2p15 = C[1] * 32768;
	pressure_update();
}

uint8_t is_init_pressure_done(void)
{
	return pressureSensorInitSuccess;
}

uint8_t init_pressure(void)
{
	uint8_t buffer[1];
	buffer[0] = 0x1e;
	uint8_t retValue = 0xFF;
	
	
	retValue = I2C_Master_Transmit(  DEVICE_PRESSURE, buffer, 1);
	if(retValue != HAL_OK)
	{
		return (HAL_StatusTypeDef)retValue;
	}
	HAL_Delay(3);
	
	for(uint8_t i=0;i<8;i++)
	{
		C[i] = get_ci_by_coef_num(i);
	}
	n_crc = crc4(C); // no evaluation at the moment hw 151026

	C5_x_2p8  = C[5] * 256;
	C2_x_2p16 = C[2] * 65536;
	C1_x_2p15 = C[1] * 32768;
	
	if(I2C1_Status() == HAL_OK)
	{
		pressureSensorInitSuccess = 1;
	}
	return pressure_update();
}


uint32_t get_adc(void)
{
	uint8_t buffer[1];
	uint8_t resivebuf[4];
	uint32_t answer = 0;
	//
	buffer[0] = 0x00; // Get ADC
	I2C_Master_Transmit( DEVICE_PRESSURE, buffer, 1);
	I2C_Master_Receive(  DEVICE_PRESSURE, resivebuf, 4);
	resivebuf[3] = 0;
	answer = 256*256 *(uint32_t)resivebuf[0]  + 256 * (uint32_t)resivebuf[1] + (uint32_t)resivebuf[2];

	return answer;
}


uint16_t  get_ci_by_coef_num(uint8_t coef_num)
{
	uint8_t resivebuf[2];

	uint8_t cmd = CMD_PROM_RD+coef_num*2; 
	I2C_Master_Transmit( DEVICE_PRESSURE, &cmd, 1);
	I2C_Master_Receive(  DEVICE_PRESSURE, resivebuf, 2);
	return (256*(uint16_t)resivebuf[0]) + (uint16_t)resivebuf[1];
}



uint8_t pressure_update(void)
{
	HAL_StatusTypeDef statusReturn = HAL_TIMEOUT;
	
	statusReturn = pressure_sensor_get_data();
	pressure_calculation();
	return (uint8_t)statusReturn;
}


uint32_t pressure_sensor_get_one_value(uint8_t cmd, HAL_StatusTypeDef *statusReturn)
{
	uint8_t command = CMD_ADC_CONV + cmd;
	HAL_StatusTypeDef statusReturnTemp = HAL_TIMEOUT;
	
	statusReturnTemp = I2C_Master_Transmit( DEVICE_PRESSURE, &command, 1);

	if(statusReturn)
	{
		*statusReturn = statusReturnTemp;
	}
	
	switch (cmd & 0x0f) // wait necessary conversion time
	{
	case CMD_ADC_256 : HAL_Delay(1); break;
	case CMD_ADC_512 : HAL_Delay(3); break;
	case CMD_ADC_1024: HAL_Delay(4); break;
	case CMD_ADC_2048: HAL_Delay(6); break;
	case CMD_ADC_4096: HAL_Delay(10); break;
	}	
	return get_adc();
}


HAL_StatusTypeDef pressure_sensor_get_data(void)
{
	HAL_StatusTypeDef statusReturn1 = HAL_TIMEOUT;
	HAL_StatusTypeDef statusReturn2 = HAL_TIMEOUT;
	
	D2 = pressure_sensor_get_one_value(CMD_ADC_D2 + CMD_ADC_4096, &statusReturn1);
	D1 = pressure_sensor_get_one_value(CMD_ADC_D1 + CMD_ADC_4096, &statusReturn2);

	if(statusReturn2 > statusReturn1) // if anything is not HAL_OK (0x00) or worse
		return statusReturn2;
	else
		return statusReturn1;
}


void  pressure_sensor_get_pressure_raw(void)
{
	D1 = pressure_sensor_get_one_value(CMD_ADC_D1 + CMD_ADC_4096, 0);
}


void  pressure_sensor_get_temperature_raw(void)
{
	D2 = pressure_sensor_get_one_value(CMD_ADC_D2 + CMD_ADC_4096, 0);
}


void pressure_calculation(void)
{
	if(I2C1_Status() != HAL_OK)
		return;
	
	pressure_calculation_AN520_004_mod_MS5803_30BA__09_2015();
	return;

	// before October 2016:	pressure_calculation_old();
	
//	pressure_calculation_new();
}

void pressure_calculation_AN520_004_mod_MS5803_30BA__09_2015(void)
{
	uint32_t local_D1; // ADC value of the pressure conversion
	uint32_t local_D2; // ADC value of the temperature conversion
	int32_t local_Px10; // compensated pressure value
	int32_t local_Tx100; // compensated temperature value
	int64_t local_dT; // int32_t, difference between actual and measured temperature
	int64_t local_OFF; // offset at actual temperature
	int64_t local_SENS; // sensitivity at actual temperature

	int64_t T2;
	int64_t OFF2;
	int64_t SENS2;

	local_D1 = D1;
	local_D2 = D2;

	local_dT 		= ((int64_t)local_D2) - ((int64_t)C[5]) * 256; //pow(2,8);
	local_OFF 	= ((int64_t)C[2]) * 65536 + local_dT * ((int64_t)C[4]) / 128; // pow(2,16), pow(2,7)
	local_SENS 	= ((int64_t)C[1]) * 32768 + local_dT * ((int64_t)C[3]) / 256; // pow(2,15), pow(2,8)

	local_Tx100 = (int32_t)(2000 + (local_dT * ((int64_t)C[6])) / 8388608);// pow(2,23)


	if(local_Tx100 < 2000) // low temperature
	{
		T2  = 3 * local_dT;
		T2 *= local_dT;
		T2 /= 8589934592;

		OFF2 = ((int64_t)local_Tx100) - 2000;
		OFF2 *= OFF2;
		OFF2 *= 3;
		OFF2 /= 2;
		
		SENS2 = ((int64_t)local_Tx100) - 2000;
		SENS2 *= SENS2;
		SENS2 *= 5;
		SENS2 /= 8;

		local_Tx100 -= (int32_t)T2;
		local_OFF 	-= OFF2;
		local_SENS 	-= SENS2;
	}
	else
	{
		T2  = 7 * local_dT;
		T2 *= local_dT;
		T2 /= 137438953472;

		OFF2 = ((int64_t)local_Tx100) - 2000;
		OFF2 *= OFF2;
		OFF2 /= 16;
		
		local_Tx100 -= (int32_t)T2;
		local_OFF 	-= OFF2;
	}
	
	local_Px10 = (int32_t)(
							(((int64_t)((local_D1 * local_SENS) / 2097152)) - local_OFF)
								/  8192 );//     )) / 10; // pow(2,21), pow(2,13)

	ambient_temperature 	= ((float)local_Tx100) / 100;
	ambient_pressure_mbar	= ((float)local_Px10) / 10;
}


void pressure_calculation_new(void)
{
#define POW2_8	(256)
#define POW2_17	(131072)
#define POW2_6	(64)
#define POW2_16	(65536)
#define POW2_7	(128)
#define POW2_23	(8388608)
#define POW2_21	(2097152)
#define POW2_15	(32768)
#define POW2_13	(8192)
#define POW2_37	(137438953472)
#define POW2_4	(16)
#define POW2_33	(8589934592)
#define POW2_3	(8)
	
	int32_t	P; // compensated pressure value
	int32_t T; // compensated temperature value
	int32_t dT; // difference between actual and measured temperature
	int64_t OFF; // offset at actual temperature
	int64_t SENS;

	int32_t T2;
	int64_t OFF2;
	int64_t SENS2;
	
	dT 		= ((int32_t)D2) - ((int32_t)C[5]) * POW2_8;
	OFF 	= ((int64_t)C[2]) * POW2_16 + ((int64_t)dT) * ((int64_t)C[4]) / POW2_7;
	SENS 	= ((int64_t)C[1]) * POW2_15 + ((int64_t)dT) * ((int64_t)C[3]) / POW2_8;

	T	= 2000 + (dT * ((int32_t)C[6])) / POW2_23;


if(T < 2000) // low temperature
	{
		T2 = 3 * dT * dT;
		T2 /= POW2_33;
		OFF2 = ((int64_t)T) - 2000;
		OFF2 *= OFF2;
		OFF2 *= 3;
		OFF2 /= 2;
		SENS2 = ((int64_t)T) - 2000;
		SENS2 *= SENS2;
		SENS2 *= 5;
		SENS2 /= POW2_3;
	}
	else // high temperature
	{
		T2 = 7 * dT * dT;
		T2 /= POW2_37;
		OFF2 = ((int64_t)T) - 2000;
		OFF2 *= OFF2;
		OFF2 /= POW2_4;
		SENS2 = 0;
	}

	T = T - T2;
	OFF = OFF - OFF2;
	SENS = SENS - SENS2;
	
	P = (int32_t)(((((int64_t)D1) * SENS) / POW2_21 - OFF) / POW2_13);
	
	ambient_temperature 	= ((float)T) / 100;
	ambient_pressure_mbar	= ((float)P) / 10;
}


void pressure_calculation_old(void) {
	//
	double ambient_temperature_centigrad = 0;
	double ambient_pressure_decimbar = 0;
	
	// static for debug
	static int64_t dt = 0;
	static int64_t temp = 0;
	static int64_t ms_off = 0;
	static int64_t sens = 0;
	//
	static int64_t ms_off2 = 0;
	static int64_t sens2 = 0;
	static int64_t t2 = 0;

/* info
uint16_t C[8] = { 1 };
uint32_t D1 = 1;
uint32_t D2 = 1;
uint8_t n_crc;
*/
	if((D2 == 0) || (D1 == 0))
		return;
	//

	// dT = D2 - C[5] * POW2_8;
	// T	= 2000 + (dT * C[6]) / POW2_23;
	dt = (int64_t)D2 - C5_x_2p8;
	//temp ; // in 10 milliGrad Celcius
	ambient_temperature_centigrad = 2000 + dt * C[6] / 8388608;
	

	if(ambient_temperature_centigrad < 2000) // low temperature
		{
			t2 = 3 * dt;
			t2 *= dt;
			t2 /= 8589934592;
			ms_off2 = ambient_temperature_centigrad - 2000;
			ms_off2 *= ms_off2;
			sens2 = ms_off2;
			ms_off2 *= 3;
			ms_off2 /= 2;
			sens2 *= 5;
			sens2 /= 8;
		}
		else // high temperature
		{
			t2 = 7 * dt;
			t2 *= dt;
			t2 /= 137438953472;
			ms_off2 = ambient_temperature_centigrad - 2000;
			ms_off2 *= ms_off2;
			ms_off2 /= 16;
			sens2 = 0;
		}
	
	
	//

	// pressure
	// OFF 	= C[2] * POW2_16 + dT * C[4] / POW2_7;
	// SENS = C[1] * POW2_15 + dT * C[3] / POW2_8;
	ms_off =  C[4] * dt;
	ms_off /= 128;
	ms_off += C2_x_2p16;
	//
	sens =  C[3] * dt;
	sens /= 256;
	sens += C1_x_2p15;

	// 2nd order correction
	ambient_temperature_centigrad -= t2;
	ms_off -= ms_off2;
	sens -= sens2;

	ambient_temperature = ambient_temperature_centigrad / 100;
		// P = (D1 * SENS / POW2_21 - OFF) / POW2_13;
	temp = D1 * sens;
	temp /= 2097152;
	temp -= ms_off;
	temp /= 8192;
	ambient_pressure_decimbar = temp; // to float/double
	ambient_pressure_mbar = ambient_pressure_decimbar / 10;
}


/* taken from AN520 by meas-spec.com dated 9. Aug. 2011
 * short and int are both 16bit according to AVR/GCC google results
 */
uint8_t crc4(uint16_t n_prom[])
{
uint16_t cnt; // simple counter
uint16_t n_rem; // crc reminder
uint16_t crc_read; // original value of the crc
uint8_t n_bit;
n_rem = 0x00;
crc_read=n_prom[7]; //save read CRC
n_prom[7]=(0xFF00 & (n_prom[7])); //CRC byte is replaced by 0
for (cnt = 0; cnt < 16; cnt++) // operation is performed on bytes
{ // choose LSB or MSB
if (cnt%2==1) n_rem ^= (uint16_t) ((n_prom[cnt>>1]) & 0x00FF);
else n_rem ^= (uint16_t) (n_prom[cnt>>1]>>8);
for (n_bit = 8; n_bit > 0; n_bit--)
{
if (n_rem & (0x8000))
{
n_rem = (n_rem << 1) ^ 0x3000;
}
else
{
n_rem = (n_rem << 1);
}
}
}
n_rem= (0x000F & (n_rem >> 12)); // // final 4-bit reminder is CRC code
n_prom[7]=crc_read; // restore the crc_read to its original place
return (n_rem ^ 0x00);
}
/*
void test_calculation(void)
{
	C1 = 29112;
	C2 = 26814;
	C3 = 19125;
	C4 = 17865;
	C5 = 32057;
	C6 = 31305;
	
	C2_x_2p16 = C2 * 65536;
	C1_x_2p15 = C1 * 32768;
	
	D1 = 4944364;
	D2 = 8198974;	
		pressure_calculation() ;
};
*/


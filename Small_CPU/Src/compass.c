/**
  ******************************************************************************
  * @file    compass.c 
  * @author  heinrichs weikamp gmbh
  * @date    27-March-2014
  * @version V0.2.0
  * @since   21-April-2016
  * @brief   for Honeywell Compass and ST LSM303D
  *           
  @verbatim                 
  ============================================================================== 
                        ##### How to use #####
  ============================================================================== 
	V0.1.0		09-March-2016
	V0.2.0		21-April-2016	Orientation fixed for LSM303D,
													roll and pitch added to calibration output,
													orientation double checked with datasheets and layout
													as well as with value output during calibration
	V0.2.1		19-May-2016		New date rate config and full-scale selection
	
	@endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 heinrichs weikamp</center></h2>
  *
  ******************************************************************************
  */ 

#include <math.h>
#include <string.h>

#include "compass.h"
#include "compass_LSM303D.h"
#include "compass_LSM303DLHC.h"

#include "i2c.h"
#include "RTE_FlashAccess.h" // to store compass_calib_data

#include "stm32f4xx_hal.h"

#define MODE_LSM303DLHC
#define	TEST_IF_HMC5883L
//#define COMPASS_DEACTIVATE

/// split byte to bits
typedef struct{ 
uint8_t bit0:1; ///< split byte to bits
uint8_t bit1:1; ///< split byte to bits
uint8_t bit2:1; ///< split byte to bits
uint8_t bit3:1; ///< split byte to bits
uint8_t bit4:1; ///< split byte to bits
uint8_t bit5:1; ///< split byte to bits
uint8_t bit6:1; ///< split byte to bits
uint8_t bit7:1; ///< split byte to bits
} ubit8_t;


/// split byte to bits
typedef union{ 
ubit8_t ub; ///< split byte to bits
uint8_t uw; ///< split byte to bits
} bit8_Type;
	

/// split word to 2 bytes
typedef struct{
uint8_t low; ///< split word to 2 bytes
uint8_t hi; ///< split word to 2 bytes
} two_byte; 
	

/// split word to 2 bytes
typedef union{
two_byte Byte; ///< split word to 2 bytes
uint16_t Word; ///< split word to 2 bytes
} tword; 


/// split signed word to 2 bytes
typedef union{
two_byte Byte; ///< split signed word to 2 bytes
int16_t Word; ///< split signed word to 2 bytes
} signed_tword; 


/// split full32 to 2 words
typedef struct{
uint16_t low16; ///< split word to 2 bytes
uint16_t hi16; ///< split word to 2 bytes
} two_word; 

typedef union{
two_word Word16; ///< split word to 2 bytes
uint32_t Full32; ///< split word to 2 bytes
} tfull32; 


/// crazy compass calibration stuff
typedef struct
{
unsigned short int compass_N;							
float Su, Sv, Sw;													
float Suu, Svv, Sww, Suv, Suw, Svw;				
float Suuu, Svvv, Swww;										
float Suuv, Suuw, Svvu, Svvw, Swwu, Swwv;	
} SCompassCalib; 


#define Q_PI    (18000)
#define Q_PIO2  (9000)

#define HMC5883L (1)	///< id used with hardwareCompass 
#define LSM303D  (2)	///< id used with hardwareCompass
#define LSM303DLHC  (3)	///< id used with hardwareCompass
#define COMPASS_NOT_RECOGNIZED  (4)	///< id used with hardwareCompass


//////////////////////////////////////////////////////////////////////////////
// fifth order of polynomial approximation of atan(), giving 0.05 deg max error
//
#define K1  (5701)  // Needs K1/2**16
#define K2  (1645)  // Needs K2/2**48  WAS NEGATIV
#define K3  ( 446)  // Needs K3/2**80

const float PI = 3.14159265;	///< pi, used in compass_calc()

typedef short int Int16;
typedef signed char Int8;
typedef Int16 Angle;


/// The (filtered) components of the magnetometer sensor
int16_t compass_DX_f; ///< output from sensor
int16_t compass_DY_f; ///< output from sensor
int16_t compass_DZ_f; ///< output from sensor


/// Found soft-iron calibration values, deduced from already filtered values
int16_t compass_CX_f; ///< calibration value
int16_t compass_CY_f; ///< calibration value
int16_t compass_CZ_f; ///< calibration value


/// The (filtered) components of the accelerometer sensor
int16_t accel_DX_f; ///< output from sensor
int16_t accel_DY_f; ///< output from sensor
int16_t accel_DZ_f; ///< output from sensor


/// The compass result values
float compass_heading;	///< the final result calculated in compass_calc()
float compass_roll;			///< the final result calculated in compass_calc()
float compass_pitch;		///< the final result calculated in compass_calc()


uint8_t compass_gain; ///< 7 on start, can be reduced during calibration

uint8_t  hardwareCompass = 0;	///< either  HMC5883L or LSM303D or not defined yet ( = 0 )

/// LSM303D variables
uint8_t magDataBuffer[6];	///< here raw data from LSM303D is stored, can be local
uint8_t accDataBuffer[6];	///< here raw data from LSM303D is stored, can be local

//uint16_t velMag = 0;
//uint16_t velAcc = 0;

//uint16_t magODR[] = {31,62,125,250,500,1000,2000};
//uint16_t accODR[] = {0,31,62,125,250,500,1000,2000,4000,8000,16000};
//uint8_t fastest = 10; //no sensor is the fastest
//uint8_t datas1 = 0;
//uint8_t zoffFlag = 0;
//uint8_t sendFlag = 0;


// all by pixhawk code:

//	struct accel_scale	_accel_scale;
unsigned		_accel_range_m_s2;
float			_accel_range_scale;
unsigned		_accel_samplerate;
unsigned		_accel_onchip_filter_bandwith;

//	struct mag_scale	_mag_scale;
unsigned		_mag_range_ga;
float			_mag_range_scale;
unsigned		_mag_samplerate;

// default scale factors
float _accel_scale_x_offset = 0.0f;
float _accel_scale_x_scale  = 1.0f;
float _accel_scale_y_offset = 0.0f;
float _accel_scale_y_scale  = 1.0f;
float _accel_scale_z_offset = 0.0f;
float _accel_scale_z_scale  = 1.0f;

float _mag_scale_x_offset = 0.0f;
float _mag_scale_x_scale = 1.0f;
float _mag_scale_y_offset = 0.0f;
float _mag_scale_y_scale = 1.0f;
float _mag_scale_z_offset = 0.0f;
float _mag_scale_z_scale = 1.0f;


/* External function prototypes ----------------------------------------------*/

extern void copyCompassDataDuringCalibration(int16_t dx, int16_t dy, int16_t dz);

/* Private function prototypes -----------------------------------------------*/

void compass_reset_calibration(SCompassCalib *g);
void compass_add_calibration(SCompassCalib *g);
void compass_solve_calibration(SCompassCalib *g);

void compass_init_HMC5883L(uint8_t fast, uint8_t gain);
void compass_sleep_HMC5883L(void);
void compass_read_HMC5883L(void);

void accelerator_init_MMA8452Q(void);
void accelerator_sleep_MMA8452Q(void);
void acceleration_read_MMA8452Q(void);

void compass_init_LSM303D(uint8_t fast, uint8_t gain);
void compass_sleep_LSM303D(void);
void compass_read_LSM303D(void);
void acceleration_read_LSM303D(void);

void compass_init_LSM303DLHC(uint8_t fast, uint8_t gain);
void compass_sleep_LSM303DLHC(void);
void compass_read_LSM303DLHC(void);
void acceleration_read_LSM303DLHC(void);


int LSM303D_accel_set_onchip_lowpass_filter_bandwidth(unsigned bandwidth);
int compass_calib_common(void);

void compass_calc_roll_pitch_only(void);

void rotate_mag_3f(float *x, float *y, float *z);
void rotate_accel_3f(float *x, float *y, float *z);


/* Exported functions --------------------------------------------------------*/


//  ===============================================================================
//	compass_init
/// @brief	This might be called several times with different gain values during calibration
/// 				On first call it figures out which hardware is integrated
///
/// @param 	gain: 7 is max gain, compass_calib() might reduce it
//  ===============================================================================

uint8_t testCompassTypeDebug = 0xFF;

void compass_init(uint8_t fast, uint8_t gain)
{
// quick off
#ifdef COMPASS_DEACTIVATE
	hardwareCompass = COMPASS_NOT_RECOGNIZED;
#endif

// don't call again with fast, gain in calib mode etc.
// if unknown
	if(hardwareCompass == COMPASS_NOT_RECOGNIZED)
	{
		return;
	}
	
// old code but without else
	if(hardwareCompass == 0)
	{
		uint8_t data = WHO_AM_I;
		I2C_Master_Transmit( DEVICE_COMPASS_303D, &data, 1);
		I2C_Master_Receive(  DEVICE_COMPASS_303D, &data, 1);
		if(data == WHOIAM_VALUE)
			hardwareCompass = LSM303D;
		else
			hardwareCompass = HMC5883L;
	}

	
// k�nnte Probleme mit altem Chip machen
// beim 303D f�hrt dieser Code dazu, dass WHOIAM_VALUE nicht geschickt wird!!!
	
#ifdef MODE_LSM303DLHC
	HAL_StatusTypeDef resultOfOperation = HAL_TIMEOUT;

	if(hardwareCompass == 0)
	{
		uint8_t data = DLHC_CTRL_REG1_A;
		resultOfOperation = I2C_Master_Transmit( DEVICE_ACCELARATOR_303DLHC, &data, 1);
		if(resultOfOperation == HAL_OK)
		{
			I2C_Master_Receive(  DEVICE_ACCELARATOR_303DLHC, &data, 1);
			testCompassTypeDebug = data;
			if((data & 0x0f) == 0x07)
			{
				hardwareCompass = LSM303DLHC;
			}
		}
		else
		{
			testCompassTypeDebug = 0xEE;
		}
	}

#endif

	if(hardwareCompass == 0)
	{
		uint8_t data = WHO_AM_I;
		I2C_Master_Transmit( DEVICE_COMPASS_303D, &data, 1);
		I2C_Master_Receive(  DEVICE_COMPASS_303D, &data, 1);
		if(data == WHOIAM_VALUE)
			hardwareCompass = LSM303D;
		else
			hardwareCompass = HMC5883L;
	}

// was in else before !	
	if(hardwareCompass == 0)
		hardwareCompass = HMC5883L;	
	
#ifdef TEST_IF_HMC5883L
	HAL_StatusTypeDef resultOfOperationHMC_MMA = HAL_TIMEOUT;

	if(hardwareCompass == HMC5883L)
	{
		uint8_t data = 0x2A; // CTRL_REG1 of DEVICE_ACCELARATOR_MMA8452Q
		resultOfOperationHMC_MMA = I2C_Master_Transmit( DEVICE_ACCELARATOR_MMA8452Q, &data, 1);
		if(resultOfOperationHMC_MMA == HAL_OK)
		{
			hardwareCompass = HMC5883L; // all fine, keep it
		}
		else
		{
			hardwareCompass = COMPASS_NOT_RECOGNIZED;
			testCompassTypeDebug = 0xEC;
		}
	}
#endif
	
	
	if(hardwareCompass == LSM303DLHC)
	{
		compass_init_LSM303DLHC(fast, gain);
	}
	else
	if(hardwareCompass == LSM303D)
	{
		compass_init_LSM303D(fast, gain);
	}
	else
	if(hardwareCompass == HMC5883L)
	{
		compass_init_HMC5883L(fast, gain);
	}
	
		tfull32 dataBlock[4];
		if(BFA_readLastDataBlock((uint32_t *)dataBlock) == BFA_OK)
		{
			compass_CX_f = dataBlock[0].Word16.low16;
			compass_CY_f = dataBlock[0].Word16.hi16;
			compass_CZ_f = dataBlock[1].Word16.low16;
		}

}


//  ===============================================================================
//	compass_calib
/// @brief with onchip_lowpass_filter configuration for accelerometer of LSM303D
//  ===============================================================================
int compass_calib(void)
{
	if(hardwareCompass == LSM303DLHC)
	{
		return compass_calib_common(); // 170821 zur Zeit kein lowpass filtering gefunden, nur high pass auf dem Register ohne Erkl�rung
	}
	else
	if(hardwareCompass == LSM303D)
	{
		LSM303D_accel_set_onchip_lowpass_filter_bandwidth(773);
		int out = compass_calib_common();
		LSM303D_accel_set_onchip_lowpass_filter_bandwidth(LSM303D_ACCEL_DEFAULT_ONCHIP_FILTER_FREQ);
		return out;
	}
	else
	if(hardwareCompass == HMC5883L)
	{
		return compass_calib_common();
	}
	else
	{
		return 0; // standard answer of compass_calib_common();
	}


}


//  ===============================================================================
//	compass_sleep
/// @brief	low power mode
//  ===============================================================================
void compass_sleep(void)
{
	if(hardwareCompass == LSM303DLHC)
	{
		compass_sleep_LSM303DLHC();
	}
	else
	if(hardwareCompass == LSM303D)
	{
		compass_sleep_LSM303D();
	}
	else
	if(hardwareCompass == HMC5883L)
	{
		compass_sleep_HMC5883L();
	}
}


//  ===============================================================================
//	compass_read
/// @brief	reads magnetometer and accelerometer for LSM303D,
///					otherwise magnetometer only
//  ===============================================================================
void compass_read(void)
{
	if(hardwareCompass == LSM303DLHC)
	{
		compass_read_LSM303DLHC();
	}
	else
	if(hardwareCompass == LSM303D)
	{
		compass_read_LSM303D();
	}
	else
	if(hardwareCompass == HMC5883L)
	{
		compass_read_HMC5883L();
	}
}


//  ===============================================================================
//	accelerator_init
/// @brief	empty for for LSM303D
//  ===============================================================================
void accelerator_init(void)
{
//	if((hardwareCompass != LSM303D) && (hardwareCompass != LSM303DLHC))
	if(hardwareCompass == HMC5883L)
		accelerator_init_MMA8452Q();
}


//  ===============================================================================
//	accelerator_sleep
/// @brief	empty for for LSM303D
//  ===============================================================================
void accelerator_sleep(void)
{
//	if((hardwareCompass != LSM303D) && (hardwareCompass != LSM303DLHC))
	if(hardwareCompass == HMC5883L)
		accelerator_sleep_MMA8452Q();
}


//  ===============================================================================
//	acceleration_read
/// @brief	empty for for LSM303D
//  ===============================================================================
void  acceleration_read(void)
{
	if(hardwareCompass == LSM303DLHC)
	{
		acceleration_read_LSM303DLHC();
	}
	else
	if(hardwareCompass == LSM303D)
	{
		acceleration_read_LSM303D();
	}
	else
	if(hardwareCompass == HMC5883L)
	{
		acceleration_read_MMA8452Q();
	}
}


/* Private functions ---------------------------------------------------------*/

//  ===============================================================================
//	LSM303D_read_reg
/// @brief	tiny helpers by pixhawk
//  ===============================================================================
uint8_t LSM303D_read_reg(uint8_t addr)
{
  uint8_t data;

	I2C_Master_Transmit( DEVICE_COMPASS_303D, &addr, 1);
	I2C_Master_Receive(  DEVICE_COMPASS_303D, &data, 1);
	return data;
}


//  ===============================================================================
//	LSM303D_write_reg
/// @brief	tiny helpers by pixhawk
//  ===============================================================================
void LSM303D_write_reg(uint8_t addr, uint8_t value)
{
  uint8_t data[2];
	
	/* enable accel*/
	data[0] = addr;
	data[1] = value;
	I2C_Master_Transmit( DEVICE_COMPASS_303D, data, 2);
}


//  ===============================================================================
//	LSM303D_write_checked_reg
/// @brief	tiny helpers by pixhawk. This runs unchecked at the moment.
//  ===============================================================================
void LSM303D_write_checked_reg(uint8_t addr, uint8_t value)
{
	LSM303D_write_reg(addr, value);
}


//  ===============================================================================
//	LSM303D_modify_reg
/// @brief	tiny helpers by pixhawk
//  ===============================================================================
void LSM303D_modify_reg(unsigned reg, uint8_t clearbits, uint8_t setbits)
{
	uint8_t	val;

	val = LSM303D_read_reg(reg);
	val &= ~clearbits;
	val |= setbits;
	LSM303D_write_checked_reg(reg, val);
}



//  ===============================================================================
//	LSM303DLHC_accelerator_read_req
/// @brief	
//  ===============================================================================
uint8_t LSM303DLHC_accelerator_read_req(uint8_t addr)
{
  uint8_t data;

	I2C_Master_Transmit( DEVICE_ACCELARATOR_303DLHC, &addr, 1);
	I2C_Master_Receive(  DEVICE_ACCELARATOR_303DLHC, &data, 1);
	return data;
}


//  ===============================================================================
//	LSM303DLHC_accelerator_write_req
/// @brief	
//  ===============================================================================
void LSM303DLHC_accelerator_write_req(uint8_t addr, uint8_t value)
{
  uint8_t data[2];
	
	/* enable accel*/
	data[0] = addr;
	data[1] = value;
	I2C_Master_Transmit( DEVICE_ACCELARATOR_303DLHC, data, 2);
}

/*
//  ===============================================================================
//	LSM303D_accel_set_range
/// @brief	tiny helpers by pixhawk
//  ===============================================================================
int LSM303D_accel_set_range(unsigned max_g)
{
	uint8_t setbits = 0;
	uint8_t clearbits = REG2_FULL_SCALE_BITS_A;
	float new_scale_g_digit = 0.0f;

	if (max_g == 0) {
		max_g = 16;
	}

	if (max_g <= 2) {
		_accel_range_m_s2 = 2.0f * LSM303D_ONE_G;
		setbits |= REG2_FULL_SCALE_2G_A;
		new_scale_g_digit = 0.061e-3f;

	} else if (max_g <= 4) {
		_accel_range_m_s2 = 4.0f * LSM303D_ONE_G;
		setbits |= REG2_FULL_SCALE_4G_A;
		new_scale_g_digit = 0.122e-3f;

	} else if (max_g <= 6) {
		_accel_range_m_s2 = 6.0f * LSM303D_ONE_G;
		setbits |= REG2_FULL_SCALE_6G_A;
		new_scale_g_digit = 0.183e-3f;

	} else if (max_g <= 8) {
		_accel_range_m_s2 = 8.0f * LSM303D_ONE_G;
		setbits |= REG2_FULL_SCALE_8G_A;
		new_scale_g_digit = 0.244e-3f;

	} else if (max_g <= 16) {
		_accel_range_m_s2 = 16.0f * LSM303D_ONE_G;
		setbits |= REG2_FULL_SCALE_16G_A;
		new_scale_g_digit = 0.732e-3f;

	} else {
		return -1;
	}

	_accel_range_scale = new_scale_g_digit * LSM303D_ONE_G;


	LSM303D_modify_reg(ADDR_CTRL_REG2, clearbits, setbits);

	return 0;
}
*/
/*
//  ===============================================================================
//	LSM303D_mag_set_range
/// @brief	tiny helpers by pixhawk
//  ===============================================================================
int LSM303D_mag_set_range(unsigned max_ga)
{
	uint8_t setbits = 0;
	uint8_t clearbits = REG6_FULL_SCALE_BITS_M;
	float new_scale_ga_digit = 0.0f;

	if (max_ga == 0) {
		max_ga = 12;
	}

	if (max_ga <= 2) {
		_mag_range_ga = 2;
		setbits |= REG6_FULL_SCALE_2GA_M;
		new_scale_ga_digit = 0.080e-3f;

	} else if (max_ga <= 4) {
		_mag_range_ga = 4;
		setbits |= REG6_FULL_SCALE_4GA_M;
		new_scale_ga_digit = 0.160e-3f;

	} else if (max_ga <= 8) {
		_mag_range_ga = 8;
		setbits |= REG6_FULL_SCALE_8GA_M;
		new_scale_ga_digit = 0.320e-3f;

	} else if (max_ga <= 12) {
		_mag_range_ga = 12;
		setbits |= REG6_FULL_SCALE_12GA_M;
		new_scale_ga_digit = 0.479e-3f;

	} else {
		return -1;
	}

	_mag_range_scale = new_scale_ga_digit;

	LSM303D_modify_reg(ADDR_CTRL_REG6, clearbits, setbits);

	return 0;
}
*/

//  ===============================================================================
//	LSM303D_accel_set_onchip_lowpass_filter_bandwidth
/// @brief	tiny helpers by pixhawk
//  ===============================================================================
int LSM303D_accel_set_onchip_lowpass_filter_bandwidth(unsigned bandwidth)
{
	uint8_t setbits = 0;
	uint8_t clearbits = REG2_ANTIALIAS_FILTER_BW_BITS_A;

	if (bandwidth == 0) {
		bandwidth = 773;
	}

	if (bandwidth <= 50) {
		setbits |= REG2_AA_FILTER_BW_50HZ_A;
		_accel_onchip_filter_bandwith = 50;

	} else if (bandwidth <= 194) {
		setbits |= REG2_AA_FILTER_BW_194HZ_A;
		_accel_onchip_filter_bandwith = 194;

	} else if (bandwidth <= 362) {
		setbits |= REG2_AA_FILTER_BW_362HZ_A;
		_accel_onchip_filter_bandwith = 362;

	} else if (bandwidth <= 773) {
		setbits |= REG2_AA_FILTER_BW_773HZ_A;
		_accel_onchip_filter_bandwith = 773;

	} else {
		return -1;
	}

	LSM303D_modify_reg(ADDR_CTRL_REG2, clearbits, setbits);

	return 0;
}


//  ===============================================================================
//	LSM303D_accel_set_driver_lowpass_filter
/// @brief	tiny helpers by pixhawk. This one is not used at the moment!
//  ===============================================================================
int LSM303D_accel_set_driver_lowpass_filter(float samplerate, float bandwidth)
{
/*
	_accel_filter_x_set_cutoff_frequency(samplerate, bandwidth);
	_accel_filter_y_set_cutoff_frequency(samplerate, bandwidth);
	_accel_filter_z_set_cutoff_frequency(samplerate, bandwidth);
*/	
	return 0;
}

/* unused 170821
//  ===============================================================================
//	LSM303D_accel_set_samplerate
/// @brief	tiny helpers by pixhawk
//  ===============================================================================
int LSM303D_accel_set_samplerate(unsigned frequency)
{
	uint8_t setbits = 0;
	uint8_t clearbits = REG1_RATE_BITS_A;

//	if (frequency == 0 || frequency == ACCEL_SAMPLERATE_DEFAULT) {
		frequency = 1600;
//	}

	if (frequency <= 3) {
		setbits |= REG1_RATE_3_125HZ_A;
		_accel_samplerate = 3;

	} else if (frequency <= 6) {
		setbits |= REG1_RATE_6_25HZ_A;
		_accel_samplerate = 6;

	} else if (frequency <= 12) {
		setbits |= REG1_RATE_12_5HZ_A;
		_accel_samplerate = 12;

	} else if (frequency <= 25) {
		setbits |= REG1_RATE_25HZ_A;
		_accel_samplerate = 25;

	} else if (frequency <= 50) {
		setbits |= REG1_RATE_50HZ_A;
		_accel_samplerate = 50;

	} else if (frequency <= 100) {
		setbits |= REG1_RATE_100HZ_A;
		_accel_samplerate = 100;

	} else if (frequency <= 200) {
		setbits |= REG1_RATE_200HZ_A;
		_accel_samplerate = 200;

	} else if (frequency <= 400) {
		setbits |= REG1_RATE_400HZ_A;
		_accel_samplerate = 400;

	} else if (frequency <= 800) {
		setbits |= REG1_RATE_800HZ_A;
		_accel_samplerate = 800;

	} else if (frequency <= 1600) {
		setbits |= REG1_RATE_1600HZ_A;
		_accel_samplerate = 1600;

	} else {
		return -1;
	}

	LSM303D_modify_reg(ADDR_CTRL_REG1, clearbits, setbits);
	return 0;
}
//  ===============================================================================
//	LSM303D_mag_set_samplerate
/// @brief	tiny helpers by pixhawk
//  ===============================================================================
int LSM303D_mag_set_samplerate(unsigned frequency)
{
	uint8_t setbits = 0;
	uint8_t clearbits = REG5_RATE_BITS_M;

	if (frequency == 0) {
		frequency = 100;
	}

	if (frequency <= 3) {
		setbits |= REG5_RATE_3_125HZ_M;
		_mag_samplerate = 25;

	} else if (frequency <= 6) {
		setbits |= REG5_RATE_6_25HZ_M;
		_mag_samplerate = 25;

	} else if (frequency <= 12) {
		setbits |= REG5_RATE_12_5HZ_M;
		_mag_samplerate = 25;

	} else if (frequency <= 25) {
		setbits |= REG5_RATE_25HZ_M;
		_mag_samplerate = 25;

	} else if (frequency <= 50) {
		setbits |= REG5_RATE_50HZ_M;
		_mag_samplerate = 50;

	} else if (frequency <= 100) {
		setbits |= REG5_RATE_100HZ_M;
		_mag_samplerate = 100;

	} else {
		return -1;
	}

	LSM303D_modify_reg(ADDR_CTRL_REG5, clearbits, setbits);
	return 0;
}
*/


// rotate_mag_3f: nicht genutzt aber praktisch; rotate_accel_3f wird benutzt
//  ===============================================================================
//	rotate_mag_3f
/// @brief	swap axis in convient way, by hw
/// @param 	*x raw input is set to *y input
/// @param 	*y raw input is set to -*x input
/// @param 	*z raw is not touched
//  ===============================================================================
void rotate_mag_3f(float *x, float *y, float *z)
{
	return;
/*	
	*x = *x;		// HMC: *x = -*y
	*y = *y;		// HMC: *y =  *x  // change 20.04.2016: zuvor *y = -*y
	*z = *z;		// HMC: *z =  *z  
*/	
}


//  ===============================================================================
//	rotate_accel_3f
/// @brief	swap axis in convient way, by hw, same as MMA8452Q
/// @param 	*x raw input, output is with sign change
/// @param 	*y raw input, output is with sign change
/// @param 	*z raw input, output is with sign change
//  ===============================================================================
void rotate_accel_3f(float *x, float *y, float *z)
{
	*x = -*x;
	*y = -*y;
	*z = -*z;
	/* tested:
	x = x, y =-y, z=-z: does not work with roll
	x = x, y =y, z=-z: does not work with pitch
	x = x, y =y, z=z: does not work at all
	*/
}


//  ===============================================================================
//	compass_init_LSM303D by PIXhawk (LSM303D::reset())
//  https://raw.githubusercontent.com/PX4/Firmware/master/src/drivers/lsm303d/lsm303d.cpp
/// @brief	The new ST 303D 
///					This might be called several times with different gain values during calibration
///					but gain change is not supported at the moment.
///
/// @param 	gain: 7 is max gain and set with here, compass_calib() might reduce it
//  ===============================================================================

//uint8_t testCompassLS303D[11];

void compass_init_LSM303D(uint8_t fast, uint8_t gain)
{
// matthias version 160620
	if(fast == 0)
	{
		LSM303D_write_checked_reg(ADDR_CTRL_REG0, 0x00);
		LSM303D_write_checked_reg(ADDR_CTRL_REG1, 0x3F); // mod 12,5 Hz 3 instead of 6,25 Hz 2
		LSM303D_write_checked_reg(ADDR_CTRL_REG2, 0xC0);
		LSM303D_write_checked_reg(ADDR_CTRL_REG3, 0x00);
		LSM303D_write_checked_reg(ADDR_CTRL_REG4, 0x00);
		LSM303D_write_checked_reg(ADDR_CTRL_REG5, 0x68); // mod 12,5 Hz 8 instead of 6,25 Hz 4
	}
	else
	{
		LSM303D_write_checked_reg(ADDR_CTRL_REG0, 0x00);
		LSM303D_write_checked_reg(ADDR_CTRL_REG1, 0x6F); // 100 Hz
		LSM303D_write_checked_reg(ADDR_CTRL_REG2, 0xC0);
		LSM303D_write_checked_reg(ADDR_CTRL_REG3, 0x00);
		LSM303D_write_checked_reg(ADDR_CTRL_REG4, 0x00);
		LSM303D_write_checked_reg(ADDR_CTRL_REG5, 0x74); // 100 Hz
	}
	LSM303D_write_checked_reg(ADDR_CTRL_REG6, 0x00);
	LSM303D_write_checked_reg(ADDR_CTRL_REG7, 0x00);

	/*
uint8_t data;
for(int i=0;i<11;i++)
{
	data = ADDR_INT_THS_L_M + i;
	I2C_Master_Transmit( DEVICE_COMPASS_303D, &data, 1);
	I2C_Master_Receive(  DEVICE_COMPASS_303D, &testCompassLS303D[i], 1);
}
*/	
	
	return;
/*
	LSM303D_accel_set_range(LSM303D_ACCEL_DEFAULT_RANGE_G); // modifies ADDR_CTRL_REG2
	LSM303D_accel_set_samplerate(LSM303D_ACCEL_DEFAULT_RATE); // modifies ADDR_CTRL_REG1

	LSM303D_mag_set_range(LSM303D_MAG_DEFAULT_RANGE_GA);
	LSM303D_mag_set_samplerate(LSM303D_MAG_DEFAULT_RATE);
*/
	
/*
// my stuff hw
	// enable accel
	LSM303D_write_checked_reg(ADDR_CTRL_REG1,
			  REG1_X_ENABLE_A | REG1_Y_ENABLE_A | REG1_Z_ENABLE_A | REG1_BDU_UPDATE | REG1_RATE_800HZ_A);
	
	// enable mag
	LSM303D_write_checked_reg(ADDR_CTRL_REG7, REG7_CONT_MODE_M);
	LSM303D_write_checked_reg(ADDR_CTRL_REG5, REG5_RES_HIGH_M | REG5_ENABLE_T);
	LSM303D_write_checked_reg(ADDR_CTRL_REG3, 0x04); // DRDY on ACCEL on INT1
	LSM303D_write_checked_reg(ADDR_CTRL_REG4, 0x04); // DRDY on MAG on INT2

	LSM303D_accel_set_range(LSM303D_ACCEL_DEFAULT_RANGE_G);
	LSM303D_accel_set_samplerate(LSM303D_ACCEL_DEFAULT_RATE);
	LSM303D_accel_set_driver_lowpass_filter((float)LSM303D_ACCEL_DEFAULT_RATE, (float)LSM303D_ACCEL_DEFAULT_DRIVER_FILTER_FREQ);
	//LSM303D_accel_set_onchip_lowpass_filter_bandwidth(773); // factory setting	
	
	// we setup the anti-alias on-chip filter as 50Hz. We believe
	// this operates in the analog domain, and is critical for
	// anti-aliasing. The 2 pole software filter is designed to
	// operate in conjunction with this on-chip filter
	if(fast)
		LSM303D_accel_set_onchip_lowpass_filter_bandwidth(773); // factory setting
	else
		LSM303D_accel_set_onchip_lowpass_filter_bandwidth(LSM303D_ACCEL_DEFAULT_ONCHIP_FILTER_FREQ);


	LSM303D_mag_set_range(LSM303D_MAG_DEFAULT_RANGE_GA);
	LSM303D_mag_set_samplerate(LSM303D_MAG_DEFAULT_RATE);
*/
}


//  ===============================================================================
//	compass_sleep_LSM303D
/// @brief	The new compass chip, hopefully this works!
//  ===============================================================================
void compass_sleep_LSM303D(void)
{
	LSM303D_write_checked_reg(ADDR_CTRL_REG1, 0x00); // CNTRL1: acceleration sensor Power-down mode
	LSM303D_write_checked_reg(ADDR_CTRL_REG7, 0x02); // CNTRL7: magnetic sensor Power-down mode
}


//  ===============================================================================
//	acceleration_read_LSM303D
/// @brief	The new LSM303D, code by pixhawk
///
/// output is accel_DX_f, accel_DY_f, accel_DZ_f
//  ===============================================================================
void acceleration_read_LSM303D(void)
{
  uint8_t data;
	float xraw_f, yraw_f, zraw_f;
	float accel_report_x, accel_report_y, accel_report_z;
	
  memset(accDataBuffer,0,6);

	accel_DX_f = 0;
	accel_DY_f = 0;
	accel_DZ_f = 0;
	
	for(int i=0;i<6;i++)
	{
		data = ADDR_OUT_X_L_A + i;
		I2C_Master_Transmit( DEVICE_COMPASS_303D, &data, 1);
		I2C_Master_Receive(  DEVICE_COMPASS_303D, &accDataBuffer[i], 1);
	}
	
	xraw_f = ((float)( (int16_t)((accDataBuffer[1] << 8) | (accDataBuffer[0]))));
	yraw_f = ((float)( (int16_t)((accDataBuffer[3] << 8) | (accDataBuffer[2]))));
	zraw_f = ((float)( (int16_t)((accDataBuffer[5] << 8) | (accDataBuffer[4]))));	

	rotate_accel_3f(&xraw_f, &yraw_f, &zraw_f); 
	
	// mh
	accel_report_x = xraw_f;
	accel_report_y = yraw_f;
	accel_report_z = zraw_f;

	// my stuff
/*	
	accel_report_x = ((xraw_f * _accel_range_scale) - _accel_scale_x_offset) * _accel_scale_x_scale;
	accel_report_y = ((yraw_f * _accel_range_scale) - _accel_scale_y_offset) * _accel_scale_y_scale;
	accel_report_z = ((zraw_f * _accel_range_scale) - _accel_scale_z_offset) * _accel_scale_z_scale;
*/	
	accel_DX_f = ((int16_t)(accel_report_x));
	accel_DY_f = ((int16_t)(accel_report_y));
	accel_DZ_f = ((int16_t)(accel_report_z));
}
/* special code after accel_report_z = ...
 * prior to output
//	  we have logs where the accelerometers get stuck at a fixed
//	  large value. We want to detect this and mark the sensor as
//	  being faulty

	if (fabsf(_last_accel[0] - x_in_new) < 0.001f &&
	    fabsf(_last_accel[1] - y_in_new) < 0.001f &&
	    fabsf(_last_accel[2] - z_in_new) < 0.001f &&
	    fabsf(x_in_new) > 20 &&
	    fabsf(y_in_new) > 20 &&
	    fabsf(z_in_new) > 20) {
		_constant_accel_count += 1;

	} else {
		_constant_accel_count = 0;
	}

	if (_constant_accel_count > 100) {
		// we've had 100 constant accel readings with large
		// values. The sensor is almost certainly dead. We
		// will raise the error_count so that the top level
		// flight code will know to avoid this sensor, but
		// we'll still give the data so that it can be logged
		// and viewed
		perf_count(_bad_values);
		_constant_accel_count = 0;
	}

	_last_accel[0] = x_in_new;
	_last_accel[1] = y_in_new;
	_last_accel[2] = z_in_new;

	accel_report.x = _accel_filter_x.apply(x_in_new);
	accel_report.y = _accel_filter_y.apply(y_in_new);
	accel_report.z = _accel_filter_z.apply(z_in_new);

	math::Vector<3> aval(x_in_new, y_in_new, z_in_new);
	math::Vector<3> aval_integrated;

	bool accel_notify = _accel_int.put(accel_report.timestamp, aval, aval_integrated, accel_report.integral_dt);
	accel_report.x_integral = aval_integrated(0);
	accel_report.y_integral = aval_integrated(1);
	accel_report.z_integral = aval_integrated(2);
*/


//  ===============================================================================
//	compass_read_LSM303D
/// @brief	The new LSM303D, code by pixhawk
///
/// output is compass_DX_f, compass_DY_f, compass_DZ_f
//  ===============================================================================
void compass_read_LSM303D(void)
{
  uint8_t data;
//	float xraw_f, yraw_f, zraw_f;
//	float mag_report_x, mag_report_y, mag_report_z;
	
  memset(magDataBuffer,0,6);

	compass_DX_f = 0;
	compass_DY_f = 0;
	compass_DZ_f = 0;
	
	for(int i=0;i<6;i++)
	{
		data = ADDR_OUT_X_L_M + i;
		I2C_Master_Transmit( DEVICE_COMPASS_303D, &data, 1);
		I2C_Master_Receive(  DEVICE_COMPASS_303D, &magDataBuffer[i], 1);
	}

	// mh 160620 flip x and y if flip display
	compass_DX_f = (((int16_t)((magDataBuffer[1] << 8) | (magDataBuffer[0]))));
	compass_DY_f = (((int16_t)((magDataBuffer[3] << 8) | (magDataBuffer[2]))));
	compass_DZ_f = (((int16_t)((magDataBuffer[5] << 8) | (magDataBuffer[4]))));
	// no rotation
	return;
/*	
	// my stuff
	compass_DX_f = (((int16_t)((magDataBuffer[1] << 8) | (magDataBuffer[0]))) / 10) - 200;
	compass_DY_f = (((int16_t)((magDataBuffer[3] << 8) | (magDataBuffer[2]))) / 10) - 200;
	compass_DZ_f = (((int16_t)((magDataBuffer[5] << 8) | (magDataBuffer[4]))) / 10) - 200;
*/
	// old
	/*	
	xraw_f = ((float)( (int16_t)((magDataBuffer[1] << 8) | (magDataBuffer[0]))));
	yraw_f = ((float)( (int16_t)((magDataBuffer[3] << 8) | (magDataBuffer[2]))));
	zraw_f = ((float)( (int16_t)((magDataBuffer[5] << 8) | (magDataBuffer[4]))));	

	rotate_mag_3f(&xraw_f, &yraw_f, &zraw_f); 

	compass_DX_f = (int16_t)((xraw_f * 0.1f) - 200.0f);
	compass_DY_f = (int16_t)((yraw_f * 0.1f) - 200.0f);
	compass_DZ_f = (int16_t)((zraw_f * 0.1f) - 200.0f);
*/	
/*	
	mag_report_x = ((xraw_f * _mag_range_scale) - _mag_scale_x_offset) * _mag_scale_x_scale;
	mag_report_y = ((yraw_f * _mag_range_scale) - _mag_scale_y_offset) * _mag_scale_y_scale;
	mag_report_z = ((zraw_f * _mag_range_scale) - _mag_scale_z_offset) * _mag_scale_z_scale;
	
	compass_DX_f = (int16_t)(mag_report_x * 1000.0f); // 1000.0 is just a wild guess by hw
	compass_DY_f = (int16_t)(mag_report_y * 1000.0f);
	compass_DZ_f = (int16_t)(mag_report_z * 1000.0f);
*/	
}


//  ===============================================================================
//	compass_init_LSM303DLHC
/// @brief	The new ST 303DLHC 2017/2018
///					This might be called several times with different gain values during calibration
///					but gain change is not supported at the moment.
///					parts from KOMPASS LSM303DLH-compass-app-note.pdf
///
/// @param 	gain:
/// @param 	fast:
//  ===============================================================================



void compass_init_LSM303DLHC(uint8_t fast, uint8_t gain)
{
// acceleration
	// todo : BDU an (wie 303D) und high res, beides in REG4
		//LSM303D_write_checked_reg(DLHC_CTRL_REG2_A,0x00); // 0x00 default, hier k�nnte filter sein 0x8?80, cutoff freq. not beschrieben
	
	if(fast == 0)
	{
		LSM303DLHC_accelerator_write_req(DLHC_CTRL_REG1_A, 0x27); // 10 hz
	}
	else
	{
		LSM303DLHC_accelerator_write_req(DLHC_CTRL_REG1_A, 0x57); // 100 hz
	}
//	LSM303D_write_checked_reg(DLHC_CTRL_REG4_A, 0x88); // 0x88: BDU + HighRes, BDU ist doof!
	LSM303D_write_checked_reg(DLHC_CTRL_REG4_A, 0x00); // 0x00 little-endian, ist's immer
//	LSM303D_write_checked_reg(DLHC_CTRL_REG4_A, 0x08); // 0x08: HighRes
	//LSM303D_write_checked_reg(DLHC_CTRL_REG4_A, 0x80); // 
	
	
// compass
		LSM303D_write_checked_reg(DLHC_CRA_REG_M,0x10);	// 15 Hz

	if(fast == 0)
	{
		LSM303D_write_checked_reg(DLHC_CRA_REG_M,0x10);	// 15 Hz
	}
	else
	{
		LSM303D_write_checked_reg(DLHC_CRA_REG_M,0x18);	// 75 Hz
	}
	LSM303D_write_checked_reg(DLHC_CRB_REG_M,0x20);	// 0x60: 2.5 Gauss ,0x40: +/-1.9 Gauss,0x20: +/-1.3 Gauss
	LSM303D_write_checked_reg(DLHC_MR_REG_M,0x00); //continuous conversation
	

	
	return;
	
	
//		LSM303D_write_checked_reg(,);
//		LSM303D_write_checked_reg(DLHC_CTRL_REG1_A, 0x27); // 0x27 = acc. normal mode with ODR 50Hz - passt nicht mit datenblatt!!
//		LSM303D_write_checked_reg(DLHC_CTRL_REG4_A, 0x40); // 0x40 = full scale range �2 gauss in continuous data update mode and change the little-endian to a big-endian structure.

	if(fast == 0)
	{
		LSM303DLHC_accelerator_write_req(DLHC_CTRL_REG1_A, 0x27); // 0x27 = acc. normal mode, all axes, with ODR 10HZ laut LSM303DLHC, page 25/42
		//
		//LSM303D_write_checked_reg(DLHC_CTRL_REG2_A,0x00); // 0x00 default, hier k�nnte filter sein 0x8?80, cutoff freq. not beschrieben
		//LSM303D_write_checked_reg(DLHC_CTRL_REG3_A,0x00); // 0x00 default
		//
		LSM303DLHC_accelerator_write_req(DLHC_CTRL_REG4_A, 0x00); // 0x00 = ich glaube little-endian ist gut
//		LSM303D_write_checked_reg(DLHC_CTRL_REG4_A, 0x40); // 0x00 = ich glaube little-endian ist gut
		//
		//LSM303D_write_checked_reg(DLHC_CTRL_REG5_A,0x00); // 0x00 default
		//LSM303D_write_checked_reg(DLHC_CTRL_REG6_A,0x00); // 0x00 default
		// magnetic sensor
		LSM303D_write_checked_reg(DLHC_CRA_REG_M,0x10);	// 15 Hz
	}
	else
	{
		LSM303DLHC_accelerator_write_req(DLHC_CTRL_REG1_A, 0x57); // 0x57 = acc. normal mode, all axes, with ODR 100HZ, LSM303DLHC, page 25/42
		//
		//LSM303D_write_checked_reg(DLHC_CTRL_REG2_A,0x00); // 0x00 default, hier k�nnte filter sein 0x8?80, cutoff freq. not beschrieben
		//LSM303D_write_checked_reg(DLHC_CTRL_REG3_A,0x00); // 0x00 default
		//
		LSM303DLHC_accelerator_write_req(DLHC_CTRL_REG4_A, 0x00); // 0x00 = ich glaube little-endian ist gut
//		LSM303D_write_checked_reg(DLHC_CTRL_REG4_A, 0x40); // 0x00 = ich glaube little-endian ist gut
		//
		//LSM303D_write_checked_reg(DLHC_CTRL_REG5_A,0x00); // 0x00 default
		//LSM303D_write_checked_reg(DLHC_CTRL_REG6_A,0x00); // 0x00 default
		// magnetic sensor
		LSM303D_write_checked_reg(DLHC_CRA_REG_M,0x18); // 75 Hz
	}
	LSM303D_write_checked_reg(DLHC_CRB_REG_M,0x02);	// +/-1.9 Gauss
	LSM303D_write_checked_reg(DLHC_MR_REG_M,0x00); //continuous conversation
	
	
/*	
// matthias version 160620
	if(fast == 0)
	{
		LSM303D_write_checked_reg(ADDR_CTRL_REG0, 0x00);
		LSM303D_write_checked_reg(ADDR_CTRL_REG1, 0x3F); // mod 12,5 Hz 3 instead of 6,25 Hz 2
		LSM303D_write_checked_reg(ADDR_CTRL_REG2, 0xC0); // anti alias 50 Hz (minimum)
		LSM303D_write_checked_reg(ADDR_CTRL_REG3, 0x00);
		LSM303D_write_checked_reg(ADDR_CTRL_REG4, 0x00);
		LSM303D_write_checked_reg(ADDR_CTRL_REG5, 0x68); // mod 12,5 Hz 8 instead of 6,25 Hz 4
	}
	else
	{
		LSM303D_write_checked_reg(ADDR_CTRL_REG0, 0x00);
		LSM303D_write_checked_reg(ADDR_CTRL_REG1, 0x6F); // 100 Hz
		LSM303D_write_checked_reg(ADDR_CTRL_REG2, 0xC0);
		LSM303D_write_checked_reg(ADDR_CTRL_REG3, 0x00);
		LSM303D_write_checked_reg(ADDR_CTRL_REG4, 0x00);
		LSM303D_write_checked_reg(ADDR_CTRL_REG5, 0x74); // 100 Hz
	}
	LSM303D_write_checked_reg(ADDR_CTRL_REG6, 0x00);
	LSM303D_write_checked_reg(ADDR_CTRL_REG7, 0x00);
*/
	return;
}

//  ===============================================================================
//	compass_sleep_LSM303DLHC
/// @brief	The new 2017/2018 compass chip.
//  ===============================================================================
void compass_sleep_LSM303DLHC(void)
{
	LSM303DLHC_accelerator_write_req(DLHC_CTRL_REG1_A, 0x07);	// CTRL_REG1_A: linear acceleration Power-down mode
	LSM303D_write_checked_reg(DLHC_MR_REG_M, 0x02); 		// MR_REG_M: magnetic sensor Power-down mode
}


//  ===============================================================================
//	compass_read_LSM303DLHC
/// @brief	The new 2017/2018 compass chip.
//  ===============================================================================
void compass_read_LSM303DLHC(void)
{
  uint8_t data;
	
  memset(magDataBuffer,0,6);

	compass_DX_f = 0;
	compass_DY_f = 0;
	compass_DZ_f = 0;
	
	for(int i=0;i<6;i++)
	{
		data = DLHC_OUT_X_L_M + i;
		I2C_Master_Transmit( DEVICE_COMPASS_303D, &data, 1);
		I2C_Master_Receive(  DEVICE_COMPASS_303D, &magDataBuffer[i], 1);
	}

	// 303DLHC new order
	compass_DX_f = (((int16_t)((magDataBuffer[0] << 8) | (magDataBuffer[1]))));
	compass_DZ_f = (((int16_t)((magDataBuffer[2] << 8) | (magDataBuffer[3]))));
	compass_DY_f = (((int16_t)((magDataBuffer[4] << 8) | (magDataBuffer[5]))));

	// no rotation, otherwise see compass_read_LSM303D()
	return;
}


//  ===============================================================================
//	acceleration_read_LSM303DLHC
/// @brief	The new 2017/2018 compass chip.
//  ===============================================================================
void acceleration_read_LSM303DLHC(void)
{
  uint8_t data;
	float xraw_f, yraw_f, zraw_f;
	float accel_report_x, accel_report_y, accel_report_z;
	
  memset(accDataBuffer,0,6);

	accel_DX_f = 0;
	accel_DY_f = 0;
	accel_DZ_f = 0;
	
	for(int i=0;i<6;i++)
	{
		data = DLHC_OUT_X_L_A + i;
		I2C_Master_Transmit( DEVICE_ACCELARATOR_303DLHC, &data, 1);
		I2C_Master_Receive(  DEVICE_ACCELARATOR_303DLHC, &accDataBuffer[i], 1);
	}
	
	xraw_f = ((float)( (int16_t)((accDataBuffer[1] << 8) | (accDataBuffer[0]))));
	yraw_f = ((float)( (int16_t)((accDataBuffer[3] << 8) | (accDataBuffer[2]))));
	zraw_f = ((float)( (int16_t)((accDataBuffer[5] << 8) | (accDataBuffer[4]))));	

	rotate_accel_3f(&xraw_f, &yraw_f, &zraw_f); 
	
	// mh f�r 303D
	accel_report_x = xraw_f;
	accel_report_y = yraw_f;
	accel_report_z = zraw_f;

	accel_DX_f = ((int16_t)(accel_report_x));
	accel_DY_f = ((int16_t)(accel_report_y));
	accel_DZ_f = ((int16_t)(accel_report_z));
}


// --------------------------------------------------------------------------------
// ----------EARLIER COMPONENTS ---------------------------------------------------
// --------------------------------------------------------------------------------

//  ===============================================================================
//	compass_init_HMC5883L
/// @brief	The horrible Honeywell compass chip
///					This might be called several times during calibration
///
/// @param 	fast: 1 is fast mode, 0 is normal mode
/// @param 	gain: 7 is max gain and set with here, compass_calib() might reduce it
//  ===============================================================================
void compass_init_HMC5883L(uint8_t fast, uint8_t gain)
{
	uint8_t write_buffer[4];

	compass_gain = gain;
	uint16_t length = 0;
	write_buffer[0] = 0x00; // 00 = config Register A
	
	if( fast )
		write_buffer[1] = 0x38;	// 0b 0011 1000; // ConfigA: 75Hz, 2 Samples averaged
	else
		write_buffer[1] = 0x68; // 0b 0110 1000; // ConfigA:  3Hz, 8 Samples averaged

	switch(gain)
	{
			case 7:
				write_buffer[2] = 0xE0;  //0b 1110 0000; // ConfigB: gain
				break;
			case 6:
				write_buffer[2] = 0xC0;  //0b 1100 0000; // ConfigB: gain
				break;
			case 5:
				write_buffer[2] = 0xA0;  //0b 1010 0000; // ConfigB: gain
				break;
			case 4:
				write_buffer[2] = 0x80;  //0b 1000 0000; // ConfigB: gain
				break;
			case 3:
				write_buffer[2] = 0x60;  //0b 0110 0000; // ConfigB: gain
				break;
			case 2:
				write_buffer[2] = 0x40;  //0b 01000 0000; // ConfigB: gain
				break;
			case 1:
				write_buffer[2] = 0x20;  //0b 00100 0000; // ConfigB: gain
				break;
			case 0:
				write_buffer[2] = 0x00;  //0b 00000 0000; // ConfigB: gain
				break;
	}
	write_buffer[3] = 0x00; // Mode: continuous mode
	length = 4;
	//hmc_twi_write(0);
	I2C_Master_Transmit(  DEVICE_COMPASS_HMC5883L, write_buffer, length);
}



//  ===============================================================================
//	compass_sleep_HMC5883L
/// @brief	Power-down mode for Honeywell compass chip
//  ===============================================================================
void compass_sleep_HMC5883L(void)
{
	uint8_t write_buffer[4];
	uint16_t length = 0;
	
	write_buffer[0] = 0x00; // 00 = config Register A
	write_buffer[1] = 0x68; // 0b 0110 1000; // ConfigA
	write_buffer[2] = 0x20; // 0b 0010 0000; // ConfigB
	write_buffer[3] = 0x02; // 0b 0000 0010; // Idle Mode
	length = 4;
	I2C_Master_Transmit(  DEVICE_COMPASS_HMC5883L, write_buffer, length);
}


//  ===============================================================================
//	accelerator_init_MMA8452Q
/// @brief	Power-down mode for acceleration chip used in combination with Honeywell compass
//  ===============================================================================
void accelerator_init_MMA8452Q(void)
{
	uint8_t write_buffer[4];
	uint16_t length = 0;
	//HAL_Delay(1);
	//return;
	write_buffer[0] = 0x0E; // XYZ_DATA_CFG
	write_buffer[1] = 0x00;//0b00000000; // High pass Filter=0 , +/- 2g range
	length = 2;
	I2C_Master_Transmit(  DEVICE_ACCELARATOR_MMA8452Q, write_buffer, length);
	//HAL_Delay(1);
	write_buffer[0] = 0x2A; // CTRL_REG1
	write_buffer[1] = 0x34; //0b00110100; // CTRL_REG1: 160ms data rate, St.By Mode, reduced noise mode
	write_buffer[2] = 0x02; //0b00000010; // CTRL_REG2: High Res in Active mode
	length = 3;
	I2C_Master_Transmit(  DEVICE_ACCELARATOR_MMA8452Q, write_buffer, length);
	
	//HAL_Delay(1);
	//hw_delay_us(100);
	write_buffer[0] = 0x2A; // CTRL_REG1
	write_buffer[1] = 0x35; //0b00110101; // CTRL_REG1: ... Active Mode
	length = 2;
	I2C_Master_Transmit(  DEVICE_ACCELARATOR_MMA8452Q, write_buffer, length);
/*	
	HAL_Delay(6);
	compass_read();
	HAL_Delay(1);
	acceleration_read();
	
	compass_calc();
*/	
}


//  ===============================================================================
//	accelerator_sleep_MMA8452Q
/// @brief	compass_sleep_HMC5883L
//  ===============================================================================
void accelerator_sleep_MMA8452Q(void)
{
	uint16_t length = 0;
	uint8_t write_buffer[4];
	
	write_buffer[0] = 0x2A; // CTRL_REG1
	write_buffer[1] = 0x00; //0b00000000; // CTRL_REG1: Standby Mode
	length = 2;
	I2C_Master_Transmit(  DEVICE_ACCELARATOR_MMA8452Q, write_buffer, length);
}


//  ===============================================================================
//	compass_read_HMC5883L
/// @brief	The new ST 303D - get ALL data and store in static variables
///
/// output is compass_DX_f, compass_DY_f, compass_DZ_f
//  ===============================================================================
void compass_read_HMC5883L(void)
{
	uint8_t buffer[20];
	compass_DX_f = 0;
	compass_DY_f = 0;
	compass_DZ_f = 0;
	uint8_t length = 0;
	uint8_t read_buffer[6];
	signed_tword 	data;
	for(int i = 0; i<6;i++)
		read_buffer[i] = 0;
	buffer[0] = 0x03; // 03 = Data Output X MSB Register
	length = 1;
	I2C_Master_Transmit(  DEVICE_COMPASS_HMC5883L, buffer, length);
	length = 6;
	I2C_Master_Receive(  DEVICE_COMPASS_HMC5883L, read_buffer, length);
	

	data.Byte.hi = read_buffer[0];
	data.Byte.low = read_buffer[1];
	//Y = Z
	compass_DY_f	= - data.Word;

	data.Byte.hi = read_buffer[2];
	data.Byte.low = read_buffer[3];
	compass_DZ_f = data.Word;

	data.Byte.hi = read_buffer[4];
	data.Byte.low = read_buffer[5]; 
	//X = -Y
	 compass_DX_f 	= data.Word;
}


//  ===============================================================================
//	acceleration_read_MMA8452Q
/// @brief	The old MMA8452Q used with the Honeywell compass
///					get the acceleration data and store in static variables
///
/// output is accel_DX_f, accel_DY_f, accel_DZ_f
//  ===============================================================================
void  acceleration_read_MMA8452Q(void)
{
	uint8_t buffer[20];
	accel_DX_f = 0;
	accel_DY_f = 0;
	accel_DZ_f = 0;
	uint8_t length = 0;
//	bit8_Type status ;
	uint8_t read_buffer[7];
	signed_tword 	data;
	for(int i = 0; i<6;i++)
		read_buffer[i] = 0;
	buffer[0] = 0x00; // 03 = Data Output X MSB Register
	length = 1;
	I2C_Master_Transmit(  DEVICE_ACCELARATOR_MMA8452Q, buffer, length);
	length = 7;
	I2C_Master_Receive(  DEVICE_ACCELARATOR_MMA8452Q, read_buffer, length);
	
//	status.uw = read_buffer[0];
	data.Byte.hi = read_buffer[1];
	data.Byte.low =  read_buffer[2];
	accel_DX_f =data.Word/16;
	data.Byte.hi = read_buffer[3];
	data.Byte.low =  read_buffer[4];
	accel_DY_f =data.Word/16;
	data.Byte.hi = read_buffer[5];
	data.Byte.low =  read_buffer[6];
	accel_DZ_f =data.Word/16;

	accel_DX_f *= -1;
	accel_DY_f *= -1;
	accel_DZ_f *= -1;
}


//  ===============================================================================
//	compass_calc_roll_pitch_only
/// @brief	only the roll and pitch parts of compass_calc()
///
/// input is accel_DX_f, accel_DY_f, accel_DZ_f
/// output is compass_pitch and compass_roll
//  ===============================================================================
void compass_calc_roll_pitch_only(void)
{
	 float sinPhi, cosPhi;
	 float Phi, Teta;

	//---- Calculate sine and cosine of roll angle Phi -----------------------
	Phi= atan2f(accel_DY_f, accel_DZ_f) ;
	compass_roll = Phi * 180.0f /PI;
	sinPhi = sinf(Phi);
	cosPhi = cosf(Phi);

	//---- calculate sin and cosine of pitch angle Theta ---------------------
	Teta = atanf(-(float)accel_DX_f/(accel_DY_f * sinPhi + accel_DZ_f * cosPhi));
	compass_pitch = Teta * 180.0f /PI;
}


//  ===============================================================================
//	compass_calc
/// @brief	all the fancy stuff first implemented in OSTC3
///
/// input is compass_DX_f, compass_DY_f, compass_DZ_f, accel_DX_f, accel_DY_f, accel_DZ_f
/// and compass_CX_f, compass_CY_f, compass_CZ_f
/// output is compass_heading, compass_pitch and compass_roll
//  ===============================================================================
void compass_calc(void)
{
     float	sinPhi, cosPhi, sinTeta, cosTeta;
		 float Phi, Teta, Psi;
     int16_t iBfx, iBfy;
     int16_t iBpx, iBpy, iBpz;
   
    //---- Make hard iron correction -----------------------------------------
    // Measured magnetometer orientation, measured ok.
    // From matthias drawing: (X,Y,Z) --> (X,Y,Z) : no rotation.
    iBpx = compass_DX_f - compass_CX_f; // X
    iBpy = compass_DY_f - compass_CY_f; // Y
    iBpz = compass_DZ_f - compass_CZ_f; // Z

    //---- Calculate sine and cosine of roll angle Phi -----------------------
    //sincos(accel_DZ_f, accel_DY_f, &sin, &cos);
    Phi= atan2f(accel_DY_f, accel_DZ_f) ;
		compass_roll = Phi * 180.0f /PI;
		sinPhi = sinf(Phi);
		cosPhi = cosf(Phi);

		//---- rotate by roll angle (-Phi) ---------------------------------------
    iBfy = iBpy * cosPhi - iBpz * sinPhi;
    iBpz = iBpy * sinPhi + iBpz * cosPhi;
    //Gz = imul(accel_DY_f, sin) + imul(accel_DZ_f, cos);
		
    //---- calculate sin and cosine of pitch angle Theta ---------------------
    //sincos(Gz, -accel_DX_f, &sin, &cos);     // NOTE: changed sin sign.
		// Teta takes into account roll of computer and sends combination of Y and Z :-) understand now hw 160421
		Teta = atanf(-(float)accel_DX_f/(accel_DY_f * sinPhi + accel_DZ_f * cosPhi));
		compass_pitch = Teta * 180.0f /PI;
		sinTeta = sinf(Teta);
		cosTeta = cosf(Teta);
    /* correct cosine if pitch not in range -90 to 90 degrees */
    if( cosTeta < 0 ) cosTeta = -cosTeta;

    ///---- de-rotate by pitch angle Theta -----------------------------------
    iBfx = iBpx *  cosTeta + iBpz * sinTeta;

    //---- Detect uncalibrated compass ---------------------------------------
    if( !compass_CX_f && !compass_CY_f && !compass_CZ_f )
    {
        compass_heading = -1;
        return;
    }

    //---- calculate current yaw = e-compass angle Psi -----------------------
    // Result in degree (no need of 0.01 deg precision...
			Psi = atan2f(-iBfy,iBfx);
		 compass_heading = Psi * 180.0f /PI;
    // Result in 0..360 range:
    if( compass_heading < 0 )
        compass_heading += 360;
}


/*
//  ===============================================================================
//	compass_calc_mini_during_calibration
/// @brief	all the fancy stuff first implemented in OSTC3
///
/// input is accel_DX_f, accel_DY_f, accel_DZ_f
/// output is compass_pitch and compass_roll
//  ===============================================================================
void compass_calc_mini_during_calibration(void)
{
     float	sinPhi, cosPhi;
		 float Phi, Teta;

    //---- Calculate sine and cosine of roll angle Phi -----------------------
    //sincos(accel_DZ_f, accel_DY_f, &sin, &cos);
    Phi= atan2f(accel_DY_f, accel_DZ_f) ;
		compass_roll = Phi * 180.0f /PI;
		sinPhi = sinf(Phi);
		cosPhi = cosf(Phi);
		
    //---- calculate sin and cosine of pitch angle Theta ---------------------
    //sincos(Gz, -accel_DX_f, &sin, &cos);     // NOTE: changed sin sign.
    Teta = atanf(-(float)accel_DX_f/(accel_DY_f * sinPhi + accel_DZ_f * cosPhi));
		compass_pitch = Teta * 180.0f /PI;
}
*/


// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// // - Calibration - ///////////////////////////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* can be lost during sleep as those are reset with compass_reset_calibration() */

//  ===============================================================================
//	compass_reset_calibration
/// @brief	all the fancy stuff first implemented in OSTC3
///
/// output is struct g and compass_CX_f, compass_CY_f, compass_CZ_f
///
/// @param 	g: is a struct with crazy stuff like Suuu, Svvv, Svvu, etc.
///					all is set to zero here
//  ===============================================================================
void compass_reset_calibration(SCompassCalib *g)
{
    g->compass_N = 0;
    g->Su = g->Sv = g->Sw = 0.0;
    g->Suu = g->Svv = g->Sww = g->Suv = g->Suw = g->Svw = 0.0;
    g->Suuu = g->Svvv = g->Swww = 0.0;
    g->Suuv = g->Suuw = g->Svvu = g->Svvw = g->Swwu = g->Swwv = 0.0;
    compass_CX_f = compass_CY_f = compass_CZ_f = 0.0;
}


//  ===============================================================================
//	compass_add_calibration
/// @brief	all the fancy stuff first implemented in OSTC3
///
/// input is compass_DX_f, compass_DY_f, compass_DZ_f
/// and compass_CX_f, compass_CY_f, compass_CZ_f
/// output is struct g
///
/// @param 	g: is a struct with crazy stuff like Suuu, Svvv, Svvu, etc.
//  ===============================================================================
void compass_add_calibration(SCompassCalib *g)
{
    float u, v, w;
   
    u = (compass_DX_f - compass_CX_f) / 32768.0f;
    v = (compass_DY_f - compass_CY_f) / 32768.0f;
    w = (compass_DZ_f - compass_CZ_f) / 32768.0f;

    g->compass_N++;
    g->Su += u;
    g->Sv += v;
    g->Sw += w;
    g->Suv += u*v;
    g->Suw += u*w;
    g->Svw += v*w;
    g->Suu  += u*u;
    g->Suuu += u*u*u;
    g->Suuv += v*u*u;
    g->Suuw += w*u*u;
    g->Svv  += v*v;
    g->Svvv += v*v*v;
    g->Svvu += u*v*v;
    g->Svvw += w*v*v;
    g->Sww  += w*w;
    g->Swww += w*w*w;
    g->Swwu += u*w*w;
    g->Swwv += v*w*w;
}

//////////////////////////////////////////////////////////////////////////////

//  ===============================================================================
//	compass_solve_calibration
/// @brief	all the fancy stuff first implemented in OSTC3
///
/// input is compass_CX_f, compass_CY_f, compass_CZ_f and g
/// output is struct g
///
/// @param 	g: is a struct with crazy stuff like Suuu, Svvv, Svvu, etc.
//  ===============================================================================
void compass_solve_calibration(SCompassCalib *g)
{
    float yu, yv, yw;
    float delta;
    float uc, vc, wc;
    

    //---- Normalize partial sums --------------------------------------------
    //
    // u, v, w should be centered on the mean value um, vm, wm:
    // x = u + um, with um = Sx/N
    //
    // So:
    // (u + um)**2 = u**2 + 2u*um + um**2
    // Su = 0, um = Sx/N
    // Sxx = Suu + 2 um Su + N*(Sx/N)**2 = Suu + Sx**2/N
    // Suu = Sxx - Sx**2/N
    yu = g->Su/g->compass_N;
    yv = g->Sv/g->compass_N;
    yw = g->Sw/g->compass_N;

    g->Suu -= g->Su*yu;
    g->Svv -= g->Sv*yv;
    g->Sww -= g->Sw*yw;

    // (u + um)(v + vm) = uv + u vm + v um + um vm
    // Sxy = Suv + N * um vm
    // Suv = Sxy - N * (Sx/N)(Sy/N);
    g->Suv -= g->Su*yv;
    g->Suw -= g->Su*yw;
    g->Svw -= g->Sv*yw;

    // (u + um)**3 = u**3 + 3 u**2 um + 3 u um**2 + um**3
    // Sxxx = Suuu + 3 um Suu + 3 um**2 Su + N.um**3
    // Su = 0, um = Sx/N:
    // Suuu = Sxxx - 3 Sx*Suu/N - N.(Sx/N)**3
    //      = Sxxx - 3 Sx*Suu/N - Sx**3/N**2

    // (u + um)**2 (v + vm) = (u**2 + 2 u um + um**2)(v + vm)
    // Sxxy = Suuv + vm Suu + 2 um (Suv + vm Su) + um**2 (Sv + N.vm)
    //
    // Su = 0, Sv = 0, vm = Sy/N:
    // Sxxy = Suuv + vm Suu + 2 um Suv + N um**2 vm
    //
    // Suuv = Sxxy - (Sy/N) Suu - 2 (Sx/N) Suv - (Sx/N)**2 Sy
    //      = Sxxy - Suu*Sy/N - 2 Suv*Sx/N - Sx*Sx*Sy/N/N
    //      = Sxxy - (Suu + Sx*Sx/N)*Sy/N - 2 Suv*Sx/N
    g->Suuu -= (3*g->Suu + g->Su*yu)*yu;
    g->Suuv -= (g->Suu + g->Su*yu)*yv + 2*g->Suv*yu;
    g->Suuw -= (g->Suu + g->Su*yu)*yw + 2*g->Suw*yu;

    g->Svvu -= (g->Svv + g->Sv*yv)*yu + 2*g->Suv*yv;
    g->Svvv -= (3*g->Svv + g->Sv*yv)*yv;
    g->Svvw -= (g->Svv + g->Sv*yv)*yw + 2*g->Svw*yv;

    g->Swwu -= (g->Sww + g->Sw*yw)*yu + 2*g->Suw*yw;
    g->Swwv -= (g->Sww + g->Sw*yw)*yv + 2*g->Svw*yw;
    g->Swww -= (3*g->Sww + g->Sw*yw)*yw;

    //---- Solve the system --------------------------------------------------
    // uc Suu + vc Suv + wc Suw = (Suuu + Svvu + Swwu) / 2
    // uc Suv + vc Svv + wc Svw = (Suuv + Svvv + Swwv) / 2
    // uc Suw + vc Svw + wc Sww = (Suuw + Svvw + Swww) / 2
    // Note this is symetric, with a positiv diagonal, hence
    // it always have a uniq solution.
    yu = 0.5f * (g->Suuu + g->Svvu + g->Swwu);
    yv = 0.5f * (g->Suuv + g->Svvv + g->Swwv);
    yw = 0.5f * (g->Suuw + g->Svvw + g->Swww);
    delta = g->Suu * (g->Svv * g->Sww - g->Svw * g->Svw)
          - g->Suv * (g->Suv * g->Sww - g->Svw * g->Suw)
          + g->Suw * (g->Suv * g->Svw - g->Svv * g->Suw);

    uc = (yu  * (g->Svv * g->Sww - g->Svw * g->Svw)
       -  yv  * (g->Suv * g->Sww - g->Svw * g->Suw)
       +  yw  * (g->Suv * g->Svw - g->Svv * g->Suw) )/delta;
    vc = (g->Suu * ( yv * g->Sww -  yw * g->Svw)
       -  g->Suv * ( yu * g->Sww -  yw * g->Suw)
       +  g->Suw * ( yu * g->Svw -  yv * g->Suw) )/delta;
    wc = (g->Suu * (g->Svv * yw  - g->Svw * yv )
       -  g->Suv * (g->Suv * yw  - g->Svw * yu )
       +  g->Suw * (g->Suv * yv  - g->Svv * yu ) )/delta;

    // Back to uncentered coordinates:
    // xc = um + uc
    uc = g->Su/g->compass_N + compass_CX_f/32768.0f + uc;
    vc = g->Sv/g->compass_N + compass_CY_f/32768.0f + vc;
    wc = g->Sw/g->compass_N + compass_CZ_f/32768.0f + wc;

    // Then save the new calibrated center:
    compass_CX_f = (short)(32768 * uc);
    compass_CY_f = (short)(32768 * vc);
    compass_CZ_f = (short)(32768 * wc);
}


//  ===============================================================================
//	compass_calib
/// @brief	the main loop for calibration
/// 				output is compass_CX_f, compass_CY_f, compass_CZ_f and g
///					160704 removed -4096 limit for LSM303D
///
/// @return always 0
//  ===============================================================================
int compass_calib_common(void)
{
		SCompassCalib g;

    // Starts with no calibration at all:
		compass_reset_calibration(&g);
	
		int64_t tickstart = 0;
		uint32_t ticks = 0; 
		uint32_t lasttick = 0;
		tickstart = HAL_GetTick();
    // Eine Minute kalibrieren
    while((ticks) < 60 * 1000)
    {
				compass_read();

				acceleration_read();
				compass_calc_roll_pitch_only();
			
				if((hardwareCompass == HMC5883L)
				&&((compass_DX_f == -4096) ||
					 (compass_DY_f == -4096) ||
					 (compass_DZ_f == -4096) ))
				{
					if(compass_gain == 0)
						return -1;
					compass_gain--;
				
					compass_init(1, compass_gain);
					compass_reset_calibration(&g);
					//tickstart = HAL_GetTick();
					continue;
				}

				copyCompassDataDuringCalibration(compass_DX_f,compass_DY_f,compass_DZ_f);
        compass_add_calibration(&g);
				HAL_Delay(1);
				lasttick = HAL_GetTick();
				if(lasttick == 0)
				{
					 tickstart = -ticks;
				}	
				ticks = lasttick - tickstart;
		}
        
    compass_solve_calibration(&g);
		
		tfull32 dataBlock[4];
		dataBlock[0].Word16.low16 = compass_CX_f;
		dataBlock[0].Word16.hi16 = compass_CY_f;
		dataBlock[1].Word16.low16 = compass_CZ_f;
		dataBlock[1].Word16.hi16 = 0xFFFF;
		dataBlock[2].Full32 = 0x7FFFFFFF;
		dataBlock[3].Full32 = 0x7FFFFFFF;
		BFA_writeDataBlock((uint32_t *)dataBlock);
	
		return 0;
}

// //////////////////////////// TEST CODE /////////////////////////////////////



//#include <QtDebug>
//#include <stdio.h>
//#include <math.h>
/*#include <stdlib.h>

short compass_DX_f, compass_DY_f, compass_DZ_f;
short compass_CX_f, compass_CY_f, compass_CZ_f;

inline float uniform(void) {
    return (rand() & 0xFFFF) / 65536.0f;
}
inline float sqr(float x) {
    return x*x;
}

static const float radius = 0.21f;
static const float cx = 0.79f, cy = -0.46f, cz = 0.24f;
// const float cx = 0, cy = 0, cz = 0;

float check_compass_calib(void)
{

    // Starts with no calibration at all:
    compass_CX_f = compass_CY_f = compass_CZ_f = 0;

    // Try 10 recalibration passes:
    for(int p=0; p<10; ++p)
    {
        compass_reset_calibration();

        //---- Generates random points on a sphere -------------------------------
        // of radius,center (cx, cy, cz):
        for(int i=0; i<100; ++i)
        {
            float theta = uniform()*360.0f;
            float phi   = uniform()*180.0f - 90.0f;

            float x = cx + radius * cosf(phi)*cosf(theta);
            float y = cy + radius * cosf(phi)*sinf(theta);
            float z = cz + radius * sinf(phi);

            compass_DX_f = (short)(32768 * x);
            compass_DY_f = (short)(32768 * y);
            compass_DZ_f = (short)(32768 * z);
            compass_add_calibration();
        }

        compass_solve_calibration();
        //qDebug() << "Center ="
//                 << compass_CX_f/32768.0f
//                 << compass_CY_f/32768.0f
//                 << compass_CZ_f/32768.0f;

        float r2 = sqr(compass_CX_f/32768.0f - cx)
                 + sqr(compass_CY_f/32768.0f - cy)
                 + sqr(compass_CZ_f/32768.0f - cz);
        if( r2 > 0.01f*0.01f )
            return sqrtf(r2);
    }
		return 0;
}*/



/*
void compass_read_LSM303D_v3(void)
{
  uint8_t data;
	
  memset(magDataBuffer,0,6);

	compass_DX_f = 0;
	compass_DY_f = 0;
	compass_DZ_f = 0;
	
	//magnetometer multi read, order xl,xh, yl,yh, zl, zh
	data = REG_MAG_DATA_ADDR;
	I2C_Master_Transmit( DEVICE_COMPASS_303D, &data, 1);
	I2C_Master_Receive(  DEVICE_COMPASS_303D, magDataBuffer, 6);

	compass_DX_f = ((int16_t)( (int16_t)((magDataBuffer[1] << 8) | (magDataBuffer[0]))));
	compass_DY_f = ((int16_t)( (int16_t)((magDataBuffer[3] << 8) | (magDataBuffer[2]))));
	compass_DZ_f = ((int16_t)( (int16_t)((magDataBuffer[5] << 8) | (magDataBuffer[4]))));	

//	compass_DX_f = compass_DX_f * stat->sensitivity_mag;
//	compass_DY_f = compass_DY_f * stat->sensitivity_mag;
//	compass_DZ_f = compass_DZ_f * stat->sensitivity_mag;
}


//  ===============================================================================
//	compass_init_LSM303D by STMicroelectronics 2013 V1.0.5 2013/Oct/23
/// @brief	The new ST 303D 
///					This might be called several times with different gain values during calibration
///
/// @param 	gain: 7 is max gain and set with here, compass_calib() might reduce it
//  ===============================================================================

void compass_init_LSM303D_v3(uint8_t gain)
{
  uint8_t data[10];

	// CNTRL1
	// 0011 acceleration data rate 0011 = 12.5 Hz (3.125 Hz - 1600 Hz)
	// 0xxx block data update off
	// x111 enable all three axes 
	
	// CNTRL5
	// 0xxx xxxx temp sensor off
	// x00x xxxx magnetic resolution
	// xxx0 1xxx magentic data rate 01 = 6,25 Hz (3.125 Hz - 50 Hz (100 Hz))
	// xxxx xx00 latch irq requests off
	
	// CNTRL7
	// 00xx high pass filter mode, 00 normal mode
	// xx0x filter for acceleration data bypassed
	// xxx0 temperature sensor mode only off
	// x0xx magnetic data low-power mode off
	// xx00 magnetic sensor mode 00 = continous-conversion mode (default 10 power-down)
	
	data[0] = CNTRL0;
	data[1] = 0x00;
	I2C_Master_Transmit( DEVICE_COMPASS_303D, data, 2);

	// acc
	data[0] = CNTRL1;
	data[1] = 0x00;
	data[2] = 0x0F;
	data[3] = 0x00;
	data[4] = 0x00;
	I2C_Master_Transmit( DEVICE_COMPASS_303D, data, 5);

	// mag
	data[0] = CNTRL3;
	data[1] = 0x00;
	data[2] = 0x00;
	data[3] = 0x18;
	data[4] = 0x20;
	I2C_Master_Transmit( DEVICE_COMPASS_303D, data, 5);
	
	data[0] = CNTRL7;
	data[1] = ((MSMS_MASK & CONTINUOS_CONVERSION) |
			((~MSMS_MASK) & CNTRL7_RESUME_VALUE));
	I2C_Master_Transmit( DEVICE_COMPASS_303D, data, 2);
	
	HAL_Delay(100);
}


//  ===============================================================================
//	compass_init_LSM303D by nordevx for arduion
/// @brief	The new ST 303D 
///					This might be called several times with different gain values during calibration
///
/// @param 	gain: 7 is max gain and set with here, compass_calib() might reduce it
//  ===============================================================================
void compass_init_LSM303D_v2(uint8_t gain)
{
  uint8_t data[2];

	// CNTRL1
	// 0011 acceleration data rate 0011 = 12.5 Hz (3.125 Hz - 1600 Hz)
	// 0xxx block data update off
	// x111 enable all three axes 
	
	// CNTRL5
	// 0xxx xxxx temp sensor off
	// x00x xxxx magnetic resolution
	// xxx0 1xxx magentic data rate 01 = 6,25 Hz (3.125 Hz - 50 Hz (100 Hz))
	// xxxx xx00 latch irq requests off
	
	// CNTRL7
	// 00xx high pass filter mode, 00 normal mode
	// xx0x filter for acceleration data bypassed
	// xxx0 temperature sensor mode only off
	// x0xx magnetic data low-power mode off
	// xx00 magnetic sensor mode 00 = continous-conversion mode (default 10 power-down)
	
	data[0] = CNTRL1;
	data[1] = 0x37; //0b 0011 0111 
	I2C_Master_Transmit( DEVICE_COMPASS_303D, data, 2);
	
	data[0] = CNTRL5;
	data[1] = 0x08; // 0b 0000 1000
	I2C_Master_Transmit( DEVICE_COMPASS_303D, data, 2);
	
	data[0] = CNTRL7;
	data[1] = 0x00; // 0b 0000 0000
	I2C_Master_Transmit( DEVICE_COMPASS_303D, data, 2);
	
	HAL_Delay(100);
}


//  ===============================================================================
//	compass_init_LSM303D_v1 by ST lsm303d.c
/// @brief	The new ST 303D 
///					This might be called several times with different gain values during calibration
///
/// @param 	gain: 7 is max gain and set with here, compass_calib() might reduce it
//  ===============================================================================
void compass_init_LSM303D_v1(uint8_t gain)
{
  uint8_t data;

	compass_gain = gain;

  memset(magDataBuffer,0,6);
  memset(accDataBuffer,0,6);

	data = CNTRL5;
	I2C_Master_Transmit( DEVICE_COMPASS_303D, &data, 1);
	I2C_Master_Receive(  DEVICE_COMPASS_303D, &data, 1);
  data = (data & 0x1c) >> 2;  
  velMag = magODR[data];  

	data = CNTRL1;
	I2C_Master_Transmit( DEVICE_COMPASS_303D, &data, 1);
	I2C_Master_Receive(  DEVICE_COMPASS_303D, &data, 1);
  data = (data & 0xf0) >> 4;  
  velAcc = accODR[data];
  
	data = CNTRL7;
	I2C_Master_Transmit( DEVICE_COMPASS_303D, &data,   1);
	I2C_Master_Receive(  DEVICE_COMPASS_303D, &datas1, 1);
  datas1 = (datas1 & 0x02);
  
  //if mag is not pd
  //mag is bigger than gyro
  if( (velMag < velAcc) || datas1 != 0 ) {
	//acc is the biggest
	fastest = ACC_IS_FASTEST;	
  }	
  else {		
	//acc is the biggest		
	fastest = MAG_IS_FASTEST;
  }
  
  zoffFlag = 1;
	
  if( fastest == MAG_IS_FASTEST)
	{	
		data = STATUS_REG_M;
		I2C_Master_Transmit( DEVICE_COMPASS_303D, &data, 1);
		I2C_Master_Receive(  DEVICE_COMPASS_303D, &data, 1);
 
//    if(ValBit(data, ZYXMDA)) {
      sendFlag = 1;      
//    }
    
  }  
  else if(fastest == ACC_IS_FASTEST)
	{
		data = STATUS_REG_A;
		I2C_Master_Transmit( DEVICE_COMPASS_303D, &data, 1);
		I2C_Master_Receive(  DEVICE_COMPASS_303D, &data, 1);
//      if(ValBit(data, DATAREADY_BIT)) {
        sendFlag = 1;
//      }    
  }  
}

//  ===============================================================================
//	compass_read_LSM303D
/// @brief	The new LSM303D :-)
///
/// output is compass_DX_f, compass_DY_f, compass_DZ_f, accel_DX_f, accel_DY_f, accel_DZ_f
//  ===============================================================================
void compass_read_LSM303D_v2(void)
{
	  uint8_t data;
	
  memset(magDataBuffer,0,6);
  memset(accDataBuffer,0,6);

	compass_DX_f = 0;
	compass_DY_f = 0;
	compass_DZ_f = 0;

	accel_DX_f = 0;
	accel_DY_f = 0;
	accel_DZ_f = 0;
	
	//Accelerometer multi read, order xl,xh, yl,yh, zl, zh
	data = REG_ACC_DATA_ADDR;
	I2C_Master_Transmit( DEVICE_COMPASS_303D, &data, 1);
	I2C_Master_Receive(  DEVICE_COMPASS_303D, accDataBuffer, 6);
	
	//magnetometer multi read, order xl,xh, yl,yh, zl, zh
	data = OUT_X_L_M;
	I2C_Master_Transmit( DEVICE_COMPASS_303D, &data, 1);
	I2C_Master_Receive(  DEVICE_COMPASS_303D, magDataBuffer, 6);
	
	accel_DX_f = ((int16_t)( (int16_t)((accDataBuffer[1] << 8) | (accDataBuffer[0]))));
	accel_DY_f = ((int16_t)( (int16_t)((accDataBuffer[3] << 8) | (accDataBuffer[2]))));
	accel_DZ_f = ((int16_t)( (int16_t)((accDataBuffer[5] << 8) | (accDataBuffer[4]))));	

//	accel_DX_f = accel_DX_f * stat->sensitivity_acc;
//	accel_DY_f = accel_DY_f * stat->sensitivity_acc;
//	accel_DZ_f = accel_DZ_f * stat->sensitivity_acc;


	compass_DX_f  = magDataBuffer[1];
	compass_DX_f *= 256;
	compass_DX_f += magDataBuffer[0];

	compass_DY_f  = magDataBuffer[3];
	compass_DY_f *= 256;
	compass_DY_f += magDataBuffer[2];

	compass_DY_f  = magDataBuffer[5];
	compass_DY_f *= 256;
	compass_DY_f += magDataBuffer[4];

}


*/


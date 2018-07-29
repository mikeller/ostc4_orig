/*	getrennte Gase f�r die verschiedenen Modi
		um Gaswechsel Eintr�ge zu vereinfachen
		das heisst:
		oc == bailout in cc mode
*/

/* Konvention:
float extExample_variable_can_be_used_with_extern;
*/

#include <string.h>
//#include "arm_math.h"
#include <math.h>
#include <stdbool.h>
#include "buehlmann.h"
#include "decom.h"


extern  const float helium_time_constant[16];
extern  const float nitrogen_time_constant[16];

extern const float buehlmann_N2_a[];
extern const float buehlmann_N2_b[];
extern const float buehlmann_He_a[];
extern const float buehlmann_He_b[];


/*
typedef struct
{
	float *pointer_array_tissue_nitrogen_bar;
	float *pointer_array_tissue_helium_bar;
	char gf_value;

	float output_ceiling_ambient_bar_or_input;
	_Bool output_ceiling_tolerated_if_ceiling_used_as_input;
}		tissue_test_tolerance_struct;
*/
typedef struct
{
	float depth;
	int id;
} SStop;

#define DECO_STOPS_MAX_TTS_CALCULATON_IN_SECONDS 59940 // 999 minuten; before: 18000 // 5(h) * 60(min) * 60 sec = 18000 sec
#define DECO_STOPS_MAX_TTS_FOR_EVERY_SECOND_CALC_IN_SECONDS 7200
#define NINETY_NINE_MINUTES_IN_SECONDS 59940

# define PRESSURE_TEN_METER 1.0f
# define PRESSURE_THREE_METER 0.333334f
# define PRESSURE_150_CM 0.15f
# define PRESSURE_HALF_METER 0.05f
/*
# define PRESSURE_150_CM_MBAR 150
# define PRESSURE_TWO_M_MBAR 200
# define PRESSURE_FIVE_M_MBAR 500
# define PRESSURE_TEN_M_MBAR 1000
# define PRESSURE_120_METER 12.0
*/
/*
_____________________________________________________________
*/


void buehlmann_backup_and_restore(_Bool backup_restore_otherwise);
float tissue_tolerance(void);
void ambient_bar_to_deco_stop_depth_bar(float ceiling);
int ascend_with_all_gaschanges(float pressure_decrease);
float next_stop_depth_input_is_actual_stop_id(int actual_id);
float get_gf_at_pressure(float pressure);
void buehlmann_calc_ndl(void);
_Bool dive1_check_deco(void);
uint8_t buehlmann_tissue_test_tolerance(float depth_in_bar_absolute);

/*
_____________________________________________________________
*/

SDecoinfo gDecotable;
float gSurface_pressure_bar;

float gPressure;
int gGas_id;
float gTTS;
float gTissue_nitrogen_bar[16];
float gTissue_helium_bar[16];
float	gGF_value;
float gCNS;
//float gMax_ceiling_bar = 0;
int gNDL;


//SLifeData *pLifeData;
SDiveSettings *pBuDiveSettings;
SDecoinfo* pDecolistBuehlmann;
//signed char gGaschange_decreasing_depth_gas_id[BUEHLMANN_STRUCT_MAX_GASES];
float gGF_low_depth_bar;
SStop gStop;

void buehlmann_init(void)
{
  //gMax_ceiling_bar = 0;
}

void buehlmann_backup_and_restore(_Bool backup_restore_otherwise)
{
	static float pressure;
	static float gas_id;
	static float tts;
	static float tissue_nitrogen_bar[16];
	static float tissue_helium_bar[16];
	static float gf_value;
	static int ndl;
	static float cns;

	if(backup_restore_otherwise)
	{
		pressure = gPressure;
		gas_id = gGas_id;
		tts = gTTS;
		gf_value = gGF_value;
		ndl = gNDL;
		cns = gCNS;
		memcpy(tissue_nitrogen_bar, gTissue_nitrogen_bar, (4*16));
		memcpy(tissue_helium_bar, gTissue_helium_bar, (4*16));
	}
	else
	{
		gPressure = pressure;
		gGas_id = gas_id;
		gTTS = tts;
		gGF_value = gf_value;
		gNDL = ndl;
		gCNS = cns;
		memcpy(gTissue_nitrogen_bar, tissue_nitrogen_bar, (4*16));
		memcpy(gTissue_helium_bar, tissue_helium_bar, (4*16));
	}

}
/*void buehlmann__test__saturate_tissues(SBuehlmann *pInput,  int seconds)
{
	pBuehlmann = pInput;
	pInput->dive_time_seconds += seconds;
	// internal copying
	gSurface_pressure_bar = pBuehlmann->pressure_surface_bar;

	gPressure = pBuehlmann->pressure_ambient_bar;
	gGas_id = pBuehlmann->actual_gas_id;
	memcpy(gTissue_nitrogen_bar, pBuehlmann->tissue_nitrogen_bar, (4*16));
	memcpy(gTissue_helium_bar, pBuehlmann->tissue_helium_bar, (4*16));

	tissues_exposure_at_gPressure_seconds(seconds);

	memcpy(pBuehlmann->tissue_nitrogen_bar, gTissue_nitrogen_bar, (4*16));
	memcpy(pBuehlmann->tissue_helium_bar, gTissue_helium_bar, (4*16));
}*/

float buehlmann_get_gCNS(void)
{
	return gCNS;
}

void buehlmann_calc_deco(SLifeData* pLifeData, SDiveSettings * pDiveSettings, SDecoinfo * pDecoInfo)
{
	float ceiling;
	int ascend_time;
	int tts_seconds;
	float pressure_delta;
	float next_depth;
	_Bool deco_reached = false;
//	tissue_test_tolerance_struct tolerance_data;
	unsigned short *stoplist;
	int i;
	
   // decom_CreateGasChangeList(pDiveSettings, pLifeData);

	gCNS = 0;
  pDecoInfo->output_time_to_surface_seconds = 0;
	pDecoInfo->output_ndl_seconds = 0;
	for(int i=0;i<DECOINFO_STRUCT_MAX_STOPS;i++)
	{
		pDecoInfo->output_stop_length_seconds[i] = 0;
	}
	/* make input available global*/
	pBuDiveSettings = pDiveSettings;

  pDecolistBuehlmann = pDecoInfo;
	/* internal copying */
	gSurface_pressure_bar = pLifeData->pressure_surface_bar;

	gPressure = pLifeData->pressure_ambient_bar;
	gGas_id = 0;
	memcpy(gTissue_nitrogen_bar, pLifeData->tissue_nitrogen_bar, (4*16));
	memcpy(gTissue_helium_bar, pLifeData->tissue_helium_bar, (4*16));
	gGF_value = ((float)pBuDiveSettings->gf_low) / 100.0f;
	
	//
	memcpy(&gDecotable, pDecolistBuehlmann, sizeof(SDecoinfo));
	stoplist = gDecotable.output_stop_length_seconds;


  if(pLifeData->dive_time_seconds < 60)
    return;
	/* coupling */

	/* functions */

	// clean stop list
	for(i = 0; i < DECOINFO_STRUCT_MAX_STOPS; i++)
		stoplist[i] = 0;
	gTTS = 0;
	gNDL = 0;

  if(pDiveSettings->internal__pressure_first_stop_ambient_bar_as_upper_limit_for_gf_low_otherwise_zero >= (gPressure - PRESSURE_150_CM))
  {
    deco_reached = true;
  }


//ascend_with_all_gaschanges(gPressure - gSurface_pressure_bar);
	gGF_value = ((float)pBuDiveSettings->gf_high) / 100.0f;
	//iling = tissue_tolerance();
	// includes backup for gGF_value
	// NDL
	buehlmann_backup_and_restore(true); // includes backup for gGF_value
	if(!dive1_check_deco() )
	{
	  buehlmann_backup_and_restore(false);
		// no deco
		pDecolistBuehlmann->output_time_to_surface_seconds = 0;
		for(i = 0; i < DECOINFO_STRUCT_MAX_STOPS; i++)
			pDecolistBuehlmann->output_stop_length_seconds[i] = 0;
		// calc NDL
		buehlmann_calc_ndl();
		pDecolistBuehlmann->output_ndl_seconds = gNDL;
		return;
	}
	buehlmann_backup_and_restore(false);
	pDecolistBuehlmann->output_ndl_seconds = 0;

  gGF_value = get_gf_at_pressure(gPressure);
	//current ceiling at actual position
	ceiling = tissue_tolerance();
	//if(ceiling < pDiveSettings->internal__pressure_first_stop_ambient_bar_as_upper_limit_for_gf_low_otherwise_zero)
		//ambient_bar_to_deco_stop_depth_bar(pDiveSettings->internal__pressure_first_stop_ambient_bar_as_upper_limit_for_gf_low_otherwise_zero);
	//else
  ambient_bar_to_deco_stop_depth_bar(ceiling);

	// set the base for all upcoming parameters
	ceiling = gStop.depth + gSurface_pressure_bar;
	tts_seconds = 0;

	// modify parameters if there is ascend or parameter fine adjustment
	if(ceiling < (gPressure - PRESSURE_150_CM)) // more than 1.5 meter below ceiling
	{
		// ascend within 10 mtr to GF_low // speed 12 mtr/min -> 50 sec / 10 mtr;  15 sec / 3 mtr.
		if(ceiling < (gPressure - PRESSURE_TEN_METER) )
		{ do {
				ascend_time = ascend_with_all_gaschanges(PRESSURE_TEN_METER);
				tts_seconds += ascend_time;
				ceiling = tissue_tolerance();
				if(tts_seconds > DECO_STOPS_MAX_TTS_CALCULATON_IN_SECONDS)
			{
				/* pInput == pBuehlmann */
				pDecolistBuehlmann->output_time_to_surface_seconds = NINETY_NINE_MINUTES_IN_SECONDS;
				return;// NINETY_NINE_MINUTES_IN_SECONDS;
			}
			} while ((ascend_time > 0 ) && ((gPressure - PRESSURE_TEN_METER ) > gSurface_pressure_bar) && (ceiling < (gPressure - PRESSURE_TEN_METER)));
		}
		do {
			buehlmann_backup_and_restore(true);
			ascend_time = ascend_with_all_gaschanges(PRESSURE_THREE_METER);
			tts_seconds += ascend_time;
			ceiling = tissue_tolerance();
			if(tts_seconds > DECO_STOPS_MAX_TTS_CALCULATON_IN_SECONDS)
			{
				/* pInput == pBuehlmann */
				pDecolistBuehlmann->output_time_to_surface_seconds = NINETY_NINE_MINUTES_IN_SECONDS;
				return;// NINETY_NINE_MINUTES_IN_SECONDS;
			}
			ambient_bar_to_deco_stop_depth_bar(ceiling);
		} while ((ascend_time > 0 ) &&  ((gStop.depth + gSurface_pressure_bar) < gPressure));

		if(gStop.depth + gSurface_pressure_bar > gPressure)
		{
			gPressure += PRESSURE_THREE_METER;
			buehlmann_backup_and_restore(false);
			tts_seconds -= ascend_time;
		}
    // calculate first stop based on tissue saturation within 10 meters of stop
    //ambient_bar_to_deco_stop_depth_bar(ceiling);
	}
	else
	{
		// initial values, upper code might not be executed (is within 150 cm)
	}




  if(ceiling > gSurface_pressure_bar)
  {

    ceiling = gStop.depth + gSurface_pressure_bar;
    // ascend the last meters to first stop (especially consider any gas changes around)
    pressure_delta = gPressure - ceiling;
    ascend_time = (int)ceil(pressure_delta * 50.0f);
    tts_seconds += ascend_with_all_gaschanges(pressure_delta);
  }
	// NDL check
	if(ceiling <= gSurface_pressure_bar)
	{
		/* pInput == pBuehlmann  same pointer*/
		// NDL with GF_low
		pDecolistBuehlmann->output_time_to_surface_seconds = 0;
		return;
	}
  if(ceiling >pDiveSettings->internal__pressure_first_stop_ambient_bar_as_upper_limit_for_gf_low_otherwise_zero)
    pDiveSettings->internal__pressure_first_stop_ambient_bar_as_upper_limit_for_gf_low_otherwise_zero = ceiling;

	// calc gf loop
	if(deco_reached)
		gGF_low_depth_bar = pDiveSettings->internal__pressure_first_stop_ambient_bar_as_upper_limit_for_gf_low_otherwise_zero - gSurface_pressure_bar;
	else
		gGF_low_depth_bar = ceiling - gSurface_pressure_bar;

	while(gStop.depth > 0)
	{
		do
		{
			next_depth = next_stop_depth_input_is_actual_stop_id(gStop.id);
      gGF_value = get_gf_at_pressure(next_depth + gSurface_pressure_bar);
			buehlmann_backup_and_restore(true);
			ascend_time = ascend_with_all_gaschanges(gStop.depth - next_depth);
			ceiling = tissue_tolerance();
			/* pre check actual limit */
			if(gDecotable.output_stop_length_seconds[gStop.id] >= 999*60)
			{
				tts_seconds -= 999*60 - gDecotable.output_stop_length_seconds[gStop.id];
				gDecotable.output_stop_length_seconds[gStop.id] = 999*60;
			}
			else
			/* more deco on the actual depth */
			if(ceiling > next_depth + gSurface_pressure_bar)
			{
				next_depth = -1;
				buehlmann_backup_and_restore(false);
				decom_tissues_exposure2(10, &pBuDiveSettings->decogaslist[gGas_id], gPressure,gTissue_nitrogen_bar,gTissue_helium_bar); // some seconds at least at each stop
				decom_oxygen_calculate_cns_exposure(10, &pBuDiveSettings->decogaslist[gGas_id], gPressure, &gCNS);
        gDecotable.output_stop_length_seconds[gStop.id] += 10;
        tts_seconds += 10;
			}
		} while(next_depth == -1);
    tts_seconds += ascend_time;
		gStop.depth = next_depth;
    for(i = gGas_id + 1; i < BUEHLMANN_STRUCT_MAX_GASES; i++)
    {
        if(pBuDiveSettings->decogaslist[i].change_during_ascent_depth_meter_otherwise_zero == 0)
            break;
        float pressureChange =  ((float)pBuDiveSettings->decogaslist[i].change_during_ascent_depth_meter_otherwise_zero) / 10;
        if(gStop.depth <= pressureChange + 0.00001f)
        {
            gGas_id = i;
        }
        else
        {
            break;
        }
    }
		gStop.id--;
	}

  gDecotable.output_time_to_surface_seconds = tts_seconds;
	memcpy(pDecolistBuehlmann, &gDecotable, sizeof(SDecoinfo));
}


float tissue_tolerance(void)
{
	float tissue_inertgas_saturation;
	float inertgas_a;
	float inertgas_b;
	float ceiling;
	float global_ceiling;
	int ci;

	global_ceiling = -1;

	for (ci = 0; ci < 16; ci++)
	{
		if(gTissue_helium_bar[ci] == 0)
		{
			tissue_inertgas_saturation = gTissue_nitrogen_bar[ci];
			//
			inertgas_a = buehlmann_N2_a[ci];
			inertgas_b = buehlmann_N2_b[ci];
		}
		else
		{
			tissue_inertgas_saturation =  gTissue_nitrogen_bar[ci] + gTissue_helium_bar[ci];
			//
			inertgas_a = ( ( buehlmann_N2_a[ci] *  gTissue_nitrogen_bar[ci]) + ( buehlmann_He_a[ci] * gTissue_helium_bar[ci]) ) / tissue_inertgas_saturation;
			inertgas_b = ( ( buehlmann_N2_b[ci] *  gTissue_nitrogen_bar[ci]) + ( buehlmann_He_b[ci] * gTissue_helium_bar[ci]) ) / tissue_inertgas_saturation;
		}
		//
		ceiling = (inertgas_b * ( tissue_inertgas_saturation - gGF_value * inertgas_a ) ) / (gGF_value - (inertgas_b * gGF_value) + inertgas_b);
		if(ceiling > global_ceiling)
			global_ceiling = ceiling;
	}
	return global_ceiling;
}

// hw 161121 for relative gradient
float tissue_tolerance_without_gf_correction(float *tissue_inertgas_saturation_output)
{
	float tissue_inertgas_saturation;
	float inertgas_a;
	float inertgas_b;
	float ceiling;
	float global_ceiling;
	int ci;

	global_ceiling = -1;

	for (ci = 0; ci < 16; ci++)
	{
		if(gTissue_helium_bar[ci] == 0)
		{
			tissue_inertgas_saturation = gTissue_nitrogen_bar[ci];
			//
			inertgas_a = buehlmann_N2_a[ci];
			inertgas_b = buehlmann_N2_b[ci];
		}
		else
		{
			tissue_inertgas_saturation =  gTissue_nitrogen_bar[ci] + gTissue_helium_bar[ci];
			//
			inertgas_a = ( ( buehlmann_N2_a[ci] *  gTissue_nitrogen_bar[ci]) + ( buehlmann_He_a[ci] * gTissue_helium_bar[ci]) ) / tissue_inertgas_saturation;
			inertgas_b = ( ( buehlmann_N2_b[ci] *  gTissue_nitrogen_bar[ci]) + ( buehlmann_He_b[ci] * gTissue_helium_bar[ci]) ) / tissue_inertgas_saturation;
		}
		//
		ceiling = inertgas_b * ( tissue_inertgas_saturation - inertgas_a );
		if(ceiling > global_ceiling)
		{
			global_ceiling = ceiling;
			if(tissue_inertgas_saturation_output)
			{
				*tissue_inertgas_saturation_output = tissue_inertgas_saturation;
			}
		}
	}
	return global_ceiling;
}


uint8_t buehlmann_tissue_test_tolerance(float depth_in_bar_absolute)
{
	float tissue_inertgas_saturation;
	float inertgas_a;
	float inertgas_b;
	float inertgas_tolerance;
	float gf_minus_1;

	gf_minus_1 = gGF_value - 1.0f;

	for (int ci = 0; ci < 16; ci++)
	{
		if(gTissue_helium_bar[ci] == 0)
		{
			tissue_inertgas_saturation = gTissue_nitrogen_bar[ci];
			//
			inertgas_a = buehlmann_N2_a[ci];
			inertgas_b = buehlmann_N2_b[ci];
		}
		else
		{
			tissue_inertgas_saturation =  gTissue_nitrogen_bar[ci] + gTissue_helium_bar[ci];
			//
			inertgas_a = ( ( buehlmann_N2_a[ci] *  gTissue_nitrogen_bar[ci]) + ( buehlmann_He_a[ci] * gTissue_helium_bar[ci]) ) / tissue_inertgas_saturation;
			inertgas_b = ( ( buehlmann_N2_b[ci] *  gTissue_nitrogen_bar[ci]) + ( buehlmann_He_b[ci] * gTissue_helium_bar[ci]) ) / tissue_inertgas_saturation;
		}
		//
		inertgas_tolerance = ( (gGF_value / inertgas_b - gf_minus_1) * depth_in_bar_absolute ) + ( gGF_value * inertgas_a );
		//
		if(inertgas_tolerance < tissue_inertgas_saturation)
			return 0;
	}
	return 1;
}


void ambient_bar_to_deco_stop_depth_bar(float ceiling)
{
	int i;

	ceiling -= gSurface_pressure_bar;

	if(ceiling <= 0)
	{
		gStop.depth = pBuDiveSettings->last_stop_depth_bar;
		gStop.id = 0;
		return;
	}


    //for(int i = 1; i < 10; i++)

	if((ceiling -  pBuDiveSettings->last_stop_depth_bar) <= 0)
	{
		gStop.depth =  pBuDiveSettings->last_stop_depth_bar;
		gStop.id = 0;
		return;
	}

	gStop.depth = pBuDiveSettings->input_second_to_last_stop_depth_bar;
	gStop.id = 1;
	ceiling -= pBuDiveSettings->input_second_to_last_stop_depth_bar;

	if(ceiling <= 0)
		return;

	for(i = 1; i < (DECOINFO_STRUCT_MAX_STOPS - 2); i++)
	{
		ceiling -= pBuDiveSettings->input_next_stop_increment_depth_bar;
		if(ceiling <= 0)
			break;
	}
	gStop.depth += i * pBuDiveSettings->input_next_stop_increment_depth_bar;
	gStop.id += i;
	return;
}

float next_stop_depth_input_is_actual_stop_id(int actual_id)
{
	if(actual_id == 0)
		return 0;

	if(actual_id == 1)
		return pBuDiveSettings->last_stop_depth_bar;

	actual_id -= 2;
	return pBuDiveSettings->input_second_to_last_stop_depth_bar + (actual_id * pBuDiveSettings->input_next_stop_increment_depth_bar);
}

int ascend_with_all_gaschanges(float pressure_decrease)
{
	float pressureTop, pressureTop_tmp, pressureBottom, pressureChange, ascendrate_in_seconds_for_one_bar, pressure_difference;
	int time_for_ascend;
	int seconds;
	int i;

	ascendrate_in_seconds_for_one_bar = 60 * 10 / pBuDiveSettings->ascentRate_meterperminute;

	if(fabsf(gPressure - gSurface_pressure_bar) < PRESSURE_HALF_METER)
	{
		gPressure = gSurface_pressure_bar;
		return 0;
	}

	pressureTop = gPressure - pressure_decrease;
	if( gSurface_pressure_bar > pressureTop)
		pressureTop = gSurface_pressure_bar;
	pressureBottom = gPressure;
	seconds = 0;
    do{
        pressureTop_tmp = pressureTop;
        for(i = gGas_id + 1; i < BUEHLMANN_STRUCT_MAX_GASES; i++)
        {
            if(pBuDiveSettings->decogaslist[i].change_during_ascent_depth_meter_otherwise_zero == 0)
                break;
            pressureChange = gSurface_pressure_bar + ((float)pBuDiveSettings->decogaslist[i].change_during_ascent_depth_meter_otherwise_zero) / 10;
            if(pressureBottom <= pressureChange)
            {
                gGas_id = i;
            }
            else
            {
                 break;
            }

        }
        for(i = gGas_id + 1; i < BUEHLMANN_STRUCT_MAX_GASES; i++)
        {
            if(pBuDiveSettings->decogaslist[i].change_during_ascent_depth_meter_otherwise_zero == 0)
                break;
            pressureChange = gSurface_pressure_bar + ((float)pBuDiveSettings->decogaslist[i].change_during_ascent_depth_meter_otherwise_zero)/ 10;
            if((pressureChange < pressureBottom) && (pressureChange > pressureTop))
            {
                pressureTop_tmp = pressureChange;
            }
        }
        pressure_difference = pressureBottom - pressureTop_tmp;
        if(pressure_difference > 0.0001f)
        {
          time_for_ascend = (int)ceilf(pressure_difference * ascendrate_in_seconds_for_one_bar);
          decom_tissues_exposure_stage_schreiner(time_for_ascend, &pBuDiveSettings->decogaslist[gGas_id],
                                              pressureBottom, pressureTop_tmp, gTissue_nitrogen_bar, gTissue_helium_bar);
					decom_oxygen_calculate_cns_stage_SchreinerStyle(time_for_ascend,&pBuDiveSettings->decogaslist[gGas_id],
                                              pressureBottom, pressureTop_tmp, &gCNS);
        }
        pressureBottom = pressureTop_tmp;
        seconds += time_for_ascend;
    }while(pressureTop_tmp > pressureTop);
    gPressure = pressureTop;
	return seconds;
}


float get_gf_at_pressure(float pressure)
{
	float gfSteigung = 0.0f;

	if(gGF_low_depth_bar < 0)
			gGF_low_depth_bar = PRESSURE_THREE_METER; // just to prevent erratic behaviour if variable is not set

    gfSteigung = ((float)(pBuDiveSettings->gf_high - pBuDiveSettings->gf_low))/ gGF_low_depth_bar;


	if((pressure - gSurface_pressure_bar) <= PRESSURE_HALF_METER)
		return ((float)pBuDiveSettings->gf_high) / 100.0f;

	if(pressure >= gSurface_pressure_bar + gGF_low_depth_bar)
		return ((float)pBuDiveSettings->gf_low) / 100.0f;

	return (pBuDiveSettings->gf_high - gfSteigung * (pressure - gSurface_pressure_bar) )/ 100.0f;
}


void buehlmann_calc_ndl(void)
{
	float local_tissue_nitrogen_bar[16];
	float local_tissue_helium_bar[16];
	int i;

	gNDL = 0;
	//Check ndl always use gHigh
  gGF_value = ((float)pBuDiveSettings->gf_high) / 100.0f;
	//10 minutes steps
	while(gNDL < (300 * 60))
	{
		memcpy(local_tissue_nitrogen_bar, gTissue_nitrogen_bar, (4*16));
		memcpy(local_tissue_helium_bar, gTissue_helium_bar, (4*16));
		//
		gNDL += 600;
    decom_tissues_exposure2(600, &pBuDiveSettings->decogaslist[gGas_id], gPressure,gTissue_nitrogen_bar,gTissue_helium_bar);
		decom_oxygen_calculate_cns_exposure(600,&pBuDiveSettings->decogaslist[gGas_id],gPressure,&gCNS);
		//tissues_exposure_at_gPressure_seconds(600);
		buehlmann_backup_and_restore(true);
		if(dive1_check_deco() == true)
		{
			buehlmann_backup_and_restore(false);
			break;
		}
		buehlmann_backup_and_restore(false);
	}

	if(gNDL < (300 * 60))
		gNDL -= 600;

	if(gNDL > (150 * 60))
		return;

	// refine
	memcpy(gTissue_nitrogen_bar, local_tissue_nitrogen_bar, (4*16));
	memcpy(gTissue_helium_bar, local_tissue_helium_bar, (4*16));

	//One minutes step
	for(i = 0; i < 20; i++)
	{
		gNDL += 60;
		//tissues_exposure_at_gPressure_seconds(60);
		decom_tissues_exposure2(60, &pBuDiveSettings->decogaslist[gGas_id], gPressure,gTissue_nitrogen_bar,gTissue_helium_bar);
		decom_oxygen_calculate_cns_exposure(60,&pBuDiveSettings->decogaslist[gGas_id],gPressure,&gCNS);
		buehlmann_backup_and_restore(true);
		if(dive1_check_deco() == true)
			break;
		buehlmann_backup_and_restore(false);
	}
	//gNDL -= 60;
	return;
}


//  ===============================================================================
//	dive1_check_deco
/// @brief	for NDL calculations
///					160614 using ceilingOther and not ceiling
//  ===============================================================================
_Bool dive1_check_deco(void)
{
	// gGF_value is set in call routine;
	// internes Backup!

	// calc like in deco
	float ceiling;
	float ceilingOther; // new hw 160614

	ceiling = tissue_tolerance();
	ambient_bar_to_deco_stop_depth_bar(ceiling); // this will set gStop.depth :-) (and gStop.id)

	// set the base for all upcoming parameters
	ceilingOther = gStop.depth + gSurface_pressure_bar;

	// modify parameters if there is ascend or parameter fine adjustment
	if(ceilingOther < (gPressure - PRESSURE_150_CM)) // more than 1.5 meter below ceiling
	{
		// ascend within 10 mtr to GF_low // speed 12 mtr/min -> 50 sec / 10 mtr;  15 sec / 3 mtr.
		while(((gPressure - PRESSURE_TEN_METER ) > gSurface_pressure_bar) && (ceiling < (gPressure - PRESSURE_TEN_METER)))
		{
			ascend_with_all_gaschanges(PRESSURE_TEN_METER);
			ceiling = tissue_tolerance();
		}
		while(((gPressure - PRESSURE_THREE_METER )> gSurface_pressure_bar) && (ceiling < gPressure))
		{
			ascend_with_all_gaschanges(PRESSURE_THREE_METER);
			ceiling = tissue_tolerance();
		}
	}
	if(ceiling <= gSurface_pressure_bar)
		return false;
	else
		return true;
}


void buehlmann_ceiling_calculator(SLifeData* pLifeData, SDiveSettings * pDiveSettings, SDecoinfo * pDecoInfo)
{
	float gf_low;
	float gf_high;
	float gf_delta;
	int dv_gf_low_stop_meter;
	_Bool test_result;
	float next_gf_value;
	float next_pressure_absolute;
	int depth_in_meter;
	
	gf_low = pDiveSettings->gf_low;
	gf_high = pDiveSettings->gf_high;

	//
	dv_gf_low_stop_meter = (int)((pDiveSettings->internal__pressure_first_stop_ambient_bar_as_upper_limit_for_gf_low_otherwise_zero - pLifeData->pressure_surface_bar) * 10);
	//
	if(dv_gf_low_stop_meter < 1)
	{
		next_gf_value = gf_high; // fix hw 161024
		gf_delta = 0;
	}
	else
	{
		next_gf_value = gf_high;
		gf_delta = gf_high - gf_low;
		gf_delta /= dv_gf_low_stop_meter; // gf_delta is delta for each meter now!!
	}
	//
	depth_in_meter = 0;
	next_pressure_absolute = pLifeData->pressure_surface_bar;

	memcpy(gTissue_nitrogen_bar, pLifeData->tissue_nitrogen_bar, (4*16));
	memcpy(gTissue_helium_bar, pLifeData->tissue_helium_bar, (4*16));
	gGF_value = next_gf_value / 100.0f;
	//
	test_result = buehlmann_tissue_test_tolerance(next_pressure_absolute);
	//
	while(!test_result && depth_in_meter < 200)
	{
		depth_in_meter += 1;
		next_gf_value = fmaxf(gf_low, next_gf_value - gf_delta);
		gGF_value = next_gf_value / 100.0f;
		next_pressure_absolute += 0.1f; // 1 meter down
		test_result = buehlmann_tissue_test_tolerance(next_pressure_absolute);
	}
	//
	if(test_result)
	{
		// old direct paste
		pDecoInfo->output_ceiling_meter = depth_in_meter;
		// new sub-meter hw 160331
		if(depth_in_meter >= 1)
		{
			for(int i = 0; i < 10; i++)
			{
				next_gf_value += gf_delta/10.0f;
				gGF_value = next_gf_value / 100.0f;
				next_pressure_absolute -= 0.01f; // 0.1 meter up
				if(!buehlmann_tissue_test_tolerance(next_pressure_absolute))
				{
					pDecoInfo->output_ceiling_meter -= ((float)i)/10.0f;
					break;
				}
			}
		}
	}
	else
	{
		pDecoInfo->output_ceiling_meter = 999;
	}
}


void buehlmann_relative_gradient_calculator(SLifeData* pLifeData, SDiveSettings * pDiveSettings, SDecoinfo * pDecoInfo)
{
	float gf_low;
	float gf_high;
	float gf_delta;
	int dv_gf_low_stop_meter;

	float rgf; // relative gradient factor by hwOS p2_deco.c
	float temp_tissue;
	float limit;
	float pres_respiration;
	float gf;

	gf_low = pDiveSettings->gf_low;
	gf_high = pDiveSettings->gf_high;

	dv_gf_low_stop_meter = (int)((pDiveSettings->internal__pressure_first_stop_ambient_bar_as_upper_limit_for_gf_low_otherwise_zero - pLifeData->pressure_surface_bar) * 10);

	if(dv_gf_low_stop_meter < 1)
	{
		gf_delta = 0;
	}
	else
	{
		gf_delta = gf_high - gf_low;
		gf_delta /= dv_gf_low_stop_meter; // gf_delta is delta for each meter now!!
	}

	
	limit = tissue_tolerance_without_gf_correction(&temp_tissue);
	pres_respiration = pLifeData->pressure_ambient_bar;
	
	if( temp_tissue <= pres_respiration )
	{
		gf = 0.0;
	}
	else
	{
		gf = (temp_tissue  - pres_respiration)
			 / (temp_tissue  - limit)
			 * 100.0f;
	}
	
	if(dv_gf_low_stop_meter < 1)
	{
		rgf = gf_high;
	}
	else
	{
		float temp1 = dv_gf_low_stop_meter; 
		float temp2 = pLifeData->depth_meter;

		if (temp2 <= 0)
				rgf = gf_high;
		else if (temp2 >= temp1)
				rgf = gf_low;
		else
				rgf = gf_low + (temp1 - temp2)*gf_delta;
	}
	
	rgf = gf / rgf;
	
	// avoid discussions about values > 100 below next deco stop
	if((rgf > 1.0f) && (pLifeData->depth_meter >= pDecoInfo->output_ceiling_meter))
		rgf = 1.0f;
	
	pDecoInfo->output_relative_gradient = rgf;
}
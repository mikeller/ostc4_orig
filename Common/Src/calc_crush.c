///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Common/Src/calc_crush.c
/// \brief	VPM Desaturation code
/// \author Heinrichs Weikamp
/// \date   2018
///
/// $Id$
///////////////////////////////////////////////////////////////////////////////
/// \par Copyright (c) 2014-2018 Heinrichs Weikamp gmbh
///
///     This program is free software: you can redistribute it and/or modify
///     it under the terms of the GNU General Public License as published by
///     the Free Software Foundation, either version 3 of the License, or
///     (at your option) any later version.
///
///     This program is distributed in the hope that it will be useful,
///     but WITHOUT ANY WARRANTY; without even the implied warranty of
///     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
///     GNU General Public License for more details.
///
///     You should have received a copy of the GNU General Public License
///     along with this program.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////////////

#include "calc_crush.h"

#include "decom.h"
#include "math.h"
#include "vpm.h"

/* Common Block Declarations */
//#pragma warning(disable:1035)

const float SURFACE_TENSION_GAMMA = 0.0179f;				//!Adj. Range: 0.015 to 0.065 N/m
const float SKIN_COMPRESSION_GAMMAC = 0.257f;				//!Adj. Range: 0.160 to 0.290 N/m
const float UNITS_FACTOR = 10.1325f;
const float WATER_VAPOR_PRESSURE = 0.493f;				// (Schreiner value)  based on respiratory quotien
const float CRIT_VOLUME_PARAMETER_LAMBDA = 7500.0f;			//!Adj. Range: 6500 to 8300 fsw-min
const float GRADIENT_ONSET_OF_IMPERM_ATM = 8.2f;			//!Adj. Range: 5.0 to 10.0 atm
const float REGENERATION_TIME_CONSTANT = 20160.0f;			//!Adj. Range: 10080 to 51840 min
const float PRESSURE_OTHER_GASES_MMHG = 102.0f;				//!Constant value for PO2 up to 2 atm
const float CONSTANT_PRESSURE_OTHER_GASES = 102.0f * 10.1325f / 760.0f; // PRESSURE_OTHER_GASES_MMHG / 760. * UNITS_FACTOR;

const float HELIUM_TIME_CONSTANT[16] = {3.68695308808482E-001f,
										2.29518933960247E-001f,
										1.46853216220327E-001f,
										9.91626867753856E-002f,
										6.78890480470074E-002f,
										4.78692804254106E-002f,
										3.37626488338989E-002f,
										2.38113081607676E-002f,
										1.68239606932026E-002f,
										1.25592893741610E-002f,
										9.80544886914621E-003f,
										7.67264977374303E-003f,
										6.01220557342307E-003f,
										4.70185307665137E-003f,
										3.68225234041620E-003f,
										2.88775228329769E-003f};

 const float NITROGEN_TIME_CONSTANT[16] = {1.38629436111989E-001f,
										8.66433975699932E-002f,
										5.54517744447956E-002f,
										3.74674151654024E-002f,
										2.56721177985165E-002f,
										1.80978376125312E-002f,
										1.27651414467762E-002f,
										9.00191143584345E-003f,
										6.35914844550409E-003f,
										4.74758342849278E-003f,
										3.70666941475907E-003f,
										2.90019740820061E-003f,
										2.27261370675392E-003f,
										1.77730046297422E-003f,
										1.39186180835330E-003f,
										1.09157036308653E-003f};

int onset_of_impermeability(SGas* pGas, float *starting_ambient_pressure,
float *ending_ambient_pressure,
float *rate,
float*   amb_pressure_onset_of_imperm,
float* gas_tension_onset_of_imperm,
float* initial_helium_pressure,
float* initial_nitrogen_pressure,
short i);
int radius_root_finder (float *a, float *b, float *c, float *low_bound, float *high_bound, float *ending_radius);
//void  get_inert_gases_(SBuehlmann* input, ,short gas_id, float ambient_pressure_bar, float* fraction_nitrogen,float* fraction_helium );
int vpm_repetitive_algorithm(SVpm* pVpm, float *surface_interval_time, float* initial_critical_radius_he, float* initial_critical_radius_n2);



/* =============================================================================== */
/*     NOTE ABOUT PRESSURE UNITS USED IN CALCULATIONS: */
/*     It is the convention in decompression calculations to compute all gas */
/*     loadings, absolute pressures, partial pressures, etc., in the units of */
/*     depth pressure that you are diving - either feet of seawater (fsw) or */
/*     meters of seawater (msw).  This program follows that convention with the */
/*     the exception that all VPM calculations are performed in SI units (by */
/*     necessity).  Accordingly, there are several conversions back and forth */
/*     between the diving pressure units and the SI units. */
/* =============================================================================== */
/* =============================================================================== */
/*     FUNCTION SUBPROGRAM FOR GAS LOADING CALCULATIONS - ASCENT AND DESCENT */
/* =============================================================================== */

float schreiner_equation__2(float *initial_inspired_gas_pressure,
float *rate_change_insp_gas_pressure,
float *interval_time_minutes,
const float *gas_time_constant,
float *initial_gas_pressure)
{
	/* System generated locals */
	float ret_val;
	float time_null_pressure = 0.0f;
	float time_rest = 0.0f;
	float time = *interval_time_minutes;
	/* =============================================================================== */
	/*     Note: The Schreiner equation is applied when calculating the uptake or */
	/*     elimination of compartment gases during linear ascents or descents at a */
	/*     constant rate.  For ascents, a negative number for rate must be used. */
	/* =============================================================================== */
	if( *rate_change_insp_gas_pressure < 0.0f)
	{
		time_null_pressure = -1.0f * *initial_inspired_gas_pressure / *rate_change_insp_gas_pressure;
		if(time > time_null_pressure )
		{
			time_rest = time - time_null_pressure;
			time = time_null_pressure;
		}
	}
	ret_val =
	*initial_inspired_gas_pressure +
	*rate_change_insp_gas_pressure *
	(time - 1.f / *gas_time_constant) -
	(*initial_inspired_gas_pressure -
	*initial_gas_pressure -
	*rate_change_insp_gas_pressure / *gas_time_constant) *
	expf(-(*gas_time_constant) * time);

	if(time_rest > 0.0f)
	{
		ret_val = ret_val * expf(-(*gas_time_constant) * time_rest);
	}


	return ret_val;
}; /* schreiner_equation__2 */

/* =============================================================================== */
/*     SUBROUTINE CALC_CRUSHING_PRESSURE */
/*     Purpose: Compute the effective "crushing pressure" in each compartment as */
/*     a result of descent segment(s).  The crushing pressure is the gradient */
/*     (difference in pressure) between the outside ambient pressure and the */
/*     gas tension inside a VPM nucleus (bubble seed).  This gradient acts to */
/*     reduce (shrink) the radius smaller than its initial value at the surface. */
/*     This phenomenon has important ramifications because the smaller the radius */
/*     of a VPM nucleus, the greater the allowable supersaturation gradient upon */
/*     ascent.  Gas loading (uptake) during descent, especially in the fast */
/*     compartments, will reduce the magnitude of the crushing pressure.  The */
/*     crushing pressure is not cumulative over a multi-level descent.  It will */
/*     be the maximum value obtained in any one discrete segment of the overall */
/*     descent.  Thus, the program must compute and store the maximum crushing */
/*     pressure for each compartment that was obtained across all segments of */
/*     the descent profile. */

/*     The calculation of crushing pressure will be different depending on */
/*     whether or not the gradient is in the VPM permeable range (gas can diffuse */
/*     across skin of VPM nucleus) or the VPM impermeable range (molecules in */
/*     skin of nucleus are squeezed together so tight that gas can no longer */
/*     diffuse in or out of nucleus; the gas becomes trapped and further resists */
/*     the crushing pressure).  The solution for crushing pressure in the VPM */
/*     permeable range is a simple linear equation.  In the VPM impermeable */
/*     range, a cubic equation must be solved using a numerical method. */

/*     Separate crushing pressures are tracked for helium and nitrogen because */
/*     they can have different critical radii.  The crushing pressures will be */
/*     the same for helium and nitrogen in the permeable range of the model, but */
/*     they will start to diverge in the impermeable range.  This is due to */
/*     the differences between starting radius, radius at the onset of */
/*     impermeability, and radial compression in the impermeable range. */
/* =============================================================================== */
int calc_crushing_pressure(SLifeData* lifeData, SVpm* vpm, float * initial_helium_pressure, float * initial_nitrogen_pressure, float starting_ambient_pressure,
float rate )
{
	/* System generated locals */
static	float r1, r2;

static	float low_bound_n2,
	ending_radius_n2,
	gradient_onset_of_imperm_pa;
static	float low_bound_he,
	ending_radius_he,
	high_bound_n2,
	crushing_pressure_n2;
	short i;
static float crushing_pressure_pascals_n2,
	gradient_onset_of_imperm,
	starting_gas_tension,
	high_bound_he,
	crushing_pressure_he,
	amb_press_onset_of_imperm_pa,
	crushing_pressure_pascals_he,
	radius_onset_of_imperm_n2,
	starting_gradient,
	radius_onset_of_imperm_he,
	ending_gas_tension;

static	float ending_ambient_pressure_pa,
	a_n2,
	b_n2,
	c_n2,
	ending_gradient,
	gas_tension_onset_of_imperm_pa,
	a_he,
	b_he, c_he;
static	float amb_pressure_onset_of_imperm[16];
static	float gas_tension_onset_of_imperm[16];

static	float helium_pressure_crush[16];
static	float	nitrogen_pressure_crush[16];


static	float ending_ambient_pressure = 0;

		ending_ambient_pressure = lifeData->pressure_ambient_bar * 10;
	for( i = 0; i < 16; i++)
	{
		helium_pressure_crush[i] = lifeData->tissue_helium_bar[i] * 10;
		nitrogen_pressure_crush[i] = lifeData->tissue_nitrogen_bar[i] * 10;
	}





	/* loop */
	/* =============================================================================== */
	/*     CALCULATIONS */
	/*     First, convert the Gradient for Onset of Impermeability from units of */
	/*     atmospheres to diving pressure units (either fsw or msw) and to Pascals */
	/*     (SI units).  The reason that the Gradient for Onset of Impermeability is */
	/*     given in the program settings in units of atmospheres is because that is */
	/*     how it was reported in the original research papers by Yount and */
	/*     colleauges. */
	/* =============================================================================== */

	gradient_onset_of_imperm = GRADIENT_ONSET_OF_IMPERM_ATM * UNITS_FACTOR;
	gradient_onset_of_imperm_pa = GRADIENT_ONSET_OF_IMPERM_ATM * 101325.0f;

	/* =============================================================================== */
	/*     Assign values of starting and ending ambient pressures for descent segment */
	/* =============================================================================== */

	//starting_ambient_pressure = *starting_depth;
	//ending_ambient_pressure = *ending_depth;

	/* =============================================================================== */
	/*     MAIN LOOP WITH NESTED DECISION TREE */
	/*     For each compartment, the program computes the starting and ending */
	/*     gas tensions and gradients.  The VPM is different than some dissolved gas */
	/*     algorithms, Buhlmann for example, in that it considers the pressure due to */
	/*     oxygen, carbon dioxide, and water vapor in each compartment in addition to */
	/*     the inert gases helium and nitrogen.  These "other gases" are included in */
	/*     the calculation of gas tensions and gradients. */
	/* =============================================================================== */

	crushing_pressure_he = 0.0f;
	crushing_pressure_n2 = 0.0f;

	for (i = 0; i < 16; ++i) {
		starting_gas_tension = initial_helium_pressure[i] + initial_nitrogen_pressure[i] +	CONSTANT_PRESSURE_OTHER_GASES;
		starting_gradient = starting_ambient_pressure - starting_gas_tension;
		ending_gas_tension = helium_pressure_crush[i] + nitrogen_pressure_crush[i] + CONSTANT_PRESSURE_OTHER_GASES;
		ending_gradient = ending_ambient_pressure - ending_gas_tension;

		/* =============================================================================== */
		/*     Compute radius at onset of impermeability for helium and nitrogen */
		/*     critical radii */
		/* =============================================================================== */

		radius_onset_of_imperm_he = 1.0f / (
		gradient_onset_of_imperm_pa /
		((SKIN_COMPRESSION_GAMMAC -
		SURFACE_TENSION_GAMMA) * 2.0f) +
		1.0f / vpm->adjusted_critical_radius_he[i]);
		radius_onset_of_imperm_n2 = 1.0f / (
		gradient_onset_of_imperm_pa /
		((SKIN_COMPRESSION_GAMMAC -
		SURFACE_TENSION_GAMMA) * 2.0f) +
		1.0f / vpm->adjusted_critical_radius_n2[i]);

		/* =============================================================================== */
		/*     FIRST BRANCH OF DECISION TREE - PERMEABLE RANGE */
		/*     Crushing pressures will be the same for helium and nitrogen */
		/* =============================================================================== */

		if (ending_gradient <= gradient_onset_of_imperm) {
			crushing_pressure_he = ending_ambient_pressure - ending_gas_tension;
			crushing_pressure_n2 = ending_ambient_pressure - ending_gas_tension;
		}

		/* =============================================================================== */
		/*     SECOND BRANCH OF DECISION TREE - IMPERMEABLE RANGE */
		/*     Both the ambient pressure and the gas tension at the onset of */
		/*     impermeability must be computed in order to properly solve for the ending */
		/*     radius and resultant crushing pressure.  The first decision block */
		/*     addresses the special case when the starting gradient just happens to be */
		/*     equal to the gradient for onset of impermeability (not very likely!). */
		/* =============================================================================== */

		if (ending_gradient > gradient_onset_of_imperm) {
			if (starting_gradient == gradient_onset_of_imperm) {
				amb_pressure_onset_of_imperm[i] = starting_ambient_pressure;
				gas_tension_onset_of_imperm[i] = starting_gas_tension;
			}

			/* =============================================================================== */
			/*     In most cases, a subroutine will be called to find these values using a */
			/*     numerical method. */
			/* =============================================================================== */

			if (starting_gradient < gradient_onset_of_imperm) {
				onset_of_impermeability(&(lifeData->actualGas), &starting_ambient_pressure, &ending_ambient_pressure, &rate,
				amb_pressure_onset_of_imperm, gas_tension_onset_of_imperm,
				initial_helium_pressure, initial_nitrogen_pressure, i);
			}

			/* =============================================================================== */
			/*     Next, using the values for ambient pressure and gas tension at the onset */
			/*     of impermeability, the equations are set up to process the calculations */
			/*     through the radius root finder subroutine.  This subprogram will find the */
			/*     root (solution) to the cubic equation using a numerical method.  In order */
			/*     to do this efficiently, the equations are placed in the form */
			/*     Ar^3 - Br^2 - C = 0, where r is the ending radius after impermeable */
			/*     compression.  The coefficients A, B, and C for helium and nitrogen are */
			/*     computed and passed to the subroutine as arguments.  The high and low */
			/*     bounds to be used by the numerical method of the subroutine are also */
			/*     computed (see separate page posted on Deco List ftp site entitled */
			/*     "VPM: Solving for radius in the impermeable regime").  The subprogram */
			/*     will return the value of the ending radius and then the crushing */
			/*     pressures for helium and nitrogen can be calculated. */
			/* =============================================================================== */

			ending_ambient_pressure_pa = ending_ambient_pressure / UNITS_FACTOR * 101325.0f;
			amb_press_onset_of_imperm_pa = 	amb_pressure_onset_of_imperm[i] / UNITS_FACTOR * 101325.0f;
			gas_tension_onset_of_imperm_pa = gas_tension_onset_of_imperm[i] / UNITS_FACTOR * 101325.0f;
			b_he = (SKIN_COMPRESSION_GAMMAC - SURFACE_TENSION_GAMMA) * 2.0f;
			a_he = ending_ambient_pressure_pa - amb_press_onset_of_imperm_pa + gas_tension_onset_of_imperm_pa
					+ (SKIN_COMPRESSION_GAMMAC - SURFACE_TENSION_GAMMA) * 2.0f / radius_onset_of_imperm_he;
			/* Computing 3rd power */
			r1 = radius_onset_of_imperm_he;
			c_he = gas_tension_onset_of_imperm_pa * (r1 * (r1 * r1));
			high_bound_he = radius_onset_of_imperm_he;
			low_bound_he = b_he / a_he;
			radius_root_finder(&a_he, &b_he, &c_he, &low_bound_he, &high_bound_he, &ending_radius_he);
			/* Computing 3rd power */
			r1 = radius_onset_of_imperm_he;
			/* Computing 3rd power */
			r2 = ending_radius_he;
			crushing_pressure_pascals_he =
			gradient_onset_of_imperm_pa +
			ending_ambient_pressure_pa -
			amb_press_onset_of_imperm_pa +
			gas_tension_onset_of_imperm_pa *
			(1.0f - r1 * (r1 * r1) / (r2 * (r2 * r2)));
			crushing_pressure_he =
			crushing_pressure_pascals_he / 101325.0f * UNITS_FACTOR;
			b_n2 = (SKIN_COMPRESSION_GAMMAC - SURFACE_TENSION_GAMMA) * 2.0f;
			a_n2 = ending_ambient_pressure_pa -
			amb_press_onset_of_imperm_pa +
			gas_tension_onset_of_imperm_pa +
			(SKIN_COMPRESSION_GAMMAC - SURFACE_TENSION_GAMMA) *
			2.0f / radius_onset_of_imperm_n2;
			/* Computing 3rd power */
			r1 = radius_onset_of_imperm_n2;
			c_n2 = gas_tension_onset_of_imperm_pa * (r1 * (r1 * r1));
			high_bound_n2 = radius_onset_of_imperm_n2;
			low_bound_n2 = b_n2 / a_n2;
			radius_root_finder(&a_n2,
			&b_n2,
			&c_n2,
			&low_bound_n2,
			&high_bound_n2,
			&ending_radius_n2);

			/* Computing 3rd power */
			r1 = radius_onset_of_imperm_n2;
			/* Computing 3rd power */
			r2 = ending_radius_n2;
			crushing_pressure_pascals_n2 =
			gradient_onset_of_imperm_pa +
			ending_ambient_pressure_pa -
			amb_press_onset_of_imperm_pa +
			gas_tension_onset_of_imperm_pa * (1.0f - r1 *
			(r1 * r1) / (r2 * (r2 * r2)));
			crushing_pressure_n2 = crushing_pressure_pascals_n2 / 101325.0f * UNITS_FACTOR;
		}

		/* =============================================================================== */
		/*     UPDATE VALUES OF MAX CRUSHING PRESSURE IN GLOBAL ARRAYS */
		/* =============================================================================== */

		/* Computing MAX */
		r1 = vpm->max_crushing_pressure_he[i];
		vpm->max_crushing_pressure_he[i] = fmaxf(r1, crushing_pressure_he);
		/* Computing MAX */
		r1 = vpm->max_crushing_pressure_n2[i];
		vpm->max_crushing_pressure_n2[i] = fmaxf(r1, crushing_pressure_n2);
	}
	return 0;
} /* calc_crushing_pressure */

/* =============================================================================== */
/*     SUBROUTINE ONSET_OF_IMPERMEABILITY */
/*     Purpose:  This subroutine uses the Bisection Method to find the ambient */
/*     pressure and gas tension at the onset of impermeability for a given */
/*     compartment.  Source:  "Numerical Recipes in Fortran 77", */
/*     Cambridge University Press, 1992. */
/* =============================================================================== */

int onset_of_impermeability(SGas* pGas, float *starting_ambient_pressure,
float *ending_ambient_pressure,
float *rate,
float*   amb_pressure_onset_of_imperm,
float* gas_tension_onset_of_imperm,
float* initial_helium_pressure,
float* initial_nitrogen_pressure,
short i)
{
	/* Local variables */
	float time, last_diff_change, mid_range_nitrogen_pressure;
	short j;
	float gas_tension_at_mid_range,
	initial_inspired_n2_pressure,
	gradient_onset_of_imperm,
	starting_gas_tension,
	low_bound,
	initial_inspired_he_pressure,
	high_bound_nitrogen_pressure,
	nitrogen_rate,
	function_at_mid_range,
	function_at_low_bound,
	high_bound,
	mid_range_helium_pressure,
	mid_range_time,
	ending_gas_tension,
	function_at_high_bound;

	float mid_range_ambient_pressure,
	high_bound_helium_pressure,
	helium_rate,
	differential_change;
	float fraction_helium_begin;
	float fraction_helium_end;
	float fraction_nitrogen_begin;
	float fraction_nitrogen_end;
	/* loop */
	/* =============================================================================== */
	/*     CALCULATIONS */
	/*     First convert the Gradient for Onset of Impermeability to the diving */
	/*     pressure units that are being used */
	/* =============================================================================== */

	gradient_onset_of_imperm = GRADIENT_ONSET_OF_IMPERM_ATM * UNITS_FACTOR;

	/* =============================================================================== */
	/*     ESTABLISH THE BOUNDS FOR THE ROOT SEARCH USING THE BISECTION METHOD */
	/*     In this case, we are solving for time - the time when the ambient pressure */
	/*     minus the gas tension will be equal to the Gradient for Onset of */
	/*     Impermeabliity.  The low bound for time is set at zero and the high */
	/*     bound is set at the elapsed time (segment time) it took to go from the */
	/*     starting ambient pressure to the ending ambient pressure.  The desired */
	/*     ambient pressure and gas tension at the onset of impermeability will */
	/*     be found somewhere between these endpoints.  The algorithm checks to */
	/*     make sure that the solution lies in between these bounds by first */
	/*     computing the low bound and high bound function values. */
	/* =============================================================================== */

	/*initial_inspired_he_pressure =
	(*starting_ambient_pressure - water_vapor_pressure) * fraction_helium[mix_number - 1];
	initial_inspired_n2_pressure =
	(*starting_ambient_pressure - water_vapor_pressure) * fraction_nitrogen[mix_number - 1];
	helium_rate = *rate * fraction_helium[mix_number - 1];
	nitrogen_rate = *rate * fraction_nitrogen[mix_number - 1];*/
	low_bound = 0.;
	high_bound = (*ending_ambient_pressure - *starting_ambient_pressure) / *rate;

	//New
	decom_get_inert_gases( *starting_ambient_pressure / 10.0f, pGas, &fraction_nitrogen_begin, &fraction_helium_begin );
	decom_get_inert_gases(*ending_ambient_pressure   / 10.0f, pGas, &fraction_nitrogen_end, &fraction_helium_end );
	initial_inspired_he_pressure =	(*starting_ambient_pressure - WATER_VAPOR_PRESSURE) * fraction_helium_begin;
	initial_inspired_n2_pressure =	(*starting_ambient_pressure - WATER_VAPOR_PRESSURE) * fraction_nitrogen_begin;
	helium_rate = ((*ending_ambient_pressure  - WATER_VAPOR_PRESSURE)* fraction_helium_end - initial_inspired_he_pressure)/high_bound;
	nitrogen_rate = ((*ending_ambient_pressure  - WATER_VAPOR_PRESSURE)* fraction_nitrogen_end - initial_inspired_n2_pressure)/high_bound;

	starting_gas_tension =
	initial_helium_pressure[i] +
	initial_nitrogen_pressure[i] +
	CONSTANT_PRESSURE_OTHER_GASES;
	function_at_low_bound =
	*starting_ambient_pressure -
	starting_gas_tension -
	gradient_onset_of_imperm;
	high_bound_helium_pressure =
	schreiner_equation__2(&initial_inspired_he_pressure,
	&helium_rate,
	&high_bound,
	&HELIUM_TIME_CONSTANT[i],
	&initial_helium_pressure[i]);
	high_bound_nitrogen_pressure =
	schreiner_equation__2(&initial_inspired_n2_pressure,
	&nitrogen_rate,
	&high_bound,
	&NITROGEN_TIME_CONSTANT[i],
	&initial_nitrogen_pressure[i]);
	ending_gas_tension =
	high_bound_helium_pressure +
	high_bound_nitrogen_pressure +
	CONSTANT_PRESSURE_OTHER_GASES;
	function_at_high_bound =
	*ending_ambient_pressure -
	ending_gas_tension -
	gradient_onset_of_imperm;
	if (function_at_high_bound * function_at_low_bound >= 0.0f) {
		//printf("\nERROR! ROOT IS NOT WITHIN BRACKETS");
	}

	/* =============================================================================== */
	/*     APPLY THE BISECTION METHOD IN SEVERAL ITERATIONS UNTIL A SOLUTION WITH */
	/*     THE DESIRED ACCURACY IS FOUND */
	/*     Note: the program allows for up to 100 iterations.  Normally an exit will */
	/*     be made from the loop well before that number.  If, for some reason, the */
	/*     program exceeds 100 iterations, there will be a pause to alert the user. */
	/* =============================================================================== */

	if (function_at_low_bound < 0.0f) {
		time = low_bound;
		differential_change = high_bound - low_bound;
	} else {
		time = high_bound;
		differential_change = low_bound - high_bound;
	}
	for (j = 1; j <= 100; ++j) {
		last_diff_change = differential_change;
		differential_change = last_diff_change * 0.5f;
		mid_range_time = time + differential_change;
		mid_range_ambient_pressure = *starting_ambient_pressure + *rate * mid_range_time;
		mid_range_helium_pressure =
		schreiner_equation__2(&initial_inspired_he_pressure,
		&helium_rate,
		&mid_range_time,
		&HELIUM_TIME_CONSTANT[i],
		&initial_helium_pressure[i]);
		mid_range_nitrogen_pressure =
		schreiner_equation__2(&initial_inspired_n2_pressure,
		&nitrogen_rate,
		&mid_range_time,
		&NITROGEN_TIME_CONSTANT[i],
		&initial_nitrogen_pressure[i]);
		gas_tension_at_mid_range =
		mid_range_helium_pressure +
		mid_range_nitrogen_pressure +
		CONSTANT_PRESSURE_OTHER_GASES;
		function_at_mid_range =
		mid_range_ambient_pressure -
		gas_tension_at_mid_range -
		gradient_onset_of_imperm;
		if (function_at_mid_range <= 0.0f) {
			time = mid_range_time;
		}
		if (fabs(differential_change) < .001f ||
		function_at_mid_range == 0.0f) {
			goto L100;
		}
	}
	//printf("\nERROR! ROOT SEARCH EXCEEDED MAXIMUM ITERATIONS");

	/* =============================================================================== */
	/*     When a solution with the desired accuracy is found, the program jumps out */
	/*     of the loop to Line 100 and assigns the solution values for ambient */
	/*     pressure and gas tension at the onset of impermeability. */
	/* =============================================================================== */

	L100:
	amb_pressure_onset_of_imperm[i] = mid_range_ambient_pressure;
	gas_tension_onset_of_imperm[i] = gas_tension_at_mid_range;
	return 0;
} /* onset_of_impermeability */


/* =============================================================================== */
/*     SUBROUTINE RADIUS_ROOT_FINDER */
/*     Purpose: This subroutine is a "fail-safe" routine that combines the */
/*     Bisection Method and the Newton-Raphson Method to find the desired root. */
/*     This hybrid algorithm takes a bisection step whenever Newton-Raphson would */
/*     take the solution out of bounds, or whenever Newton-Raphson is not */
/*     converging fast enough.  Source:  "Numerical Recipes in Fortran 77", */
/*     Cambridge University Press, 1992. */
/* =============================================================================== */

int radius_root_finder (float *a,
float *b,
float *c,
float *low_bound,
float *high_bound,
float *ending_radius)
{
	/* System generated locals */
	float r1, r2;

	/* Local variables */
	float radius_at_low_bound,
	last_diff_change,
	function,
	radius_at_high_bound;
	short i;
	float function_at_low_bound,
	last_ending_radius,
	function_at_high_bound,
	derivative_of_function,
	differential_change;

	/* loop */
	/* =============================================================================== */
	/*     BEGIN CALCULATIONS BY MAKING SURE THAT THE ROOT LIES WITHIN BOUNDS */
	/*     In this case we are solving for radius in a cubic equation of the form, */
	/*     Ar^3 - Br^2 - C = 0.  The coefficients A, B, and C were passed to this */
	/*     subroutine as arguments. */
	/* =============================================================================== */

	function_at_low_bound =
	*low_bound * (*low_bound * (*a * *low_bound - *b)) - *c;
	function_at_high_bound =
	*high_bound * (*high_bound * (*a * *high_bound - *b)) - *c;
	if (function_at_low_bound > 0.0f && function_at_high_bound > 0.0f) {
//		printf("\nERROR! ROOT IS NOT WITHIN BRACKETS");

	}

	/* =============================================================================== */
	/*     Next the algorithm checks for special conditions and then prepares for */
	/*     the first bisection. */
	/* =============================================================================== */

	if (function_at_low_bound < 0.0f && function_at_high_bound < 0.0f) {
		//printf("\nERROR! ROOT IS NOT WITHIN BRACKETS");

	}
	if (function_at_low_bound == 0.0f) {
		*ending_radius = *low_bound;
		return 0;
	} else if (function_at_high_bound == 0.0f) {
		*ending_radius = *high_bound;
		return 0;
	} else if (function_at_low_bound < 0.0f) {
		radius_at_low_bound = *low_bound;
		radius_at_high_bound = *high_bound;
	} else {
		radius_at_high_bound = *low_bound;
		radius_at_low_bound = *high_bound;
	}
	*ending_radius = (*low_bound + *high_bound) * .5f;
	last_diff_change = (r1 = *high_bound - *low_bound, fabs(r1));
	differential_change = last_diff_change;

	/* =============================================================================== */
	/*     At this point, the Newton-Raphson Method is applied which uses a function */
	/*     and its first derivative to rapidly converge upon a solution. */
	/*     Note: the program allows for up to 100 iterations.  Normally an exit will */
	/*     be made from the loop well before that number.  If, for some reason, the */
	/*     program exceeds 100 iterations, there will be a pause to alert the user. */
	/*     When a solution with the desired accuracy is found, exit is made from the */
	/*     loop by returning to the calling program.  The last value of ending */
	/*     radius has been assigned as the solution. */
	/* =============================================================================== */

	function =
	*ending_radius * (*ending_radius * (*a * *ending_radius - *b)) - *c;
	derivative_of_function =
	*ending_radius * (*ending_radius *  3.0f * *a - *b * 2.0f);
	for (i = 1; i <= 100; ++i) {
		if (((*ending_radius - radius_at_high_bound) * derivative_of_function - function) *
		((*ending_radius - radius_at_low_bound) * derivative_of_function - function) >= 0.0f
		|| (r1 = function * 2.0f, fabs(r1)) >
		(r2 = last_diff_change * derivative_of_function, fabs(r2))) {
			last_diff_change = differential_change;
			differential_change =
			(radius_at_high_bound - radius_at_low_bound) * .5f;
			*ending_radius = radius_at_low_bound + differential_change;
			if (radius_at_low_bound == *ending_radius) {
				return 0;
			}
		} else {
			last_diff_change = differential_change;
			differential_change = function / derivative_of_function;
			last_ending_radius = *ending_radius;
			*ending_radius -= differential_change;
			if (last_ending_radius == *ending_radius) {
				return 0;
			}
		}
		if (fabs(differential_change) < 1e-12) {
			return 0;
		}
		function =
		*ending_radius * (*ending_radius * (*a * *ending_radius - *b)) - *c;
		derivative_of_function =
		*ending_radius * (*ending_radius * 3.0f * *a - *b * 2.0f);
		if (function < 0.0f) {
			radius_at_low_bound = *ending_radius;
		} else {
			radius_at_high_bound = *ending_radius;
		}
	}
//	printf("\nERROR! ROOT SEARCH EXCEEDED MAXIMUM ITERATIONS");
	return 0;
} /* radius_root_finder */




void vpm_init(SVpm* pVpm, short conservatism, short repetitive_dive, long seconds_since_last_dive)
{

	float critical_radius_n2_microns,
		critical_radius_he_microns;
	float initial_critical_radius_n2[16];
	float initial_critical_radius_he[16];
	int i = 0;
	float surface_time = seconds_since_last_dive / 60;
	pVpm->repetitive_variables_not_valid = !repetitive_dive;
	//pVpm->vpm_conservatism = conservatism;
	switch(conservatism)
	{
		case 0:
			critical_radius_n2_microns=0.55;			//!Adj. Range: 0.2 to 1.35 microns
			critical_radius_he_microns=0.45;			//!Adj. Range: 0.2 to 1.35 microns
			break;
		case 1:
			critical_radius_n2_microns=0.58;
			critical_radius_he_microns=0.48;
			break;
		case 2:
			critical_radius_n2_microns=0.62;
			critical_radius_he_microns=0.52;
			break;
		case 3:
			critical_radius_n2_microns=0.68;
			critical_radius_he_microns=0.58;
			break;
		case 4:
			critical_radius_n2_microns=0.75;
			critical_radius_he_microns=0.65;
			break;
		case 5:
			critical_radius_n2_microns=0.82;
			critical_radius_he_microns=0.72;
			break;
	}

	 for (i = 0; i < 16; ++i) {
		 initial_critical_radius_n2[i] = critical_radius_n2_microns * 1e-6f;
		 initial_critical_radius_he[i] = critical_radius_he_microns * 1e-6f;
	 }



	if( (surface_time > 0)
		&& (!pVpm->repetitive_variables_not_valid) )
		//&& (pVpm->decomode_vpm_plus_conservatism_last_dive > 0)
		//&& (pVpm->decomode_vpm_plus_conservatism_last_dive - 1 == pVpm->vpm_conservatism))
	{
		vpm_repetitive_algorithm(pVpm, &surface_time,initial_critical_radius_he, initial_critical_radius_n2);
	}
	else
	{
		//Kein gültiger Wiederholungstauchgang
		for (i = 0; i < 16; ++i) {
			pVpm->adjusted_critical_radius_n2[i] = initial_critical_radius_n2[i];
			pVpm->adjusted_critical_radius_he[i] = initial_critical_radius_he[i];
		}
		pVpm->repetitive_variables_not_valid = 0;
	}
	for (i = 0; i < 16; ++i) {
		pVpm->max_crushing_pressure_he[i] = 0.0f;
		pVpm->max_crushing_pressure_n2[i] = 0.0f;
		pVpm->max_actual_gradient[i] = 0.0f;
								pVpm->adjusted_crushing_pressure_he[i]  = 0.0f;
								pVpm->adjusted_crushing_pressure_n2[i]  = 0.0f;
								pVpm->initial_allowable_gradient_he[i]  = 0.0f;
								pVpm->initial_allowable_gradient_n2[i]  = 0.0f;
	}
	pVpm->max_first_stop_depth_save = 0;
	pVpm->depth_start_of_deco_zone_save = 0;
	pVpm->run_time_start_of_deco_zone_save = 0;
	pVpm->deco_zone_reached = 0;
}

/* =============================================================================== */
/*     SUBROUTINE VPM_REPETITIVE_ALGORITHM */
/*     Purpose: This subprogram implements the VPM Repetitive Algorithm that was */
/*     envisioned by Professor David E. Yount only months before his passing. */
/* =============================================================================== */
int vpm_repetitive_algorithm(SVpm* pVpm, float *surface_interval_time, float* initial_critical_radius_he, float* initial_critical_radius_n2)
{
	/* Local variables */
	static float max_actual_gradient_pascals;
	//static float initial_allowable_grad_n2_pa, initial_allowable_grad_he_pa;
	static short i;
	static float adj_crush_pressure_n2_pascals,
	new_critical_radius_n2,
	adj_crush_pressure_he_pascals,
	new_critical_radius_he;

	/* loop */
	/* =============================================================================== */
	/*     CALCULATIONS */

	/*			by hw 160215  */
	/*			IN: */
	/*			pVpm->max_actual_gradient[i] */
	/*			pVpm->initial_allowable_gradient_n2[i] */
	/*			pVpm->initial_allowable_gradient_he[i] */
	/*			pVpm->adjusted_crushing_pressure_he[i] */
	/*			pVpm->adjusted_crushing_pressure_n2[i] */
	/*			OUT:	 */
	/*			pVpm->adjusted_critical_radius_n2[i] */
	/*			pVpm->adjusted_critical_radius_he[i]  */
	/* =============================================================================== */

	for (i = 0; i < 16; ++i) {
		max_actual_gradient_pascals = pVpm->max_actual_gradient[i] / UNITS_FACTOR * 101325.0f;
		adj_crush_pressure_he_pascals = pVpm->adjusted_crushing_pressure_he[i] / UNITS_FACTOR * 101325.0f;
		adj_crush_pressure_n2_pascals = pVpm->adjusted_crushing_pressure_n2[i] / UNITS_FACTOR * 101325.0f;
/*
		initial_allowable_grad_he_pa =
		pVpm->initial_allowable_gradient_he[i] / UNITS_FACTOR * 101325.0f;
		initial_allowable_grad_n2_pa =
		pVpm->initial_allowable_gradient_n2[i] / UNITS_FACTOR * 101325.0f;
*/
		if (pVpm->max_actual_gradient[i] > pVpm->initial_allowable_gradient_n2[i])
		{
			new_critical_radius_n2 =
				SURFACE_TENSION_GAMMA * 2.0f *
				(SKIN_COMPRESSION_GAMMAC - SURFACE_TENSION_GAMMA) /
				(max_actual_gradient_pascals * SKIN_COMPRESSION_GAMMAC -
				SURFACE_TENSION_GAMMA * adj_crush_pressure_n2_pascals);

			pVpm->adjusted_critical_radius_n2[i] =
				initial_critical_radius_n2[i]
				+ (initial_critical_radius_n2[i] - new_critical_radius_n2)
				*  exp(-(*surface_interval_time) / REGENERATION_TIME_CONSTANT);

		} else {
			pVpm->adjusted_critical_radius_n2[i] =
			initial_critical_radius_n2[i];
		}
		if (pVpm->max_actual_gradient[i] > pVpm->initial_allowable_gradient_he[i])
		{
			new_critical_radius_he =
				SURFACE_TENSION_GAMMA * 2.0f *
				(SKIN_COMPRESSION_GAMMAC - SURFACE_TENSION_GAMMA) /
				(max_actual_gradient_pascals * SKIN_COMPRESSION_GAMMAC -
				SURFACE_TENSION_GAMMA * adj_crush_pressure_he_pascals);

			pVpm->adjusted_critical_radius_he[i] =
				initial_critical_radius_he[i]
				+ ( initial_critical_radius_he[i] -	new_critical_radius_he)
				* exp(-(*surface_interval_time) / REGENERATION_TIME_CONSTANT);
		} else {
			pVpm->adjusted_critical_radius_he[i] =
			initial_critical_radius_he[i];
		}
	}
	return 0;
} /* vpm_repetitive_algorithm */

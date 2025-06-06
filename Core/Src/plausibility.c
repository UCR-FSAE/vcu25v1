/*
 * plausibility.c
 *
 *  Created on: Jun 4, 2025
 *      Author: Steven Ryan Leonido
 */

#include "plausibility.h"

extern uint32_t appsRaw1Min;
extern uint32_t appsRaw1Max;
extern uint32_t appsRaw2Min;
extern uint32_t appsRaw2Max;

static uint32_t appsRaw1 = 0;
static uint32_t appsRaw2 = 0;

/*
 * AccelPos:
 * Get Accelerator Potentiometer values from ADC and determine position percentage
 * NEED TO REFERENCE GLOBAL MIN AND MAX VALUES FOR POTENTIOMETERS, LOOK AT JUSTIN BRANCH
 */
float AccelPos() {
	// TO BE IMPLEMENTED

	if (HAL_ADC_PollForConversion(&hadc3, 5) == HAL_OK) {
		appsRaw1 = HAL_ADC_GetValue(&hadc3);
	}
	else {
		// throw fault
	}
	if (HAL_ADC_PollForConversion(&hadc3, 7) == HAL_OK) {
		appsRaw2 = HAL_ADC_GetValue(&hadc3);
	}
	else {
		// throw fault
	}

	// FIGURE OUT HOW TO CALCULATE PERCENTAGE BASED ON LIMITS
	// PLACEHOLDER, Unsure if works yet
	uint32_t app1Range = appsRaw1Max - apps2Min;
	uint32_t app2Range = appsRaw2Max - apps2Min;

	float percent1 = appsRaw1 / apps1Range;
	float percent2 = appsRaw2 / apps2Range;

	float percentAvg = (percent1 + percent2) / 2;


	// PLACEHOLDER
	return percentAvg;

}

/*
 * BrakePos:
 * Get Brake Potentiometer values from ADC and determine position percentage
 */
float BrakePos() {
	// TO BE IMPLEMENTED

	// PLACEHOLDER
	return 0.5;
}

/*
 * Plausibility Check:
 * Looks at two pedal positions and if both abosve 5%, then return 0, else return 1
 */
int PlausibilityCheck(float accel, float brake) {

	if (accel > 0.05 && brake > 0.05)
		return 0;
	else
		return 1;
}

/*
 * MapTorque:
 * Gets Accelerator and Brake positions
 * Does Plausibility check, if fail, then return value to disable inverter
 * If Pass, Map torque request based on Accelerator Pedal Position and return torque request value
 */
int MapTorque() {
	HAL_ADC_Start(&hadc3);

	float accel = AccelPos();
	float brake = BrakePos();

	int check = PlausibilityCheck(accel, brake);

	if (check == 0) {
		// TO BE IMPELEMENTED: DISABLE INVERTER, NEED TO DETERMINE HOW
		return 0;
	}
	else {
		// TO BE IMPLEMENTED: DETERMINE HOW TO MAP TORQUE BASED ON PEDAL POSITION

		//PLACEHOLDER
		return 10;
	}
}

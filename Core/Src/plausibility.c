/*
 * plausibility.c
 *
 *  Created on: Jun 4, 2025
 *      Author: Steven Ryan Leonido
 */

#include "plausibility.h"

#define NUM_POINTS 5

// This is basically the look up tables for the pedal map (VERY BASIC), uses linear interpolation for values inbetween ones set in LUT
float pedal_table[NUM_POINTS] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
float torque_table[NUM_POINTS] = {0.0f, 3.0f, 6.0f, 9.0f, 12.0f}; // Purposefully letting it go only up to 12 Nm to see if it works first

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc3;

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
	uint32_t app1Range = appsRaw1Max - appsRaw1Min;
	uint32_t app2Range = appsRaw2Max - appsRaw2Min;

	float percent1 = appsRaw1 / app1Range;
	float percent2 = appsRaw2 / app2Range;

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
	if (HAL_ADC_PollForConversion(&hadc1, 5) == HAL_OK) {
		appsRaw1 = HAL_ADC_GetValue(&hadc3);
	}
	else {
		// throw fault
	}
	if (HAL_ADC_PollForConversion(&hadc1, 6) == HAL_OK) {
		appsRaw2 = HAL_ADC_GetValue(&hadc3);
	}
	else {
		// throw fault
	}

	// FIGURE OUT HOW TO CALCULATE PERCENTAGE BASED ON LIMITS
	// PLACEHOLDER, Unsure if works yet
	uint32_t app1Range = appsRaw1Max - appsRaw1Min;
	uint32_t app2Range = appsRaw2Max - appsRaw2Min;

	float percent1 = appsRaw1 / app1Range;
	float percent2 = appsRaw2 / app2Range;

	float percentAvg = (percent1 + percent2) / 2;


	// PLACEHOLDER
	return percentAvg;
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
 * GetTorqueFromPedal:
 * takes in the pedal position and maps it to a torque value based on the defined arrays.
 * Uses linear interpolation to determine values in between array values.
 */

float getTorqueFromPedal(float pedal_position) {
    if (pedal_position <= pedal_table[0]) return torque_table[0];
    if (pedal_position >= pedal_table[NUM_POINTS - 1]) return torque_table[NUM_POINTS - 1];

    for (int i = 0; i < NUM_POINTS - 1; i++) {
        if (pedal_position >= pedal_table[i] && pedal_position <= pedal_table[i + 1]) {
            float x0 = pedal_table[i];
            float x1 = pedal_table[i + 1];
            float y0 = torque_table[i];
            float y1 = torque_table[i + 1];

            // Linear interpolation
            return y0 + (pedal_position - x0) * (y1 - y0) / (x1 - x0);
        }
    }

    // Should not reach here, throw error if you do
    return 0.0f;
}

/*
 * MapTorque:
 * Gets Accelerator and Brake positions
 * Does Plausibility check, if fail, then return value to disable inverter
 * If Pass, Map torque request based on Accelerator Pedal Position and return torque request value
 */
int MapTorque() {
	HAL_ADC_Start(&hadc1);
	HAL_ADC_Start(&hadc3);


	float accel = AccelPos();
	float brake = BrakePos();

	int check = PlausibilityCheck(accel, brake);

	if (check == 0) {
		// TO BE IMPELEMENTED: DISABLE INVERTER, NEED TO DETERMINE HOW

		// can I return a negative value here to push into queue to pass into VCU Process to disable inverter??? ( -Jyro )

		return 0;
	}
	else {
		// TO BE IMPLEMENTED: DETERMINE HOW TO MAP TORQUE BASED ON PEDAL POSITION
		// Range of Torque request is 0-60 N*m
		// how do you want to map 0-60 to 0-1 value of pedal position.
		float torque = getTorqueFromPedal(accel);

		//PLACEHOLDER Just going to constantly send 5 N*m for now
		return torque;
	}
}

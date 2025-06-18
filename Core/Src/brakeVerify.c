/*
 * brakeVerify.c
 *
 *  Created on: Jun 13, 2025
 *      Author: Jyro
 */

#include "brakeVerify.h"

//Global Variables
extern volatile float global_brake_position;
extern volatile bool global_brake_data_updated;
extern volatile bool global_brake_isShort;

 // user defines
#define ADC_MAX_VALUE     4095    /* 12-bit ADC max value */
#define TORQUE_MAX_VALUE  32767   /* 16-bit max torque value */
#define ADC_THRESHOLD     100     /* Noise threshold for ADC */
#define BRAKE_THRESHOLD   100     /* Threshold to detect brake press */

#define TASK_PERIOD_MS   5u
#define ADC_NUM_CHANNELS 2

extern uint32_t brakesRaw1Min;
extern uint32_t brakesRaw1Max;
extern uint32_t brakesConverted;

static uint32_t brakeRaw1 = 0;

void brakeVerifyProcess(void) {

	if (HAL_ADC_Start(&hadc3) != HAL_OK) { Error_Handler(); }

	if (HAL_ADC_PollForConversion(&hadc3, 10) == HAL_OK) {
		brakeRaw1 = HAL_ADC_GetValue(&hadc3);
	}
	else {
		// throw fault - could set a brake fault flag here
		brakeRaw1 = 0;
		HAL_ADC_Stop(&hadc3); // stop the ADC if it timed out due to error
	}
	// Check if calibration data is valid
	if (brakesRaw1Max == brakesRaw1Min) {
		// Not calibrated yet or invalid calibration
		brakeRaw1 = 0;

	}

	//One more check if ADC outside of range
	if (brakeRaw1 > ADC_MAX_VALUE || brakeRaw1 < brakesRaw1Min) {
		// outside of range so set to 0 and send fault
		brakeRaw1 = 0;
		global_brake_isShort = true;
	}

	// Calculate brake position as percentage
	float percent1 = (float)(brakeRaw1 - brakesRaw1Min) / (float)(brakesRaw1Max - brakesRaw1Min);

	// Clamp to valid range
	if (percent1 < 0.0f) percent1 = 0.0f;
	if (percent1 > 1.0f) percent1 = 0.0f;

	global_brake_position = percent1;
	global_brake_data_updated = true;

}

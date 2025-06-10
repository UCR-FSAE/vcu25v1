/*
 * plausibility.c
 *
 *  Created on: Jun 4, 2025
 *      Author: Steven Ryan Leonido
 */

#include "plausibility.h"

// EXTERN DECLARATIONS FOR GLOBAL VARIABLES
extern volatile float global_accel_position;
extern volatile float global_torque_command;
extern volatile bool global_data_updated;

#define NUM_POINTS 5

// This is basically the look up tables for the pedal map (VERY BASIC), uses linear interpolation for values inbetween ones set in LUT
float pedal_table[NUM_POINTS] = {0.25f, 0.35f, 0.5f, 0.65f, 0.75f};
float torque_table[NUM_POINTS] = {0.0f, 3.0f, 6.0f, 9.0f, 12.0f}; // Purposefully letting it go only up to 12 Nm to see if it works first

float accel;
float torque;

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc3;
extern bool inverterFault;

//extern uint32_t appsRaw1;  // Get the raw value
extern uint32_t brakesRaw1Min;
extern uint32_t brakesRaw1Max;
extern uint32_t brakesRaw2Min;
extern uint32_t brakesRaw2Max;
//extern uint32_t appsConverted;
extern uint32_t brakesConverted;
extern osMessageQueueId_t torqueQueueHandle;
extern osMessageQueueId_t appsQueueHandle;


float BrakePos() {
	// Fixed: Read brake sensors properly
	uint32_t brakeRaw1_local = 0;
	uint32_t brakeRaw2_local = 0;
	
	if (HAL_ADC_PollForConversion(&hadc1, 5) == HAL_OK) {
		brakeRaw1_local = HAL_ADC_GetValue(&hadc1);
	}
	else {
		// throw fault - could set a brake fault flag here
		return 0.0f;  // Safe default
	}
	
	if (HAL_ADC_PollForConversion(&hadc1, 6) == HAL_OK) {
		brakeRaw2_local = HAL_ADC_GetValue(&hadc1);
	}
	else {
		// throw fault - could set a brake fault flag here
		return 0.0f;  // Safe default
	}

	// Check if calibration data is valid
	if (brakesRaw1Max == brakesRaw1Min || brakesRaw2Max == brakesRaw2Min) {
		// Not calibrated yet or invalid calibration
		return 0.0f;  // Safe default
	}

	// Calculate brake position as percentage
	float percent1 = (float)(brakeRaw1_local - brakesRaw1Min) / (float)(brakesRaw1Max - brakesRaw1Min);
	float percent2 = (float)(brakeRaw2_local - brakesRaw2Min) / (float)(brakesRaw2Max - brakesRaw2Min);

	// Clamp to valid range
	if (percent1 < 0.0f) percent1 = 0.0f;
	if (percent1 > 1.0f) percent1 = 1.0f;
	if (percent2 < 0.0f) percent2 = 0.0f;
	if (percent2 > 1.0f) percent2 = 1.0f;

	// Return average of both sensors
	float percentAvg = (percent1 + percent2) / 2.0f;
	return percentAvg;
}

/*
 * Plausibility Check:
 * Looks at two pedal positions and if both abosve 5%, then return 0, else return 1
 */
bool PlausibilityCheck(float accel, float brake) {

	if (accel > 0.05 && brake > 0.05) {
		// disable the inverter flag
//		inverterFault = 1;
		return false;
	}

	else {
		// continue
		inverterFault = 0;
		return true;
	}

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
    static uint32_t counter = 0;
    counter++;

    // DISABLED: LED debugging moved to ADC verify task only
    // if (counter % 500 == 0) {
    //     HAL_GPIO_TogglePin(GPIOB, LD1_Pin);
    // }

    // Read from global variable instead of queue
    accel = global_accel_position;
    
    // Reset the update flag (optional - can be used for detecting new data)
    global_data_updated = false;

    float torque = getTorqueFromPedal(accel);

    // Store torque in global variable instead of queue
    global_torque_command = torque;

    return (int)torque;
}

/*
 * plausibility.c
 *
 *  Created on: Jun 4, 2025
 *      Author: Steven Ryan Leonido
 */

#include "plausibility.h"

// EXTERN DECLARATIONS FOR GLOBAL VARIABLES
extern volatile float global_accel_position;
extern volatile float global_brake_position;
extern volatile float global_torque_command;
extern volatile bool global_accel_data_updated;
extern volatile bool global_brake_data_updated;
extern volatile bool global_plausibility_check;

#define NUM_POINTS 6

// This is basically the look up tables for the pedal map (VERY BASIC), uses linear interpolation for values inbetween ones set in LUT
float pedal_table[NUM_POINTS] = {0.15f, 0.35f, 0.5f, 0.65f, 0.75f, 0.95f};
float torque_table[NUM_POINTS] = {0.0f, 15.0f, 25.0f, 30.0f, 65.0f, 75.0f}; // Purposefully letting it go only up to 12 Nm to see if it works first

float accel;
float brake;
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

extern bool brakeTrigger;

/*
 * Plausibility Check:
 * Looks at two pedal positions and if both abosve 5%, then return 0, else return 1
 */
bool PlausibilityCheck(float accel, float brake) {

	if (accel > 0.05 && brake > 0.0) {
		// disable the inverter flag
		inverterFault = 1;
		global_plausibility_check = false;	//have fault
		return false;
	}
	else {
		// continue
//		inverterFault = 0;

		global_plausibility_check = true;	//no fault
		return true;
	}
}


/*
 *
 * Over 25% Check
 *
 */


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

            return y0 + (pedal_position - x0) * (y1 - y0) / (x1 - x0);
        }
    }

    return 0.0f;
}

/*
 * MapTorque:
 * Gets Accelerator and Brake positions
 * Does Plausibility check, if fail, then return value to disable inverter
 * If Pass, Map torque request based on Accelerator Pedal Position and return torque request value
 */

int MapTorque() {

    accel = global_accel_position;
    brake = global_brake_position;
    
    // reset flags
    global_accel_data_updated = false;
    global_brake_data_updated = false;

    if (brake >= 0.3) {
        // trigger brake light
        brakeTrigger = true;
//        HAL_GPIO_WritePin(GPIOB, LD3_Pin, SET);
//        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, SET);

    }
    else {
    	brakeTrigger = false;
//    	HAL_GPIO_WritePin(GPIOB, LD3_Pin, RESET);
//        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, RESET);
    }

    if (!PlausibilityCheck(accel, brake)) {
        inverterFault = 1;
        global_torque_command = 0;
        HAL_GPIO_WritePin(GPIOB, LD2_Pin, SET);
        HAL_GPIO_WritePin(GPIOB, LD3_Pin, SET);
    }
    else {
        float torque = getTorqueFromPedal(accel);
        global_torque_command = torque;

        return (int)torque;
    }

    return 0;

}

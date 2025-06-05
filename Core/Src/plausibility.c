/*
 * plausibility.c
 *
 *  Created on: Jun 4, 2025
 *      Author: Steven Ryan Leonido
 */

#include "plausibility.h"

/*
 * AccelPos:
 * Get Accelerator Potentiometer values from ADC and determine position percentage
 * NEED TO REFERENCE GLOBAL MIN AND MAX VALUES FOR POTENTIOMETERS, LOOK AT JUSTIN BRANCH
 */
float AccelPos() {
	// TO BE IMPLEMENTED

	// PLACEHOLDER
	return 0.5;

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

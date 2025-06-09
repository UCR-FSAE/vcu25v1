/*
 * plausibility.h
 *
 *  Created on: Jun 4, 2025
 *      Author: Steven Ryan Leonido
 */

#ifndef INC_PLAUSIBILITY_H_
#define INC_PLAUSIBILITY_H_

#include "main.h"

/*
 * Description for plausibility check:
 * System must take in 2 potentiometer values for accelerator and 2 values for brake.
 * System must check if either of the brake or accelerator position is above 5%
 * at the same time if both pedal positions are above 5%, disable
 * If okay, system should send torque request as normal.
 * Normal system operation: Map torque request based off of accelerometer position
 * and send torque request to inverter process.
 */


static uint32_t brakesRaw1 = 0;
static uint32_t brakesRaw2 = 0;


float AccelPos();

float BrakePos();

int PlausibilityCheck(float accel, float brake);

int MapTorque();


#endif /* INC_PLAUSIBILITY_H_ */

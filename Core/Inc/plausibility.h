/*
 * plausibility.h
 *
 *  Created on: Jun 4, 2025
 *      Author: steve
 */

#ifndef INC_PLAUSIBILITY_H_
#define INC_PLAUSIBILITY_H_

#include "main.h"

/*
 * Description for plausibility check
 * System must take in 2 potentiometer values for accelerator and 2 values for brake.
 * System must check if either of the brake or accelerator position is above 5% at the same time
 * If both pedal positions are above 5%, disable
 *  If okay, system should send torque request as normal
 *  Normal system operation: Map torque request based off of accelerometer position and send torque request to inverter process
 *
 */

/*
 * AccelPos:
 * Get Accelerator Potentiometer values from ADC and determine position percentage
 */
float AccelPos();

/*
 * BrakePos:
 * Get Brake Potentiometer values from ADC and determine position percentage
 */
float BrakePos();

/*
 * Plausibility Check:
 * Looks at two pedal positions and if both abosve 5%, then return 0, else return 1
 */
int PlausibilityCheck(float accel, float brake);

/*
 * MapTorque:
 * Gets Accelerator and Brake positions
 * Does Plausibility check, if fail, then return value to disable inverter
 * If Pass, Map torque request based on Accelerator Pedal Position and return torque request value
 */
int MapTorque();


#endif /* INC_PLAUSIBILITY_H_ */

/*
 * ready.c
 *
 *  Created on: Jun 18, 2025
 *      Author: Justin Im
 */

#include "ready.h"
#include "main.h"

// External variables
extern volatile float global_brake_position;
extern bool inverterFault;

// Static variables for ready system
static bool initial_brake_check_done = false;
static bool car_ready_to_drive = false;

void readyInit(void) {
    initial_brake_check_done = false;
    car_ready_to_drive = false;
    inverterFault = true; // Start with inverter disabled
}

void readyCheck(void) {
    // Check if initial brake input has been done
    if (!initial_brake_check_done) {
        // Wait for brakes to be pressed to 30%
        if (global_brake_position >= 0.30f) {
            initial_brake_check_done = true;
            // Now check PE6 and PE4 for ready inputs
            GPIO_PinState pe6_state = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_6);
            GPIO_PinState pe4_state = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4);
            
            if (pe6_state == GPIO_PIN_SET && pe4_state == GPIO_PIN_SET) {
                car_ready_to_drive = true;
                inverterFault = false; // Allow car to drive
            } else {
                car_ready_to_drive = false;
                inverterFault = true; // Stop car from driving
            }
        } else {
            // Brakes not pressed enough yet, keep inverter disabled
            inverterFault = true;
        }
    } else {
        // Initial check done, continuously monitor PE6 and PE4
        GPIO_PinState pe6_state = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_6);
        GPIO_PinState pe4_state = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4);
        
        if (pe6_state == GPIO_PIN_SET && pe4_state == GPIO_PIN_SET) {
            car_ready_to_drive = true;
            inverterFault = false; // Allow car to drive
        } else {
            car_ready_to_drive = false;
            inverterFault = true; // Stop car from driving
        }
    }
}

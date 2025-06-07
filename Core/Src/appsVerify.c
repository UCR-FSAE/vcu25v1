
#include "appsVerify.h"

// user defines
#define ADC_MAX_VALUE     4095    /* 12-bit ADC max value */
#define TORQUE_MAX_VALUE  32767   /* 16-bit max torque value */
#define ADC_THRESHOLD     100     /* Noise threshold for ADC */
#define BRAKE_THRESHOLD   100     /* Threshold to detect brake press */

#define TASK_PERIOD_MS   5u

// variables
// use these for mapping
extern uint32_t appsRaw1Min;
extern uint32_t appsRaw1Max;
extern uint32_t appsRaw2Min;
extern uint32_t appsRaw2Max;
extern uint32_t appsConverted;

extern bool pedalFault;


static uint32_t appsRaw1 = 0;
static uint32_t appsRaw2 = 0;
static uint32_t appsConverted1;
static uint32_t appsConverted2;

void appsVerifyInit(void) {
	// start adc
	HAL_ADC_Start(&hadc3);

	// calibrate apps sensors

}


void appsVerifyProcess(void) {
	static bool offsetFlag = false;
	TickType_t  mismatchStart  = 0;

	// read accelerator pedal press for 5 ms
	if (HAL_ADC_PollForConversion(&hadc3, 5) == HAL_OK) {
		appsRaw1 = HAL_ADC_GetValue(&hadc3);
	}
	else {
		// throw fault
	}
	if (HAL_ADC_PollForConversion(&hadc3, 5) == HAL_OK) {
		appsRaw2 = HAL_ADC_GetValue(&hadc3);
	}
	else {
		// throw fault
	}

	// mapping
	appsConverted1 = (uint16_t)(((uint32_t)(appsRaw1 - appsRaw1Min) * TORQUE_MAX_VALUE) / (appsRaw1Max - appsRaw1Min));
	appsConverted2 = (uint16_t)(((uint32_t)(appsRaw2 - appsRaw2Min) * TORQUE_MAX_VALUE) / (appsRaw2Max - appsRaw2Min));

	// if the values differ by 10% for 100ms, throw fault
	if (appsConverted1/appsConverted2 >= 1.1 || appsConverted1/appsConverted2 <= 0.9) {
		if (!offsetFlag) {
			offsetFlag = true;
			mismatchStart = xTaskGetTickCount();
		}
		// get tick count from first mismatch to 100
        else if ((xTaskGetTickCount() - mismatchStart) >= pdMS_TO_TICKS(100)) {
        	pedalFault = true;
        }
	}
	else {
		offsetFlag = false;
	}

	// calculate the average apps value
	appsConverted = ( appsConverted1 + appsConverted2 ) / 2;

	return;
}


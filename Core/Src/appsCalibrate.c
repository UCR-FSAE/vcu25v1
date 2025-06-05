
#include "appsCalibrate.h"

extern uint32_t appsRaw1Min;
extern uint32_t appsRaw1Max;
extern uint32_t appsRaw2Min;
extern uint32_t appsRaw2Max;


void appsCalibrate(void) {
	// read accelerator pedal press

	// 3 seconds for min calibration
	if (HAL_ADC_PollForConversion(&hadc3, 5) == HAL_OK) {
		appsRaw1Min = HAL_ADC_GetValue(&hadc3);
	}
	else {
		// throw fault
	}
	if (HAL_ADC_PollForConversion(&hadc3, 7) == HAL_OK) {
		appsRaw2Min = HAL_ADC_GetValue(&hadc3);
	}
	else {
		// throw fault
	}

	// 3 seconds for max calibration
	if (HAL_ADC_PollForConversion(&hadc3, 5) == HAL_OK) {
		appsRaw1Max = HAL_ADC_GetValue(&hadc3);
	}
	else {
		// throw fault
	}
	if (HAL_ADC_PollForConversion(&hadc3, 7) == HAL_OK) {
		appsRaw2Max = HAL_ADC_GetValue(&hadc3);
	}
	else {
		// throw fault
	}


	// check thresholds
//	if (acceleratorRaw1 < ADC_THRESHOLD && acceleratorRaw2 < ADC_THRESHOLD) {
//
//	}

	// perform some sort of math here to fix the pot offset

	// if the values differ by 10% for 100ms, throw fault


	return;
}


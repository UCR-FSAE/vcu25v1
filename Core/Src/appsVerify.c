
#include "appsVerify.h"


// variables
// use these for mapping
extern uint32_t appsRaw1Min;
extern uint32_t appsRaw1Max;
extern uint32_t appsRaw2Min;
extern uint32_t appsRaw2Max;


static uint32_t appsRaw1 = 0;
static uint32_t appsRaw2 = 0;

void appsVerifyInit(void) {
	// start adc
	HAL_ADC_Start(&hadc3);

	// calibrate apps sensors



}


void appsVerifyProcess(void) {
	// read accelerator pedal press
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

	// check thresholds
//	if (acceleratorRaw1 < ADC_THRESHOLD && acceleratorRaw2 < ADC_THRESHOLD) {
//
//	}

	// perform some sort of math here to fix the pot offset

	// if the values differ by 10% for 100ms, throw fault


	return;
}



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
uint32_t adcResults[2];


static uint32_t appsRaw1 = 0;
static uint32_t appsRaw2 = 0;
static float appsConverted1;
static float appsConverted2;

void appsVerifyProcess(void) {
	static bool offsetFlag = false;
	TickType_t  mismatchStart  = 0;

    // Start ADC conversion for both channels
    if (HAL_ADC_Start(&hadc3) != HAL_OK) {
        // throw fault - ADC start failed
        pedalFault = true;
        return;
    }

    // Wait for first conversion
    if (HAL_ADC_PollForConversion(&hadc3, 5) == HAL_OK) {
        adcResults[0] = HAL_ADC_GetValue(&hadc3);
    }
    else {
        pedalFault = true;
        HAL_ADC_Stop(&hadc3);
        return;
    }

    // Wait for second conversion
    if (HAL_ADC_PollForConversion(&hadc3, 5) == HAL_OK) {
        adcResults[1] = HAL_ADC_GetValue(&hadc3);
    }
    else {
        pedalFault = true;
        HAL_ADC_Stop(&hadc3);
        return;
    }

	// Store raw values
	appsRaw1 = adcResults[0];
	appsRaw2 = adcResults[1];

    // Prevent division by zero
    if (appsRaw1Max == appsRaw1Min || appsRaw2Max == appsRaw2Min) {
        pedalFault = true;
        return;
    }


    // converts raw apps values to digital values
    appsConverted1 = (float)(appsRaw1 - appsRaw1Min) / (float)(appsRaw1Max - appsRaw1Min);
    appsConverted2 = (float)(appsRaw2 - appsRaw2Min) / (float)(appsRaw2Max - appsRaw2Min);


    uint32_t diff = (appsConverted1 > appsConverted2) ?
                    (appsConverted1 - appsConverted2) :
                    (appsConverted2 - appsConverted1);

    uint32_t threshold = (appsConverted1 + appsConverted2) / 20; // 10% of average

    if (diff > threshold && (appsConverted1 > 0 || appsConverted2 > 0)) {
        if (!offsetFlag) {
            offsetFlag = true;
            mismatchStart = osKernelGetTickCount();  // Use CMSIS-RTOS
        }
        else if ((osKernelGetTickCount() - mismatchStart) >= 100) { // 100ms in ticks
            pedalFault = true;
        }
    }
    else {
        offsetFlag = false;
    }

    // Calculate the average apps value
    appsConverted = (appsConverted1 + appsConverted2) / 2;
}

